// SPDX-License-Identifier: Apache-2.0
// UNINTEGRATED CANDIDATE: requires real product compilation and pixel/lifecycle
// validation before packaging or claiming the public APIs are implemented.
#include "accessibility_blink_controller.h"

#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <poll.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <utility>

#include "display_manager.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_wrapper.h"
#include "ui/rs_canvas_node.h"
#include "ui/rs_surface_node.h"
#include "ui/rs_ui_context.h"
#include "transaction/rs_transaction.h"
#include "wm/window.h"
#include "window_option.h"

namespace OHOS {
namespace Accessibility {
namespace {
constexpr int32_t ALREADY_FLASHING = 1;
constexpr int32_t DEVICE_IN_USE = 2;
constexpr int32_t FEATURE_DISABLED = 5;
constexpr int32_t SINGLE_BLINK = 1;
constexpr int32_t CONTINUOUS_BLINK = 2;
constexpr int64_t PULSE_MS = 750; // Below one complete on/off cycle per second.
constexpr uint32_t VISIBLE_COLOR = 0x70ffff00;
constexpr const char *TICK = "AccessibilityScreenBlinkTick";

struct Reply {
    RetError error = RET_ERR_FAILED;
    int32_t result = SUCCESS;
};
struct Request {
    std::mutex mutex;
    bool cancelled = false;
    bool completed = false;
    Reply reply;
    std::promise<Reply> promise;
};
}

class AccessibilityBlinkController::Impl {
public:
    explicit Impl(ReadEnabled readEnabled) : readEnabled_(std::move(readEnabled))
    {
        runner_ = AppExecFwk::EventRunner::Create("AccessibilityScreenBlink");
        if (runner_) {
            handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
        }
    }
    ~Impl() { Shutdown(); }

    RetError Control(bool start, int32_t mode, int32_t scenario, int32_t pid,
        uint32_t token, int32_t &result)
    {
        result = SUCCESS;
        if ((mode != SINGLE_BLINK && mode != CONTINUOUS_BLINK) || scenario < 1 || scenario > 4 || pid <= 0) {
            return RET_ERR_INVALID_PARAM;
        }
        auto request = std::make_shared<Request>();
        auto future = request->promise.get_future();
        const uint64_t epoch = epoch_.load();
        {
            std::lock_guard<std::mutex> submit(submitMutex_);
            if (closing_ || !handler_) {
                return RET_ERR_FAILED;
            }
            if (!handler_->PostTask([this, request, start, mode, scenario, pid, token, epoch] {
                {
                    std::lock_guard<std::mutex> guard(request->mutex);
                    if (request->cancelled || closing_ || epoch != epoch_.load()) {
                        request->completed = true;
                        request->promise.set_value(request->reply);
                        return;
                    }
                }
                const uint64_t previous = generation_;
                Reply reply = Apply(start, mode, scenario, pid, token, epoch, request);
                bool abandoned;
                {
                    std::lock_guard<std::mutex> guard(request->mutex);
                    abandoned = request->cancelled || epoch != epoch_.load() || closing_;
                    if (abandoned) {
                        reply = {};
                    }
                    request->reply = reply;
                    request->completed = true;
                }
                if (abandoned && generation_ != previous) {
                    Clear();
                }
                request->promise.set_value(reply);
            }, "AccessibilityScreenBlinkControl")) {
                return RET_ERR_FAILED;
            }
        }
        if (future.wait_for(std::chrono::seconds(3)) == std::future_status::ready) {
            const auto reply = future.get();
            result = reply.result;
            return reply.error;
        }
        std::lock_guard<std::mutex> guard(request->mutex);
        if (request->completed) {
            result = request->reply.result;
            return request->reply.error;
        }
        request->cancelled = true;
        return RET_ERR_TIME_OUT;
    }

    void CancelForAccountChange()
    {
        std::lock_guard<std::mutex> submit(submitMutex_);
        ++epoch_;
        if (!closing_ && handler_) {
            // Never synchronously wait here: account changes run on the AAMS
            // handler which supplies ReadEnabled to the screen worker.
            handler_->PostTask([this] { Clear(); }, "AccessibilityBlinkAccountChanged");
        }
    }

    void Shutdown()
    {
        std::shared_ptr<AppExecFwk::EventHandler> handler;
        {
            std::lock_guard<std::mutex> submit(submitMutex_);
            if (closing_.exchange(true)) {
                return;
            }
            ++epoch_;
            handler = handler_;
        }
        if (handler) {
            handler->RemoveAllEvents();
            if (!handler->PostSyncTask([this] { Clear(); }, "AccessibilityBlinkShutdown")) {
                HILOG_ERROR("Unable to drain accessibility screen blink worker");
            }
        }
        std::lock_guard<std::mutex> submit(submitMutex_);
        handler_.reset();
        runner_.reset();
    }

private:
    bool Cancelled(const std::shared_ptr<Request> &request, uint64_t epoch)
    {
        std::lock_guard<std::mutex> guard(request->mutex);
        return request->cancelled || closing_ || epoch != epoch_.load();
    }

    Reply Apply(bool start, int32_t mode, int32_t scenario, int32_t pid, uint32_t token, uint64_t epoch,
        const std::shared_ptr<Request> &request)
    {
        if (!start) {
            if (active_ && (pid != ownerPid_ || token != ownerToken_ || mode != mode_ || scenario != scenario_)) {
                return {RET_OK, DEVICE_IN_USE};
            }
            return {Clear(), SUCCESS};
        }
        bool enabled = false;
        const RetError read = readEnabled_(enabled);
        // Settings/WM IPC can outlive the public command deadline. Never
        // create a new visible window for a request already abandoned.
        if (Cancelled(request, epoch)) {
            return {};
        }
        if (read != RET_OK) {
            return {read, SUCCESS};
        }
        if (!enabled) {
            return {RET_OK, FEATURE_DISABLED};
        }
        if (active_) {
            return {RET_OK, pid == ownerPid_ && token == ownerToken_ && mode == mode_ && scenario == scenario_ ?
                ALREADY_FLASHING : DEVICE_IN_USE};
        }
        if (window_ && Clear() != RET_OK) {
            return {};
        }
#ifdef SYS_pidfd_open
        const int pidfd = static_cast<int>(syscall(SYS_pidfd_open, pid, 0));
#else
        const int pidfd = -1;
#endif
        if (pidfd < 0) {
            return {}; // No PID-existence fallback: it cannot protect against reuse.
        }
        ownerFd_ = pidfd;
        ownerPid_ = pid;
        ownerToken_ = token;
        mode_ = mode;
        scenario_ = scenario;
        activeEpoch_ = epoch;
        ++generation_;
        if (!CreateWindow([this, request, epoch] { return Cancelled(request, epoch); })) {
            Clear();
            return {};
        }
        active_ = true;
        visible_ = true;
        if (!Schedule()) {
            Clear();
            return {};
        }
        return {RET_OK, SUCCESS};
    }

    bool CreateWindow(const std::function<bool()> &cancelled)
    {
        if (cancelled()) {
            return false;
        }
        auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
        if (!display || display->GetWidth() <= 0 || display->GetHeight() <= 0) {
            return false;
        }
        displayId_ = display->GetId();
        width_ = display->GetWidth();
        height_ = display->GetHeight();
        sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
        option->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
        option->SetDisplayId(displayId_);
        option->SetWindowRect({0, 0, static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)});
        option->SetFocusable(false);
        option->SetTouchable(false);
        window_ = Rosen::Window::Create("AccessibilityScreenBlink", option);
        if (!window_) {
            return false;
        }
        surface_ = window_->GetSurfaceNode();
        context_ = surface_ ? surface_->GetRSUIContext() : nullptr;
        if (!context_ || !context_->GetRSTransaction()) {
            return false;
        }
        canvas_ = Rosen::RSCanvasNode::Create(false, false, context_);
        if (!canvas_) {
            return false;
        }
        canvas_->SetBounds(0, 0, width_, height_);
        canvas_->SetFrame(0, 0, width_, height_);
        canvas_->SetBackgroundColor(VISIBLE_COLOR);
        surface_->AddChild(canvas_, -1);
        context_->GetRSTransaction()->FlushImplicitTransaction();
        return !cancelled() && window_->Show() == Rosen::WMError::WM_OK;
    }

    bool Schedule()
    {
        const uint64_t generation = generation_;
        return handler_->PostTask([this, generation] { Tick(generation); }, TICK, PULSE_MS);
    }

    void Tick(uint64_t generation)
    {
        if (!active_ || generation != generation_) {
            return;
        }
        bool enabled = false;
        pollfd owner = {ownerFd_, POLLIN, 0};
        auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
        if (closing_ || activeEpoch_ != epoch_.load() || poll(&owner, 1, 0) != 0 ||
            readEnabled_(enabled) != RET_OK || !enabled || !display || display->GetId() != displayId_ ||
            display->GetWidth() != width_ || display->GetHeight() != height_ || mode_ == SINGLE_BLINK) {
            Clear();
            return;
        }
        visible_ = !visible_;
        canvas_->SetBackgroundColor(visible_ ? VISIBLE_COLOR : 0);
        context_->GetRSTransaction()->FlushImplicitTransaction();
        if (!Schedule()) {
            Clear();
        }
    }

    RetError Clear()
    {
        ++generation_;
        active_ = false;
        visible_ = false;
        if (handler_) {
            handler_->RemoveTask(TICK);
        }
        if (ownerFd_ >= 0) {
            close(ownerFd_);
            ownerFd_ = -1;
        }
        if (canvas_ && context_ && context_->GetRSTransaction()) {
            canvas_->SetBackgroundColor(0);
            context_->GetRSTransaction()->FlushImplicitTransaction();
        }
        if (window_ && window_->Destroy() != Rosen::WMError::WM_OK) {
            HILOG_ERROR("Unable to destroy accessibility screen blink window");
            return RET_ERR_FAILED; // Retain the transparent window for a later cleanup attempt.
        }
        canvas_.reset();
        surface_.reset();
        context_.reset();
        window_ = nullptr;
        return RET_OK;
    }

    ReadEnabled readEnabled_;
    std::mutex submitMutex_;
    std::atomic<bool> closing_ {false};
    std::atomic<uint64_t> epoch_ {0};
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    sptr<Rosen::Window> window_;
    std::shared_ptr<Rosen::RSSurfaceNode> surface_;
    std::shared_ptr<Rosen::RSCanvasNode> canvas_;
    std::shared_ptr<Rosen::RSUIContext> context_;
    uint64_t generation_ = 0;
    uint64_t activeEpoch_ = 0;
    uint64_t displayId_ = 0;
    int32_t width_ = 0;
    int32_t height_ = 0;
    int ownerFd_ = -1;
    int32_t ownerPid_ = 0;
    uint32_t ownerToken_ = 0;
    int32_t mode_ = 0;
    int32_t scenario_ = 0;
    bool active_ = false;
    bool visible_ = false;
};

AccessibilityBlinkController::AccessibilityBlinkController(ReadEnabled readEnabled)
    : impl_(std::make_unique<Impl>(std::move(readEnabled))) {}
AccessibilityBlinkController::~AccessibilityBlinkController() = default;
RetError AccessibilityBlinkController::Control(bool start, int32_t mode, int32_t scenario,
    int32_t callerPid, uint32_t callerToken, int32_t &result)
{
    return impl_->Control(start, mode, scenario, callerPid, callerToken, result);
}
void AccessibilityBlinkController::Shutdown() { impl_->Shutdown(); }
void AccessibilityBlinkController::CancelForAccountChange() { impl_->CancelForAccountChange(); }
} // namespace Accessibility
} // namespace OHOS
