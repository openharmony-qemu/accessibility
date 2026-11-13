/*
 * Copyright (C) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "accessible_ability_manager_service.h"
#include "accessibility_blink_controller.h"
#include "element_operator_callback_impl.h"
#include "ability_manager_client.h"
#include "want.h"
#include <algorithm>
#include <cinttypes>
#include <new>
#include <string>
#include <unistd.h>
#include <functional>
#ifdef OHOS_BUILD_ENABLE_HITRACE
#include <hitrace_meter.h>
#endif // OHOS_BUILD_ENABLE_HITRACE

#include "ability_info.h"
#include "accessibility_event_info.h"
#ifdef OHOS_BUILD_ENABLE_POWER_MANAGER
#include "accessibility_power_manager.h"
#endif
#include "accessibility_short_key_dialog.h"
#include "accessibility_window_manager.h"
#include "hilog_wrapper.h"
#include "input_manager.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "parameter.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "utils.h"
#include "xcollie_helper.h"
#include <ipc_skeleton.h>
#include "resource_manager.h"
#include "res_config.h"
#include "locale_config.h"
#include "locale_info.h"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"
#include "accessibility_caption.h"
#include "msdp_manager.h"
#include "accessibility_permission.h"
#include "magnification_def.h"
#include "mem_mgr_client.h"
#include "mem_mgr_proxy.h"
#include "common_event_manager.h"
#include "accessibility_notification_helper.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_TAG AAMS_LOG_TAG
#define LOG_DOMAIN AAMS_LOG_DOMAIN

using namespace std;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccessibilityConfig;

namespace OHOS {
namespace Accessibility {
namespace {
    const char* AAMS_SERVICE_NAME = "AccessibleAbilityManagerService";
    const char* AAMS_ACTION_RUNNER_NAME = "AamsActionRunner";
    const char* AAMS_SEND_EVENT_RUNNER_NAME = "AamsSendEventRunner";
    const char* AAMS_CHANNEL_RUNNER_NAME = "AamsChannelRunner";
    const char* AAMS_HOVER_ENTER_RUNNER_NAME = "AamsHoverEnterRunner";
    const char* SYSTEM_PARAMETER_AAMS_NAME = "accessibility.config.ready";
    const char* SCREEN_READER_BUNDLE_ABILITY_NAME = "com.ohos.screenreader/AccessibilityExtAbility";
    const char* DEVICE_PROVISIONED = "device_provisioned";
    const char* SCREEN_MAGNIFICATION_KEY = "accessibility_display_magnification_enabled";
    const char* SCREEN_MAGNIFICATION_TYPE = "accessibility_magnification_capability";
    const char* SCREEN_MAGNIFICATION_MODE = "accessibility_magnification_mode";
    const char* ELDER_CARE_ENABLED_KEY = "accessibility_elder_care_switch_enabled";
    const char* SENIOR_MODE_STATE_KEY = "accessibility_senior_mode_state_for_app";
    const char* SCREEN_MAGNIFICATION_SCALE = "accessibility_display_magnification_scale";
    const char* SCREEN_MAGNIFICATION_TRIGGER_METHOD = "accessibility_display_magnification_trigger_method";
    const char* ZOOM_GESTURE_CONFLICT_DIALOG_OPEN = "accessibility_zoom_gesture_conflict_dialog_open";
    const char* MAGNIFICATION_PARAM = "const.accessibility.magnification";
    const char* VOICE_RECOGNITION_KEY = "accessibility_sound_recognition_switch";
    const char* VOICE_RECOGNITION_TYPES = "accessibility_sound_recognition_enabled";
    const char* FLASH_REMINDER_SWITCH_KEY = "accessibility_flash_reminder_switch";
#ifdef ACCESSIBILITY_WATCH_FEATURE
    const char* DELAY_UNLOAD_TASK = "TASK_UNLOAD_ACCESSIBILITY_SA";
    constexpr int32_t UNLOAD_TASK_INTERNAL = 3 * 60 * 1000; // ms
#endif
    const char* USER_SETUP_COMPLETED = "user_setup_complete";
    const char* ACCESSIBILITY_CLONE_FLAG = "accessibility_config_clone";
    const char* SHORTCUT_ENABLED = "accessibility_shortcut_enabled";
    const char* HAP_PATH = "/system/app/Settings/Settings.hap";
    const char* HAP_BUNDLE = "com.ohos.settings";
    const char* UI_TEST_BUNDLE_NAME = "ohos.uitest";
    const char* PC_MODE_SWITCH = "window_pcmode_switch_status";
    const char* PC_MODE_SUPPORT = "const.window.support_window_pcmode_switch";
    const char* TRANSITION_ANIMATIONS_NOTIFICATION = "transition_animations_notification";
    const char* IGNORE_REPEAT_CLICK_NOTIFICATION = "ignore_repeat_click_notification";
    const char* ANCO_ACCESSIBILITY_STATE = "anco_accessibility_state";
    constexpr int32_t INVALID_SHORTCUT_STATE = 2;
    constexpr int32_t QUERY_USER_ID_RETRY_COUNT = 600;
    constexpr int32_t QUERY_USER_ID_SLEEP_TIME = 50;
    constexpr int32_t DEFAULT_ACCOUNT_ID = 100;
    constexpr int32_t ROOT_UID = 0;
    constexpr int32_t TREE_ID_INVALID = 0;
    constexpr int32_t SINGLE_TREE_ID = 0;
    constexpr int32_t WINDOW_ID_INVALID = -1;
    constexpr int64_t ELEMENT_ID_INVALID = -1;
    enum SCREENREADER_STATE : int32_t {
        UNINIT = -1,
        OFF = 0,
        ON = 1,
    };
    constexpr int32_t SHORT_KEY_TIMEOUT_BEFORE_USE = 3000; // ms
    constexpr int32_t SHORT_KEY_TIMEOUT_AFTER_USE = 1000; // ms
    constexpr int32_t MAX_PUBLISH_RETRY_TIMES = 3;
    constexpr int32_t PUBLISH_RETRY_DELAY_MS = 100; // ms
    const char* TIMER_REGISTER_STATE_OBSERVER = "accessibility:registerStateObServer";
    const char* TIMER_REGISTER_CAPTION_OBSERVER = "accessibility:registerCaptionObServer";
    const char* TIMER_REGISTER_ENABLEABILITY_OBSERVER = "accessibility:registerEnableAbilityObServer";
    const char* TIMER_GET_ALL_CONFIG = "accessibility:getAllConfig";
    const char* TIMER_REGISTER_CONFIG_OBSERVER = "accessibility:registerConfigObserver";
    const char* MAGNIFICATION_SCALE = "magnification_scale";
    const char* MAGNIFICATION_DISABLE = "magnification_disabled";
    const char* SWITCH_FULL_SCREEN = "switch_full_screen_magnification";
    const char* SWITCH_WINDOW = "switch_window_magnification";
    const char* CAPTION_ENABLED_KEY = "vassistant_ai_caption_enable";
    const char* CAPTION_SHORTKEY_NAME = "CAPTIONS_ASSISTANT";
    const char* CAPTION_BUNDLE_NAME = "com.ohos.vassistant";
    const char* CAPTION_ABILITY_NAME = "AiCaptionServiceExtAbility";
    const char* CAPTION_CLOSE_URL_ADDRESS  = "ohos://vassistant/caption?action=close";
    constexpr int32_t XCOLLIE_TIMEOUT = 6; // s
    constexpr int QUANTITY = 2; // plural string
    constexpr int32_t BROKER_UID = 5557;
    constexpr int32_t SECURITY_COMPONENT_UID = 3050;
    static std::map<std::string, std::string> ResourceMap = {
        {MAGNIFICATION_SCALE, ""},
        {MAGNIFICATION_DISABLE, ""},
        {SWITCH_FULL_SCREEN, ""},
        {SWITCH_WINDOW, ""}
    };

    const std::map<std::string, int32_t> AccessibilityConfigTable = {
        {"HIGH_CONTRAST_TEXT", HIGH_CONTRAST_TEXT},
        {"INVERT_COLOR", INVERT_COLOR},
        {"DALTONIZATION_COLOR_FILTER", DALTONIZATION_COLOR_FILTER},
        {"CONTENT_TIMEOUT", CONTENT_TIMEOUT},
        {"ANIMATION_OFF", ANIMATION_OFF},
        {"BRIGHTNESS_DISCOUNT", BRIGHTNESS_DISCOUNT},
        {"AUDIO_MONO", AUDIO_MONO},
        {"AUDIO_BALANCE", AUDIO_BALANCE},
        {"MOUSE_KEY", MOUSE_KEY},
        {"CAPTION_STATE", CAPTION_STATE},
        {"CAPTION_STYLE", CAPTION_STYLE},
        {"SCREEN_MAGNIFICATION", SCREEN_MAGNIFICATION},
        {"MOUSE_AUTOCLICK", MOUSE_AUTOCLICK}
    };
} // namespace

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(&Singleton<AccessibleAbilityManagerService>::GetInstance());

AccessibleAbilityManagerService::AccessibleAbilityManagerService()
    : SystemAbility(ACCESSIBILITY_MANAGER_SERVICE_ID, true)
{
    HILOG_INFO("AccessibleAbilityManagerService is constructed");
    dependentServicesStatus_[ABILITY_MGR_SERVICE_ID] = false;
    dependentServicesStatus_[BUNDLE_MGR_SERVICE_SYS_ABILITY_ID] = false;
    dependentServicesStatus_[COMMON_EVENT_SERVICE_ID] = false;
    dependentServicesStatus_[DISPLAY_MANAGER_SERVICE_SA_ID] = false;
    dependentServicesStatus_[SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN] = false;
    dependentServicesStatus_[WINDOW_MANAGER_SERVICE_ID] = false;
    dependentServicesStatus_[DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID] = false;

    accessibilitySettings_ = std::make_shared<AccessibilitySettings>();
    accessibilityShortKey_ = std::make_shared<AccessibilityShortKey>();
}

AccessibleAbilityManagerService::~AccessibleAbilityManagerService()
{
    HILOG_INFO("AccessibleAbilityManagerService::~AccessibleAbilityManagerService");
    if (blinkController_) {
        blinkController_->Shutdown();
    }
    a11yAccountsData_.Clear();
}

void AccessibleAbilityManagerService::InitHandler()
{
    if (!runner_) {
        runner_ = AppExecFwk::EventRunner::Create(AAMS_SERVICE_NAME, AppExecFwk::ThreadMode::FFRT);
        if (!runner_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS runner failed");
            return;
        }
    }

    if (!handler_) {
        handler_ = std::make_shared<AAMSEventHandler>(runner_);
        if (!handler_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS event handler failed");
            return;
        }
    }
}

void AccessibleAbilityManagerService::InitActionHandler()
{
    if (!actionRunner_) {
        actionRunner_ = AppExecFwk::EventRunner::Create(AAMS_ACTION_RUNNER_NAME, AppExecFwk::ThreadMode::FFRT);
        if (!actionRunner_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS action runner failed");
            return;
        }
    }

    if (!actionHandler_) {
        actionHandler_ = std::make_shared<AAMSEventHandler>(actionRunner_);
        if (!actionHandler_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS action handler failed");
            return;
        }
    }
}

void AccessibleAbilityManagerService::InitSendEventHandler()
{
    if (!sendEventRunner_) {
        sendEventRunner_ = AppExecFwk::EventRunner::Create(AAMS_SEND_EVENT_RUNNER_NAME, AppExecFwk::ThreadMode::FFRT);
        if (!sendEventRunner_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS sendEvent runner failed");
            return;
        }
    }

    if (!sendEventHandler_) {
        sendEventHandler_ = std::make_shared<AAMSEventHandler>(sendEventRunner_);
        if (!sendEventHandler_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS sendEvent handler failed");
            return;
        }
    }
}

void AccessibleAbilityManagerService::InitChannelHandler()
{
    if (!channelRunner_) {
        channelRunner_ = AppExecFwk::EventRunner::Create(AAMS_CHANNEL_RUNNER_NAME, AppExecFwk::ThreadMode::FFRT);
        if (!channelRunner_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS channel runner failed");
            return;
        }
    }

    if (!channelHandler_) {
        channelHandler_ = std::make_shared<AAMSEventHandler>(channelRunner_);
        if (!channelHandler_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS channel handler failed");
            return;
        }
    }
}

void AccessibleAbilityManagerService::InitHoverEnterHandler()
{
    if (!hoverEnterRunner_) {
        hoverEnterRunner_ = AppExecFwk::EventRunner::Create(AAMS_HOVER_ENTER_RUNNER_NAME, AppExecFwk::ThreadMode::FFRT);
        if (!hoverEnterRunner_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS hoverEnter runner failed");
            return;
        }
    }

    if (!hoverEnterHandler_) {
        hoverEnterHandler_ = std::make_shared<AAMSEventHandler>(hoverEnterRunner_);
        if (!hoverEnterHandler_) {
            HILOG_ERROR("AccessibleAbilityManagerService::OnStart failed:create AAMS hoverEnter handler failed");
            return;
        }
    }
}

void AccessibleAbilityManagerService::OnStart()
{
    HILOG_COMM_INFO("AccessibleAbilityManagerService::OnStart start");

    InitHandler();
    InitActionHandler();
    InitSendEventHandler();
    InitChannelHandler();
    InitHoverEnterHandler();

    SetParameter(SYSTEM_PARAMETER_AAMS_NAME, "false");

    HILOG_DEBUG("AddAbilityListener!");
    AddSystemAbilityListener(ABILITY_MGR_SERVICE_ID);
    AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    AddSystemAbilityListener(DISPLAY_MANAGER_SERVICE_SA_ID);
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
    AddSystemAbilityListener(WINDOW_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    AddSystemAbilityListener(MEMORY_MANAGER_SA_ID);

    accessibilitySettings_->RegisterSettingsHandler(handler_);
    {
        std::lock_guard<ffrt::mutex> guard(blinkMutex_);
        blinkStopping_ = false;
    }
}

void AccessibleAbilityManagerService::OnStop()
{
    HILOG_COMM_INFO("stop AccessibleAbilityManagerService");
    std::shared_ptr<AccessibilityBlinkController> blink;
    {
        std::lock_guard<ffrt::mutex> guard(blinkMutex_);
        blinkStopping_ = true;
        blink.swap(blinkController_);
    }
    // Drain the screen worker before tearing down its settings handler.
    if (blink) {
        blink->Shutdown();
    }
    if (!handler_) {
        HILOG_ERROR("AccessibleAbilityManagerService::OnStop failed!");
        return;
    }

    ffrt::promise<void> syncPromise;
    ffrt::future syncFuture = syncPromise.get_future();
    handler_->PostTask([this, &syncPromise]() {
        HILOG_DEBUG();

        Singleton<AccessibilityCommonEvent>::GetInstance().UnSubscriberEvent();
        if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
            Singleton<ExtendManagerServiceProxy>::GetInstance().UnregisterDisplayListener();
        }
        UnsubscribeOsAccount();

        currentAccountId_ = -1;
        a11yAccountsData_.Clear();
        stateObserversDeathRecipient_ = nullptr;

        syncPromise.set_value();
        }, "TASK_ONSTOP");
    syncFuture.wait();

    for (auto &iter : dependentServicesStatus_) {
        iter.second = false;
    }

    isReady_ = false;
    isPublished_ = false;
    SetParameter(SYSTEM_PARAMETER_AAMS_NAME, "false");
    int pid = getpid();
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(pid, 1, 0, ACCESSIBILITY_MANAGER_SERVICE_ID);
    HILOG_INFO("AccessibleAbilityManagerService::OnStop OK.");
}

void AccessibleAbilityManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    HILOG_DEBUG("systemAbilityId:%{public}d added!", systemAbilityId);
    if (!handler_) {
        HILOG_DEBUG("Event handler is nullptr.");
        return;
    }

    handler_->PostTask([=]() {
        if (systemAbilityId == MEMORY_MANAGER_SA_ID) {
            int pid = getpid();
            Memory::MemMgrClient::GetInstance().NotifyProcessStatus(pid, 1, 1, ACCESSIBILITY_MANAGER_SERVICE_ID);
            return;
        }
        auto iter = dependentServicesStatus_.find(systemAbilityId);
        if (iter == dependentServicesStatus_.end()) {
            HILOG_ERROR("SystemAbilityId is not found!");
            return;
        }

        dependentServicesStatus_[systemAbilityId] = true;
        if (std::any_of(dependentServicesStatus_.begin(), dependentServicesStatus_.end(),
            [](const std::map<int32_t, bool>::value_type &status) { return !status.second; })) {
            HILOG_DEBUG("Not all the dependence is ready!");
            return;
        }

        if (Init() == false) {
            HILOG_COMM_ERROR("AccessibleAbilityManagerService::Init failed!");
            return;
        }

        if (!isPublished_) {
            if (Publish(this) == false) {
                HILOG_ERROR("AccessibleAbilityManagerService::Publish failed!");
                return;
            }
            isPublished_ = true;
        }

        InitInnerResource();

        isReady_ = true;
        SetParameter(SYSTEM_PARAMETER_AAMS_NAME, "true");
        PublishAccessibilityCommonEvent(SYSTEM_PARAMETER_AAMS_NAME);
        HILOG_COMM_INFO("AAMS is ready!");
        RegisterShortKeyEvent();
        PostDelayUnloadTask();
        RegisterScreenMagnificationState();
        RegisterScreenMagnificationType();
        RegisterVoiceRecognitionState();
        RegisterFlashReminderSwitch();
        RegisterSeniorModeState();
        RegisterSeniorModeStateForAppObserver();
        if (accessibilitySettings_) {
            accessibilitySettings_->RegisterParamWatcher();
            UpdateAccessibilityState();
        }
        }, "OnAddSystemAbility");
}

void AccessibleAbilityManagerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    HILOG_COMM_INFO("systemAbilityId:%{public}d removed!", systemAbilityId);
    Utils::RecordOnRemoveSystemAbility(systemAbilityId);
    if (!handler_) {
        HILOG_DEBUG("Event handler is nullptr.");
        return;
    }

    handler_->PostTask([=]() {
        HILOG_INFO("Remove system ability start");
        auto iter = dependentServicesStatus_.find(systemAbilityId);
        if (iter == dependentServicesStatus_.end()) {
            HILOG_ERROR("SystemAbilityId is not found!");
            return;
        }

        dependentServicesStatus_[systemAbilityId] = false;
        if (isReady_) {
            SwitchedUser(-1);
            Singleton<AccessibilityCommonEvent>::GetInstance().UnSubscriberEvent();
            if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
                Singleton<ExtendManagerServiceProxy>::GetInstance().UnregisterDisplayListener();
            }

            isReady_ = false;
            SetParameter(SYSTEM_PARAMETER_AAMS_NAME, "false");
        }

        if (systemAbilityId == DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID) {
            sptr<AccessibilityAccountData> accountData =
                Singleton<AccessibleAbilityManagerService>::GetInstance().GetCurrentAccountData();
            if (accountData == nullptr) {
                HILOG_ERROR("get accountData failed");
                return;
            }
            auto config = accountData->GetConfig();
            if (config == nullptr) {
                HILOG_ERROR("config is nullptr");
                return;
            }
            config->SetInitializeState(false);
        }
        }, "OnRemoveSystemAbility");
}

int AccessibleAbilityManagerService::Dump(int fd, const std::vector<std::u16string>& args)
{
    HILOG_DEBUG("dump AccessibilityManagerServiceInfo");
    if (!handler_) {
        HILOG_ERROR("Parameters check failed!");
        return RET_ERR_NULLPTR;
    }
    ffrt::promise<int> syncPromise;
    ffrt::future syncFuture = syncPromise.get_future();
    handler_->PostTask([this, &syncPromise, fd, args]() {
        if (!accessibilityDumper_) {
            accessibilityDumper_ = new(std::nothrow) AccessibilityDumper();
            if (!accessibilityDumper_) {
                HILOG_ERROR("accessibilityDumper_ is nullptr");
                syncPromise.set_value(-1);
                return;
            }
        }
        syncPromise.set_value(accessibilityDumper_->Dump(fd, args));
        }, "TASK_DUMP_INFO");
    return syncFuture.get();
}

RetError AccessibleAbilityManagerService::VerifyingToKenId(const int32_t windowId, const int64_t elementId,
    int32_t userId, uint32_t tokenId)
{
    if (tokenId == 0) {
        tokenId = IPCSkeleton::GetCallingTokenID();
    }
    HILOG_DEBUG("VerifyingToKenId: windowId[%{public}d], elementId[%{public}" PRId64 "]", windowId, elementId);
    if (elementId == ELEMENT_ID_INVALID || windowId == WINDOW_ID_INVALID) {
        HILOG_DEBUG("windowId[%{public}d], elementId[%{public}" PRId64 "]", windowId, elementId);
        return RET_OK;
    }

    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    if (accountData == nullptr) {
        Utils::RecordUnavailableEvent(A11yUnavailableEvent::CONNECT_EVENT,
            A11yError::ERROR_CONNECT_TARGET_APPLICATION_FAILED);
            HILOG_ERROR("Get current account data failed!!");
            return RET_ERR_CONNECTION_EXIST;
    }
    return accountData->GetElementOperatorManager().VerifyingToKenId(windowId, elementId, tokenId);
}

int32_t AccessibleAbilityManagerService::GetUserIdByDisplayId(uint64_t displayId)
{
    int32_t localId = -1;
    ErrCode errCode = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(displayId, localId);
    if (errCode != ERR_OK) {
        return GetCurrentAccountId();
    }
    return localId;
}

void AccessibleAbilityManagerService::SendAccessibilityEventToAA(
    EventType eventType, GestureType gestureId, uint64_t displayId)
{
    HILOG_INFO("eventType is 0x%{public}x.", eventType);
    int32_t userId = GetUserIdByDisplayId(displayId);
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    RETURN_IF_NULL(accountData);
    AccessibilityEventInfo eventInfo {};
    int32_t windowId = accountData->GetWindowManager().GetActiveWindowId();
    eventInfo.SetWindowId(windowId);
    eventInfo.SetEventType(eventType);
    if (eventType == EventType::TYPE_GESTURE_EVENT) {
        eventInfo.SetGestureType(gestureId);
    }
    AccessibilityEventInfoParcel eventInfoParcel(eventInfo);
    InnerSendEvent(eventInfoParcel, 0, userId);
}

ErrCode AccessibleAbilityManagerService::SendEvent(
    const AccessibilityEventInfoParcel &eventInfoParcel, int32_t flag)
{
    return InnerSendEvent(eventInfoParcel, flag, InnerGetCallingUid());
}

ErrCode AccessibleAbilityManagerService::InnerSendEvent(
    const AccessibilityEventInfoParcel &eventInfoParcel, int32_t flag, int32_t userId)
{
    if (!sendEventHandler_ || !hoverEnterHandler_) {
        HILOG_ERROR("Parameters check failed!");
        return RET_ERR_NULLPTR;
    }
    AccessibilityEventInfo uiEvent = static_cast<AccessibilityEventInfo>(eventInfoParcel);
    EventType eventType = uiEvent.GetEventType();
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    auto sendEventTask = [this, uiEvent, flag, tokenId, userId]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr. userId = %{public}d", userId);
            return;
        }
        accountData->GetElementOperatorManager().SendEvent(uiEvent, flag, tokenId);
    };

    if (eventType == TYPE_VIEW_HOVER_ENTER_EVENT) {
        hoverEnterHandler_->PostTask(sendEventTask, "TASK_SEND_EVENT");
    } else {
        sendEventHandler_->PostTask(sendEventTask, "TASK_SEND_EVENT");
    }

    return RET_OK;
}
// LCOV_EXCL_STOP

ErrCode AccessibleAbilityManagerService::RegisterStateObserver(
    const sptr<IAccessibleAbilityManagerStateObserver>& stateObserver, uint32_t &state)
{
    HILOG_DEBUG();
    int32_t userId = InnerGetCallingUid();
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    if (accountData == nullptr) {
        return ERR_INVALID_DATA;
    }
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (accountData->RegisterStateObserver(stateObserver, state) != RET_OK) {
        return RET_ERR_FAILED;
    }
    if (!stateObserversDeathRecipient_) {
        stateObserversDeathRecipient_ = new(std::nothrow) StateCallbackDeathRecipient();
        if (!stateObserversDeathRecipient_) {
            HILOG_ERROR("stateObserversDeathRecipient_ is null");
            return ERR_INVALID_DATA;
        }
    }

    if (!stateObserver->AsObject()) {
        HILOG_ERROR("object is null");
        return ERR_INVALID_DATA;
    }

    stateObserver->AsObject()->AddDeathRecipient(stateObserversDeathRecipient_);
    return ERR_OK;
}

bool AccessibleAbilityManagerService::FindFocusedElement(
    AccessibilityElementInfo &elementInfo, uint32_t timeout, int32_t userId)
{
    HILOG_DEBUG();
    if (timeout > TIME_OUT_OPERATOR) {
        HILOG_ERROR("invalid timeout value!");
        return false;
    }
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    RETURN_FALSE_IF_NULL(accountData);
    return accountData->GetElementOperatorManager().FindFocusedElement(elementInfo, timeout);
}

bool AccessibleAbilityManagerService::ExecuteActionOnAccessibilityFocused(const ActionType &action, int32_t userId)
{
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    RETURN_FALSE_IF_NULL(accountData);
    return accountData->GetElementOperatorManager().ExecuteActionOnAccessibilityFocused(action);
}

bool AccessibleAbilityManagerService::InnerGetAccessibilityWindow(
    int32_t windowId, AccessibilityWindowInfo &window, int32_t userId)
{
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    RETURN_FALSE_IF_NULL(accountData);
    return accountData->GetWindowManager().GetAccessibilityWindow(windowId, window);
}

void AccessibleAbilityManagerService::PublishAccessibilityCommonEvent(const std::string &event)
{
    HILOG_INFO("event: %{public}s", event.c_str());
    AAFwk::Want want;
    want.SetAction(event);
    EventFwk::CommonEventData commonData {want};
    for (int retry = 1; retry <= MAX_PUBLISH_RETRY_TIMES; retry++) {
        if (EventFwk::CommonEventManager::PublishCommonEvent(commonData)) {
            return;
        }
        HILOG_ERROR("PublishCommonEvent failed, retry times: %{public}d", retry);
        if (retry < MAX_PUBLISH_RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(PUBLISH_RETRY_DELAY_MS));
        }
    }
}

ErrCode AccessibleAbilityManagerService::RegisterCaptionObserver(
    const sptr<IAccessibleAbilityManagerCaptionObserver> &callback)
{
    HILOG_DEBUG();
    if (!callback || !actionHandler_) {
        HILOG_ERROR("Parameters check failed!");
        return ERR_INVALID_VALUE;
    }

    XCollieHelper timer(TIMER_REGISTER_CAPTION_OBSERVER, XCOLLIE_TIMEOUT);
    std::shared_ptr<ffrt::promise<uint32_t>> syncPromise = std::make_shared<ffrt::promise<uint32_t>>();
    ffrt::future syncFuture = syncPromise->get_future();
    actionHandler_->PostTask([this, syncPromise, callback]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("Account data is null");
            syncPromise->set_value(ERR_INVALID_VALUE);
            return;
        }
        if (!captionPropertyCallbackDeathRecipient_) {
            captionPropertyCallbackDeathRecipient_ = new(std::nothrow) CaptionPropertyCallbackDeathRecipient();
            if (!captionPropertyCallbackDeathRecipient_) {
                HILOG_ERROR("captionPropertyCallbackDeathRecipient_ is null");
                syncPromise->set_value(ERR_INVALID_VALUE);
                return;
            }
        }
        if (!callback->AsObject()) {
            HILOG_ERROR("object is null");
            syncPromise->set_value(0);
            return;
        }
        callback->AsObject()->AddDeathRecipient(captionPropertyCallbackDeathRecipient_);
        accountData->AddCaptionPropertyCallback(callback);
        HILOG_DEBUG("the size of caption property callbacks is %{public}zu",
            accountData->GetCaptionPropertyCallbacks().size());
        syncPromise->set_value(NO_ERROR);
        }, "TASK_REGISTER_CAPTION_OBSERVER");

    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_OPERATOR));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("Failed to wait RegisterCaptionObserver result");
        return RET_ERR_TIME_OUT;
    }
    return syncFuture.get();
}

ErrCode AccessibleAbilityManagerService::RegisterEnableAbilityListsObserver(
    const sptr<IAccessibilityEnableAbilityListsObserver> &observer)
{
    HILOG_DEBUG();
    if (!observer || !actionHandler_) {
        HILOG_ERROR("Parameters check failed!");
        return ERR_INVALID_DATA;
    }
    XCollieHelper timer(TIMER_REGISTER_ENABLEABILITY_OBSERVER, XCOLLIE_TIMEOUT);
    std::shared_ptr<ffrt::promise<ErrCode>> syncPromisePtr = std::make_shared<ffrt::promise<ErrCode>>();
    ffrt::future syncFuture = syncPromisePtr->get_future();
    int32_t userId = InnerGetCallingUid();
    actionHandler_->PostTask([this, syncPromisePtr, observer, userId]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("Account data is null");
            syncPromisePtr->set_value(ERR_INVALID_DATA);
            return;
        }
        if (!enableAbilityListsObserverDeathRecipient_) {
            enableAbilityListsObserverDeathRecipient_ = new(std::nothrow) EnableAbilityListsObserverDeathRecipient();
            if (!enableAbilityListsObserverDeathRecipient_) {
                HILOG_ERROR("enableAbilityListsObserverDeathRecipient_ is null");
                syncPromisePtr->set_value(ERR_INVALID_DATA);
                return;
            }
        }
        if (!observer->AsObject()) {
            HILOG_ERROR("object is null");
            syncPromisePtr->set_value(ERR_OK);
            return;
        }
        observer->AsObject()->AddDeathRecipient(enableAbilityListsObserverDeathRecipient_);
        accountData->AddEnableAbilityListsObserver(observer);
        syncPromisePtr->set_value(ERR_OK);
        }, "TASK_REGISTER_ENABLE_ABILITY_LISTS_OBSERVER");

    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_OPERATOR));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("Failed to wait RegisterEnableAbilityListsObserver result");
        return ERR_TRANSACTION_FAILED;
    }
    return syncFuture.get();
}

ErrCode AccessibleAbilityManagerService::RegisterEnableAbilityCallbackObserver(
    const sptr<IAccessibilityEnableAbilityCallbackObserver> &observer)
{
    HILOG_DEBUG();
    if (!observer || !actionHandler_) {
        HILOG_ERROR("Parameters check failed!");
        return ERR_INVALID_DATA;
    }
    XCollieHelper timer(TIMER_REGISTER_ENABLEABILITY_OBSERVER, XCOLLIE_TIMEOUT);
    std::shared_ptr<ffrt::promise<ErrCode>> syncPromisePtr = std::make_shared<ffrt::promise<ErrCode>>();
    ffrt::future syncFuture = syncPromisePtr->get_future();
    int32_t userId = InnerGetCallingUid();
    actionHandler_->PostTask([this, syncPromisePtr, observer, userId]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("Account data is null");
            syncPromisePtr->set_value(ERR_INVALID_DATA);
            return;
        }
        if (!enableAbilityCallbackObserverDeathRecipient_) {
            enableAbilityCallbackObserverDeathRecipient_ = new(std::nothrow) EnableAbilityListsObserverDeathRecipient();
            if (!enableAbilityCallbackObserverDeathRecipient_) {
                HILOG_ERROR("enableAbilityListsObserverDeathRecipient_ is null");
                syncPromisePtr->set_value(ERR_INVALID_DATA);
                return;
            }
        }
        if (!observer->AsObject()) {
            HILOG_ERROR("object is null");
            syncPromisePtr->set_value(ERR_OK);
            return;
        }
        observer->AsObject()->AddDeathRecipient(enableAbilityCallbackObserverDeathRecipient_);
        accountData->AddEnableAbilityCallbackObserver(observer);
        syncPromisePtr->set_value(ERR_OK);
        }, "TASK_REGISTER_ENABLE_ABILITY_LISTS_OBSERVER");

    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_OPERATOR));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("Failed to wait RegisterEnableAbilityListsObserver result");
        return ERR_TRANSACTION_FAILED;
    }
    return syncFuture.get();
}
// LCOV_EXCL_STOP

ErrCode AccessibleAbilityManagerService::GetAbilityList(uint32_t abilityTypes, int32_t stateType,
    std::vector<AccessibilityAbilityInfoParcel>& infos)
{
    HILOG_DEBUG("abilityTypes(%{public}d) stateType(%{public}d)", abilityTypes, stateType);
    if (!handler_ || (stateType > ABILITY_STATE_INSTALLED) || (stateType < ABILITY_STATE_ENABLE)) {
        HILOG_ERROR("Parameters check failed! stateType:%{public}d", stateType);
        return RET_ERR_INVALID_PARAM;
    }
    auto resultInfos = std::make_shared<std::vector<AccessibilityAbilityInfoParcel>>();
    if (resultInfos == nullptr) {
        HILOG_ERROR("resultInfos is nullptr.");
        return RET_ERR_NULLPTR;
    }
    auto syncPromise = std::make_shared<ffrt::promise<RetError>>();
    if (syncPromise == nullptr) {
        HILOG_ERROR("syncPromise is nullptr.");
        return RET_ERR_NULLPTR;
    }
    ffrt::future syncFuture = syncPromise->get_future();
    int32_t userId = InnerGetCallingUid();
    handler_->PostTask([this, syncPromise, resultInfos, abilityTypes, stateType, userId]() {
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("Get current account data failed!!");
            syncPromise->set_value(RET_ERR_FAILED);
            return;
        }

        vector<AccessibilityAbilityInfo> abilities;
        accountData->GetAbilitiesByState(static_cast<AbilityStateType>(stateType), abilities);
        HILOG_DEBUG("abilityes count is %{public}zu", abilities.size());
        for (auto &ability : abilities) {
            if (abilityTypes == AccessibilityAbilityTypes::ACCESSIBILITY_ABILITY_TYPE_ALL ||
                (ability.GetAccessibilityAbilityType() & abilityTypes)) {
                AccessibilityAbilityInfoParcel info(ability);
                resultInfos->push_back(info);
            }
        }
        HILOG_DEBUG("infos count is %{public}zu", resultInfos->size());
        syncPromise->set_value(RET_OK);
        }, "TASK_GET_ABILITY_LIST");
    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_1000MS));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("GetAbilityList Failed to wait result");
        return RET_ERR_TIME_OUT;
    }
    infos = *resultInfos;
    return syncFuture.get();
}

bool AccessibleAbilityManagerService::IsApp() const
{
    HILOG_DEBUG();

    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == TOKEN_HAP) {
        HILOG_DEBUG("caller is an application");
        return true;
    }
    return false;
}

ErrCode AccessibleAbilityManagerService::RegisterElementOperatorByWindowId(
    const int32_t windowId, const sptr<IAccessibilityElementOperator> &elementOperator, uint64_t displayId)
{
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    bool isBroker = IsBroker();
    int32_t userId = InnerGetCallingUid();
    handler_->PostTask([=]() {
        HILOG_INFO("Register windowId[%{public}d] userId[%{public}d]", windowId, userId);
#ifdef OHOS_BUILD_ENABLE_HITRACE
        HITRACE_METER_NAME(HITRACE_TAG_ACCESSIBILITY_MANAGER, "RegisterElementOperator");
#endif // OHOS_BUILD_ENABLE_HITRACE
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        RETURN_IF_NULL(accountData);
        RetError ret = accountData->GetElementOperatorManager().RegisterElementOperatorByWindowId(
            windowId, elementOperator, tokenId, isBroker, displayId);
        if (ret != RET_OK) {
            HILOG_ERROR("register element operator failed %{public}d", ret);
            return;
        }
        accountData->GetWindowManager().IsCheckWindowIdEventExist(windowId);
        if (windowId == SCENE_BOARD_WINDOW_ID) {
            accountData->GetWindowManager().InitSceneBoard();
        }
        }, "TASK_REGISTER_ELEMENT_OPERATOR");
    return RET_OK;
}

// LCOV_EXCL_START
ErrCode AccessibleAbilityManagerService::RegisterElementOperatorByParameter(const RegistrationPara& parameter,
    const sptr<IAccessibilityElementOperator>& elementOperator)
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    int64_t nodeId = parameter.elementId;
    int32_t userId = InnerGetCallingUid();

    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }
    bool isApp = IsApp();
    handler_->PostTask([=]() {
        HILOG_INFO("Register windowId[%{public}d] userId[%{public}d]", parameter.windowId, userId);
#ifdef OHOS_BUILD_ENABLE_HITRACE
        HITRACE_METER_NAME(HITRACE_TAG_ACCESSIBILITY_MANAGER, "RegisterElementOperator");
#endif // OHOS_BUILD_ENABLE_HITRACE
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            return;
        }
        if (RET_OK != accountData->GetElementOperatorManager().RegisterElementOperatorByParameter(
            parameter, elementOperator, tokenId, isApp)) {
            return;
        }
        accountData->GetWindowManager().IsCheckWindowIdEventExist(parameter.windowId);
        }, "TASK_REGISTER_ELEMENT_OPERATOR");
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::DeregisterElementOperatorByWindowId(int32_t windowId, uint64_t displayId)
{
    int32_t userId = InnerGetCallingUid();
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return InnerDeregisterElementOperatorByWindowId(windowId, userId, displayId, tokenId, true);
}

ErrCode AccessibleAbilityManagerService::DeregisterElementOperatorByWindowIdAndTreeId(const int32_t windowId,
    const int32_t treeId, uint64_t displayId)
{
    int32_t userId = InnerGetCallingUid();
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return InnerDeregisterElementOperatorByWindowIdAndTreeId(windowId, treeId, userId, displayId, tokenId, true);
}

ErrCode AccessibleAbilityManagerService::InnerDeregisterElementOperatorByWindowId(
    int32_t windowId, int32_t userId, uint64_t displayId, uint32_t tokenId, bool needCheckToken)
{
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }
    bool isBroker = IsBroker();
    handler_->PostTask([=]() {
        HILOG_INFO("Deregister windowId[%{public}d] userId[%{public}d]", windowId, userId);
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr.");
            return;
        }
        if (needCheckToken) {
            ErrCode ret = CheckDeregisterTokenId(windowId, SINGLE_TREE_ID, tokenId, accountData);
            if (ret != RET_OK) {
                HILOG_ERROR("CheckDeregisterTokenId failed %{public}d", ret);
                return;
            }
        }
        if (accountData->GetElementOperatorManager().DeregisterElementOperatorByWindowId(
            windowId, displayId, isBroker) != RET_OK) {
            return;
        }
        if (windowId == SCENE_BOARD_WINDOW_ID) {
            accountData->GetWindowManager().ClearSceneBoard();
        }
        }, "TASK_DEREGISTER_ELEMENT_OPERATOR");
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::InnerDeregisterElementOperatorByWindowIdAndTreeId(
    int32_t windowId, int32_t treeId, int32_t userId, uint64_t displayId, uint32_t tokenId, bool needCheckToken)
{
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }

    handler_->PostTask([=]() {
        HILOG_INFO("Deregister windowId[%{public}d], treeId[%{public}d], userId[%{public}d] start",
            windowId, treeId, userId);
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr.");
            return;
        }
        if (needCheckToken) {
            ErrCode ret = CheckDeregisterTokenId(windowId, treeId, tokenId, accountData);
            if (ret != RET_OK) {
                HILOG_ERROR("CheckDeregisterTokenId failed %{public}d", ret);
                return;
            }
        }
        accountData->GetElementOperatorManager().DeregisterElementOperatorByWindowIdAndTreeId(
            windowId, treeId, displayId);
        }, "TASK_DEREGISTER_ELEMENT_OPERATOR");
    return RET_OK;
}

bool AccessibleAbilityManagerService::IsSystemApp() const
{
    HILOG_DEBUG();

    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType != TOKEN_HAP) {
        HILOG_INFO("Caller is not a application.");
        return true;
    }
    uint64_t accessTokenId = IPCSkeleton::GetCallingFullTokenID();
    bool isSystemApplication = TokenIdKit::IsSystemAppByFullTokenID(accessTokenId);
    return isSystemApplication;
}

bool AccessibleAbilityManagerService::IsBroker() const
{
    return IPCSkeleton::GetCallingUid() == BROKER_UID;
}

int32_t AccessibleAbilityManagerService::InnerGetCallingUid()
{
    int32_t userId =  Utils::GetUserIdByCallingUid();
    if (userId == 0) {
        userId = GetCurrentAccountId();
    }
    return userId;
}

ErrCode AccessibleAbilityManagerService::CheckDeregisterTokenId(
    int32_t windowId, int32_t treeId, uint32_t tokenId, sptr<AccessibilityAccountData> &accountData)
{
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_CONNECTION_EXIST;
    }
    sptr<AccessibilityWindowConnection> connection = accountData->GetAccessibilityWindowConnection(windowId);
    if (connection == nullptr) {
        HILOG_ERROR("connection is empty.");
        return RET_ERR_REGISTER_EXIST;
    }
    if (windowId == SCENE_BOARD_WINDOW_ID) {
        return connection->CheckScbTokenIdMap(tokenId) ? RET_OK : RET_ERR_TOKEN_ID;
    }
    uint32_t expectTokenId = connection->GetTokenIdMap(treeId);
    if (tokenId != expectTokenId) {
        HILOG_ERROR("tokenId error!");
        return RET_ERR_TOKEN_ID;
    }
    return RET_OK;
}

bool AccessibleAbilityManagerService::CheckPermission(const std::string &permission) const
{
    HILOG_DEBUG();
    uint32_t callerToken = IPCSkeleton::GetCallingTokenID();
    int result = TypePermissionState::PERMISSION_GRANTED;
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == TOKEN_INVALID) {
        HILOG_WARN("AccessToken type invalid!");
        return false;
    } else {
        result = AccessTokenKit::VerifyAccessToken(callerToken, permission);
    }
    if (result == TypePermissionState::PERMISSION_DENIED) {
        HILOG_WARN("AccessTokenID denied!");
        return false;
    }
    HILOG_DEBUG("tokenType %{private}d dAccessTokenID:%{private}u, permission:%{private}s matched!",
        tokenType, callerToken, permission.c_str());
    return true;
}

ErrCode AccessibleAbilityManagerService::GetCaptionProperty(CaptionPropertyParcel &caption, bool isPermissionRequired)
{
    if (isPermissionRequired && !IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetCaptionProperty(caption);
}

ErrCode AccessibleAbilityManagerService::SetCaptionProperty(const CaptionPropertyParcel &caption,
    bool isPermissionRequired)
{
    if (isPermissionRequired && !IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (isPermissionRequired && !CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetCaptionProperty(caption);
}

ErrCode AccessibleAbilityManagerService::SetCaptionState(const bool state, bool isPermissionRequired)
{
    if (isPermissionRequired && !IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (isPermissionRequired && !CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetCaptionState(state);
}

ErrCode AccessibleAbilityManagerService::GetCaptionState(bool &state, bool isPermissionRequired)
{
    if (isPermissionRequired && !IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetCaptionState(state);
}

ErrCode AccessibleAbilityManagerService::EnableAbility(const std::string &name, const uint32_t capabilities,
    const bool connectCallBackFlag)
{
    HILOG_DEBUG();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }

    std::string appBundleName = "";
    if (connectCallBackFlag) {
        Utils::GetBundleNameByCallingUid(appBundleName);
    }

    ffrt::promise<RetError> syncPromise;
    ffrt::future syncFuture = syncPromise.get_future();
    handler_->PostTask([this, &syncPromise, &name, &capabilities, connectCallBackFlag, appBundleName]() {
        HILOG_DEBUG();
        RetError result = InnerEnableAbility(name, capabilities, appBundleName);
        syncPromise.set_value(result);
        }, "TASK_ENABLE_ABILITIES");
    return syncFuture.get();
}

bool AccessibleAbilityManagerService::SetHighContrastTextAbility(bool state)
{
    HILOG_DEBUG();
    Utils::RecordEnableShortkeyAbilityEvent("HIGH_CONTRAST_TEXT", !state);
    RetError result = accessibilitySettings_->SetHighContrastTextState(!state);
    if (result != RET_OK) {
        return false;
    }
    if (state == true) {
        return true;
    }

    int32_t accountId = Singleton<AccessibleAbilityManagerService>::GetInstance().GetCurrentAccountId();
    if (!Utils::UpdateColorModeConfiguration(accountId)) {
        return false;
    }
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return false;
    }
    accountData->GetConfig()->SetColorModeState(A11yDarkModeType::DEFAULT_DARK_MODE_STATE);
    return true;
}
// LCOV_EXCL_STOP

bool AccessibleAbilityManagerService::SetTargetAbility(const int32_t targetAbilityValue)
{
    HILOG_DEBUG();

    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return false;
    }

    bool state;
    bool success = false;
    switch (targetAbilityValue) {
        case HIGH_CONTRAST_TEXT:
            state = accountData->GetConfig()->GetHighContrastTextState();
            return SetHighContrastTextAbility(state);
        case INVERT_COLOR:
            state = accountData->GetConfig()->GetInvertColorState();
            Utils::RecordEnableShortkeyAbilityEvent("INVERT_COLOR", !state);
            return accessibilitySettings_->SetInvertColorState(!state) == RET_OK;
        case ANIMATION_OFF:
            state = accountData->GetConfig()->GetAnimationOffState();
            Utils::RecordEnableShortkeyAbilityEvent("ANIMATION_OFF", !state);
            success = accessibilitySettings_->SetAnimationOffState(!state) == RET_OK;
            UpdateAccessibilityState();
            return success;
        case SCREEN_MAGNIFICATION: {
            state = accountData->GetConfig()->GetScreenMagnificationState();
            Utils::RecordEnableShortkeyAbilityEvent("SCREEN_MAGNIFICATION", !state);
            RetError ret = accessibilitySettings_->SetScreenMagnificationState(!state);
            if (!state) {
                uint32_t type = GetMagnificationType();
                uint32_t mode = GetMagnificationMode();
                Singleton<ExtendManagerServiceProxy>::GetInstance().SetMagnificationState(true, type, mode);
            }
            SettingsExtServiceAbilityConnection::UpdateSearchItem(!state);
            return ret == RET_OK;
        }
        case AUDIO_MONO:
            state = accountData->GetConfig()->GetAudioMonoState();
            Utils::RecordEnableShortkeyAbilityEvent("AUDIO_MONO", !state);
            success = accessibilitySettings_->SetAudioMonoState(!state) == RET_OK;
            UpdateAccessibilityState();
            return success;
        case MOUSE_KEY:
            state = accountData->GetConfig()->GetMouseKeyState();
            Utils::RecordEnableShortkeyAbilityEvent("MOUSE_KEY", !state);
            return accessibilitySettings_->SetMouseKeyState(!state) == RET_OK;
        case CAPTION_STATE:
            state = accountData->GetConfig()->GetCaptionState();
            Utils::RecordEnableShortkeyAbilityEvent("CAPTION_STATE", !state);
            return accessibilitySettings_->SetCaptionState(!state) == RET_OK;
        default:
            return false;
    }
}

ErrCode AccessibleAbilityManagerService::GetScreenReaderState(bool &state)
{
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return ERR_INVALID_DATA;
    }
    state = accountData->GetScreenReaderState();
    return ERR_OK;
}

RetError AccessibleAbilityManagerService::InnerEnableAbility(const std::string &name, const uint32_t capabilities,
    const std::string callerBundleName)
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    auto iter = removedAutoStartAbilities_.begin();
    for (; iter != removedAutoStartAbilities_.end(); ++iter) {
        if (*iter == name) {
            removedAutoStartAbilities_.erase(iter);
            break;
        }
    }
    return accountData->EnableAbility(name, capabilities, callerBundleName);
}

ErrCode AccessibleAbilityManagerService::GetEnabledAbilities(std::vector<std::string> &enabledAbilities)
{
    HILOG_DEBUG();
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }

    ffrt::promise<RetError> syncPromise;
    ffrt::future syncFuture = syncPromise.get_future();
    handler_->PostTask([this, &syncPromise, &enabledAbilities]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr");
            syncPromise.set_value(RET_ERR_NULLPTR);
            return;
        }
        enabledAbilities = accountData->GetEnabledAbilities();
        syncPromise.set_value(RET_OK);
        }, "TASK_GET_ENABLE_ABILITIES");
    return syncFuture.get();
}

RetError AccessibleAbilityManagerService::SetCurtainScreenUsingStatus(bool isEnable)
{
    HILOG_DEBUG();
    if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
        Singleton<ExtendManagerServiceProxy>::GetInstance().SetCurtainScreenUsingStatus(isEnable);
    }
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::DisableAbility(const std::string &name)
{
    HILOG_INFO();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }

#ifdef OHOS_BUILD_ENABLE_POWER_MANAGER
    std::string bundleName = "";
    bundleName = Utils::GetBundleNameFromUri(name);
    auto &powerManager = Singleton<AccessibilityPowerManager>::GetInstance();
    if (!powerManager.UnholdRunningLock(bundleName)) {
        HILOG_ERROR("Failed to unhold running lock.");
        return RET_ERR_FAILED;
    }
#endif
    ffrt::promise<RetError> syncPromise;
    ffrt::future syncFuture = syncPromise.get_future();
    handler_->PostTask([this, &syncPromise, &name]() {
        HILOG_DEBUG();
        RetError result = InnerDisableAbility(name);
        syncPromise.set_value(result);
        }, "TASK_DISABLE_ABILITIES");
    return syncFuture.get();
}

RetError AccessibleAbilityManagerService::InnerDisableAbility(const std::string &name)
{
    HILOG_INFO();
#ifdef OHOS_BUILD_ENABLE_HITRACE
    HITRACE_METER_NAME(HITRACE_TAG_ACCESSIBILITY_MANAGER, "InnerDisableAbility:" + name);
#endif // OHOS_BUILD_ENABLE_HITRACE

    if (!actionHandler_) {
        HILOG_ERROR("actionHandler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }
    int32_t userId = InnerGetCallingUid();
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    if (accountData->GetConnectingA11yAbility(name) != nullptr) {
        HILOG_WARN("refuse to disconnect ability %{public}s when connecting", name.c_str());
        return RET_OK;
    }
    if (name == SCREEN_READER_BUNDLE_ABILITY_NAME) {
        actionHandler_->PostTask([this, userId]() {
            ExecuteActionOnAccessibilityFocused(ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, userId);
            }, "TASK_CLEAR_FOCUS");
        SetCurtainScreenUsingStatus(false);
    }
    RetError ret = accountData->RemoveEnabledAbility(name);
    if (ret != RET_OK) {
        HILOG_ERROR("RemoveEnabledAbility failed");
        return ret;
    }

    accountData->SetAbilityAutoStartState(name, false);
    accountData->RemoveConnectingA11yAbility(name);
    accountData->UpdateAbilities();
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::CheckExtensionAbilityPermission(std::string& processName)
{
    auto id = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo info;
    auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(id, info);
    if (result != 0) {
        HILOG_ERROR("get native token info failed!");
        return RET_ERR_TOKEN_ID;
    }
    if (info.processName == "") {
        HILOG_ERROR("get native processName failed!");
        return RET_ERR_FAILED;
    }

    processName = info.processName;
    bool ret = Permission::CheckCallingPermission(OHOS_PERMISSION_ACCESSIBILITY_EXTENSION_ABILITY);
    if (ret == true) {
        HILOG_INFO("get hap permission");
        return RET_OK;
    }

    if (processName.compare("hdcd") != 0) {
        HILOG_ERROR("permission check failed, processName = %{public}s", processName.c_str());
        return RET_ERR_NO_PERMISSION;
    }

    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::SetMagnificationState(const bool state)
{
    HILOG_INFO("state = %{public}d", state);
    bool currentState = GetMagnificationState();
    uint32_t type = GetMagnificationType();
    uint32_t mode = GetMagnificationMode();
    if (!system::GetBoolParameter(MAGNIFICATION_PARAM, false)) {
        HILOG_WARN("Not support magnification");
        return RET_ERR_MAGNIFICATION_NOT_SUPPORT;
    }
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }

    if (state && !currentState) {
        HILOG_ERROR("magnification is not enabled.");
        return RET_ERR_ENABLE_MAGNIFICATION;
    }

    if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
        if (Singleton<ExtendManagerServiceProxy>::GetInstance().LoadExtProxy()) {
            Singleton<ExtendManagerServiceProxy>::GetInstance().SetMagnificationState(state, type, mode);
        }
    }
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::EnableUITestAbility(
    const sptr<IRemoteObject> &obj, int32_t userId, int32_t &actualUserId)
{
    HILOG_DEBUG();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }

    if (!obj) {
        HILOG_ERROR("obj is nullptr.");
        return RET_ERR_NULLPTR;
    }

    std::string processName = "";
    auto ret = CheckExtensionAbilityPermission(processName);
    if (ret != RET_OK) {
        return ret;
    }
    if (userId == 0) {
        userId = GetCurrentAccountId();
    }
    actualUserId = userId;
    ffrt::promise<RetError> syncPromise;
    ffrt::future syncFuture = syncPromise.get_future();
    handler_->PostTask([this, &syncPromise, obj, processName, userId]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr %{public}d", userId);
            syncPromise.set_value(RET_ERR_NULLPTR);
            return;
        }
        std::string uiTestUri = Utils::GetUri(processName, processName);
        sptr<AccessibleAbilityConnection> connection = accountData->GetAccessibleAbilityConnection(uiTestUri);
        if (connection) {
            HILOG_ERROR("connection is existed!!");
            syncPromise.set_value(RET_ERR_CONNECTION_EXIST);
            return;
        }

        std::function<void()> addUITestClientFunc = std::bind(&AccessibilityAccountData::AddUITestClient, accountData,
            obj, processName, processName);
        handler_->PostTask(addUITestClientFunc, "AddUITestClient");
        accountData->AddEnabledAbility(uiTestUri);
        syncPromise.set_value(RET_OK);
        }, "TASK_ENABLE_UI_TEST_ABILITIES");
    return syncFuture.get();
}

ErrCode AccessibleAbilityManagerService::DisableUITestAbility(int32_t userId)
{
    HILOG_DEBUG();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr.");
        return RET_ERR_NULLPTR;
    }

    std::string processName = "";
    auto ret = CheckExtensionAbilityPermission(processName);
    if (ret != RET_OK) {
        return ret;
    }
    if (userId == 0) {
        userId = GetCurrentAccountId();
    }
    std::shared_ptr<ffrt::promise<RetError>> syncPromise = std::make_shared<ffrt::promise<RetError>>();
    ffrt::future syncFuture = syncPromise->get_future();
    handler_->PostTask([this, syncPromise, processName, userId]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr");
            syncPromise->set_value(RET_ERR_NULLPTR);
            return;
        }
        std::string uiTestUri = Utils::GetUri(processName, processName);
        sptr<AccessibleAbilityConnection> connection = accountData->GetAccessibleAbilityConnection(uiTestUri);
        if (!connection) {
            HILOG_ERROR("connection is not existed!!");
            syncPromise->set_value(RET_ERR_NO_CONNECTION);
            return;
        }
        std::function<void()> removeUITestClientFunc =
            std::bind(&AccessibilityAccountData::RemoveUITestClient, accountData, connection, processName);
        handler_->PostTask(removeUITestClientFunc, "RemoveUITestClient");
        accountData->RemoveEnabledAbility(uiTestUri);
        syncPromise->set_value(RET_OK);
        }, "TASK_DISABLE_UI_TEST_ABILITIES");

    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_OPERATOR));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("Failed to wait DisableUITestAbility result");
        return RET_ERR_TIME_OUT;
    }
    return syncFuture.get();
}

ErrCode AccessibleAbilityManagerService::GetActiveWindow(int32_t &windowId)
{
    return InnerGetActiveWindow(windowId, InnerGetCallingUid());
}

ErrCode AccessibleAbilityManagerService::InnerGetActiveWindow(int32_t &windowId, int32_t userId)
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr.");
        return RET_ERR_NULLPTR;
    }
    windowId = accountData->GetWindowManager().GetActiveWindowId();
    return ERR_OK;
}
ErrCode AccessibleAbilityManagerService::GetActiveWindow(int32_t &windowId, bool systemApi)
{
    if (systemApi && !CheckPermission(OHOS_PERMISSION_ACCESSIBILITY_EXTENSION_ABILITY)) {
        HILOG_WARN("GetActiveWindow permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return InnerGetActiveWindow(windowId, InnerGetCallingUid());
}

bool AccessibleAbilityManagerService::Init()
{
    HILOG_DEBUG();
    Singleton<AccessibilityCommonEvent>::GetInstance().SubscriberEvent(handler_);

    int32_t retry = QUERY_USER_ID_RETRY_COUNT;
    int32_t sleepTime = QUERY_USER_ID_SLEEP_TIME;
    std::vector<int32_t> accountIds;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountIds);
    while (ret != ERR_OK || accountIds.size() == 0) {
        HILOG_DEBUG("Query account information failed, left retry count:%{public}d", retry);
        if (retry == 0) {
            HILOG_ERROR("Query account information failed!!!");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountIds);
        retry--;
    }
    if (accountIds.size() > 0) {
        HILOG_DEBUG("Query account information success, account id:%{public}d", accountIds[0]);
        SwitchedUser(accountIds[0]);
    }
    return true;
}

void AccessibleAbilityManagerService::InitInnerResource()
{
    UpdateSettingsInAtoHosTask();
}

sptr<AccessibilityAccountData> AccessibleAbilityManagerService::GetCurrentAccountData()
{
    HILOG_DEBUG();
    if (currentAccountId_ == -1) {
        HILOG_ERROR("current account id is wrong");
        return nullptr;
    }

    return a11yAccountsData_.GetCurrentAccountData(currentAccountId_);
}

sptr<AccessibilityAccountData> AccessibleAbilityManagerService::GetAccountData(int32_t accountId)
{
    HILOG_DEBUG();
    if (accountId == -1) {
        HILOG_ERROR("account id is wrong");
        return nullptr;
    }
    auto accountData = a11yAccountsData_.GetAccountData(accountId);
    if (accountData != nullptr) {
        return accountData;
    }
    AddedUser(accountId);
    return a11yAccountsData_.GetAccountData(accountId);
}

std::vector<int32_t> AccessibleAbilityManagerService::GetAllAccountIds()
{
    HILOG_DEBUG();
    return a11yAccountsData_.GetAllAccountIds();
}

void AccessibleAbilityManagerService::ClearFocus(int32_t windowId)
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("Get account data failed");
        return;
    }
    accountData->GetElementOperatorManager().ClearFocus(windowId);
}

void AccessibleAbilityManagerService::OutsideTouch(int32_t windowId)
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("Get account data failed");
        return;
    }
    accountData->GetElementOperatorManager().OutsideTouch(windowId);
}

void AccessibleAbilityManagerService::StateCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    Singleton<AccessibleAbilityManagerService>::GetInstance().RemoveCallback(STATE_CALLBACK, this, remote);
}

void AccessibleAbilityManagerService::CaptionPropertyCallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    Singleton<AccessibleAbilityManagerService>::GetInstance().RemoveCallback(CAPTION_PROPERTY_CALLBACK, this, remote);
}

void AccessibleAbilityManagerService::EnableAbilityListsObserverDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    Singleton<AccessibleAbilityManagerService>::GetInstance().RemoveCallback(
        ENABLE_ABILITY_LISTS_CALLBACK, this, remote);
}

void AccessibleAbilityManagerService::AddedUser(int32_t accountId)
{
    HILOG_DEBUG();
    a11yAccountsData_.AddAccountData(accountId);
}

void AccessibleAbilityManagerService::RemovedUser(int32_t accountId)
{
    HILOG_DEBUG();
    if (accountId == currentAccountId_) {
        HILOG_ERROR("Remove user failed, this account is current account.");
        return;
    }
    a11yAccountsData_.RemoveAccountData(accountId);
}

void AccessibleAbilityManagerService::SwitchedUser(int32_t accountId)
{
    HILOG_DEBUG();

    if (accountId == currentAccountId_) {
        HILOG_WARN("The account is current account id.");
        return;
    }

    std::map<std::string, uint32_t> importantEnabledAbilities;
    SCREENREADER_STATE screenReaderState = SCREENREADER_STATE::UNINIT;
    if (currentAccountId_ != -1) {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("Current account data is null");
            return;
        }
        defaultConfigCallbacks_ = accountData->GetConfigCallbacks();
        screenReaderState = accountData->GetDefaultUserScreenReaderState() ?
            SCREENREADER_STATE::ON : SCREENREADER_STATE::OFF;
        if (screenReaderState == SCREENREADER_STATE::ON) {
            ExecuteActionOnAccessibilityFocused(ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS, accountId);
        }
        accountData->GetImportantEnabledAbilities(importantEnabledAbilities);
        accountData->OnAccountSwitched();
        accountData->UpdateAccountCapabilities();
        if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
            Singleton<ExtendManagerServiceProxy>::GetInstance().UpdateInputFilter(0);
            HILOG_INFO("Unregister all multimodal interceptors");
        }
        UpdateAccessibilityState();
        UpdateShortKeyRegister();
    }
    {
        std::lock_guard<ffrt::mutex> guard(blinkMutex_);
        currentAccountId_ = accountId;
        if (blinkController_) {
            blinkController_->CancelForAccountChange();
        }
    }
    if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
        Singleton<ExtendManagerServiceProxy>::GetInstance().SetCurrentAccountId(accountId);
    }
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr.");
        return;
    }
    accountData->Init();
    accountData->SetConfigCallbacks(defaultConfigCallbacks_);
#ifdef OHOS_BUILD_ENABLE_POWER_MANAGER
    float discount = accountData->GetConfig()->GetBrightnessDiscount();
    if (discount != 1 || Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
        if (Singleton<ExtendManagerServiceProxy>::GetInstance().LoadExtProxy()) {
            Singleton<ExtendManagerServiceProxy>::GetInstance().DiscountBrightness(discount);
        }
    }
#endif
    AccountSA::OsAccountType accountType = accountData->GetAccountType();
    if (screenReaderState != SCREENREADER_STATE::UNINIT &&
        (accountType == AccountSA::OsAccountType::PRIVATE || accountType == AccountSA::OsAccountType::ADMIN)) {
        bool state = (screenReaderState == SCREENREADER_STATE::ON) ? true : false;
        bool ignoreRepeatClickState = accountData->GetConfig()->GetIgnoreRepeatClickState();
        if (ignoreRepeatClickState && state) {
            HILOG_INFO("in switch user screenReaderState is true, recovery ignore repeat click.");
            accountData->GetConfig()->SetIgnoreRepeatClickState(false);
        }
        accountData->SetAbilityAutoStartState(SCREEN_READER_BUNDLE_ABILITY_NAME, state);
        HILOG_INFO("set screenreader auto-start state = %{public}d", state);
    }

    handler_->PostTask([this, accountData, importantEnabledAbilities]() mutable {
        if (accountData->GetInstalledAbilitiesFromBMS()) {
            accountData->UpdateImportantEnabledAbilities(importantEnabledAbilities);
            accountData->UpdateAbilities();
            UpdateAccessibilityManagerService();
        }
    }, "SwitchedUser");

    if (accountData->GetInstalledAbilitiesFromBMS()) {
        accountData->UpdateImportantEnabledAbilities(importantEnabledAbilities);
        accountData->UpdateAbilities();
        UpdateAccessibilityManagerService();
    }
    UpdateAllSetting();
    UpdateAutoStartAbilities();
    UpdateVoiceRecognitionState();
    RegisterShortKeyEvent();
    RegisterScreenMagnificationState();
    RegisterScreenMagnificationType();
    RegisterVoiceRecognitionState();
    RegisterPcModeSwitch();
    RegisterFlashReminderSwitch();
    RegisterNotificationState();
    RegisterSeniorModeState();
    RegisterSeniorModeStateForAppObserver();
}

void AccessibleAbilityManagerService::PackageRemoved(const std::string &bundleName)
{
    sptr<AccessibilityAccountData> packageAccount = GetCurrentAccountData();
    if (!packageAccount) {
        HILOG_ERROR("packageAccount is nullptr.");
        return;
    }

    packageAccount->DelAutoStartPrefKeyInRemovePkg(bundleName);
    std::vector<std::string> multiTarget = packageAccount->GetConfig()->GetShortkeyMultiTarget();
    std::string name = packageAccount->GetConfig()->GetShortkeyTarget();
    auto installedAbilities_ = packageAccount->GetInstalledAbilities();
    for (auto &installAbility : installedAbilities_) {
        std::string abilityId = installAbility.GetId();
        HILOG_DEBUG("abilityId%{public}s", abilityId.c_str());
        if (bundleName != installAbility.GetPackageName()) {
            continue;
        }
        if (std::find(removedAutoStartAbilities_.begin(), removedAutoStartAbilities_.end(), abilityId) ==
            removedAutoStartAbilities_.end()) {
            removedAutoStartAbilities_.push_back(abilityId);
        }
        // no use later version
        if (abilityId == name) {
            std::string targetName = "";
            packageAccount->GetConfig()->SetShortkeyTarget(targetName);
            UpdateShortkeyTarget();
        }
        // multi
        for (const auto &target : multiTarget) {
            if (target == abilityId) {
                packageAccount->GetConfig()->SetShortkeyMultiTargetInPkgRemove(abilityId);
                UpdateShortkeyMultiTarget();
            }
        }
    }

    if (packageAccount->RemoveAbility(bundleName)) {
        HILOG_DEBUG("ability%{public}s removed!", bundleName.c_str());
        UpdateAccessibilityManagerService();
    }
}

void AccessibleAbilityManagerService::PackageAdd(const std::string &bundleName)
{
    sptr<AccessibilityAccountData> packageAccount = GetCurrentAccountData();
    if (!packageAccount) {
        HILOG_ERROR("packageAccount is nullptr");
        return;
    }
    for (auto &abilityId : removedAutoStartAbilities_) {
        if (packageAccount->GetAbilityAutoStartState(abilityId)) {
            packageAccount->SetAbilityAutoStartState(abilityId, false);
        }
    }
    packageAccount->AddAbility(bundleName);
}

void AccessibleAbilityManagerService::PackageChanged(const std::string &bundleName)
{
    sptr<AccessibilityAccountData> packageAccount = GetCurrentAccountData();
    if (!packageAccount || !packageAccount->GetConfig()) {
        HILOG_ERROR("packageAccount is nullptr");
        return;
    }

    bool isNeedUpdateShortKeyTarget = false;
    std::string target = packageAccount->GetConfig()->GetShortkeyTarget();
    if (target.substr(0, target.find("/")) == bundleName) {
        isNeedUpdateShortKeyTarget = true;
    }
    std::vector<std::string> multiTarget = packageAccount->GetConfig()->GetShortkeyMultiTarget();

    packageAccount->ChangeAbility(bundleName);
    UpdateAccessibilityManagerService();

    std::vector<std::string> sameBundleTarget;
    auto installedAbilities_ = packageAccount->GetInstalledAbilities();
    for (auto &installAbility : installedAbilities_) {
        std::string abilityId = installAbility.GetId();
        if (bundleName != installAbility.GetPackageName()) {
            continue;
        }
        if (abilityId == target) {
            isNeedUpdateShortKeyTarget = false;
        }
        sameBundleTarget.push_back(abilityId);
    }

    if (isNeedUpdateShortKeyTarget) {
        packageAccount->GetConfig()->SetShortkeyTarget("");
        UpdateShortkeyTarget();
    }
    std::vector<std::string> tmpAbilities = multiTarget;
    bool isNeedUpdateShortKeyMultiTarget = false;
    Utils::SelectUsefulFromVecWithSameBundle(tmpAbilities, sameBundleTarget,
        isNeedUpdateShortKeyMultiTarget, bundleName);
    if (isNeedUpdateShortKeyMultiTarget) {
        packageAccount->GetConfig()->SetShortkeyMultiTarget(tmpAbilities);
        UpdateShortkeyMultiTarget();
    }
}

void AccessibleAbilityManagerService::UpdateAccessibilityManagerService()
{
    HILOG_DEBUG("start.");

    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("Account data is null");
        return;
    }

    accountData->UpdateAccountCapabilities();
    UpdateInputFilter();
    UpdateAccessibilityState();
    UpdateShortKeyRegister();
}

void AccessibleAbilityManagerService::UpdateAccessibilityState()
{
    std::vector<int32_t> accountIds = GetAllAccountIds();
    sptr<AccessibilityAccountData> accountData = nullptr;
    uint32_t state = 0;
    for (int32_t accountId : accountIds) {
        accountData = GetAccountData(accountId);
        if (!accountData) {
            continue;
        }
        state = accountData->UpdateAccessibilityState();
        if (accountId == GetCurrentAccountId() && state != currentAccessibilityState_) {
            currentAccessibilityState_ = state;
            std::shared_ptr<AccessibilitySettingProvider> service =
                AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
            if (service == nullptr) {
                HILOG_ERROR("service is nullptr");
                return;
            }
            service->PutIntValue(ANCO_ACCESSIBILITY_STATE, static_cast<int32_t>(currentAccessibilityState_));
        }
    }
}
void AccessibleAbilityManagerService::UpdateCriticalState()
{
    std::vector<int32_t> accountIds = GetAllAccountIds();
    sptr<AccessibilityAccountData> accountData = nullptr;
    int pid = getpid();
    for (int32_t accountId : accountIds) {
        accountData = GetAccountData(accountId);
        if (!accountData) {
            continue;
        }
        if (accountData->GetAccessibilityState() || accountData->GetInputFilterFlag()) {
            Memory::MemMgrClient::GetInstance().SetCritical(pid, true, ACCESSIBILITY_MANAGER_SERVICE_ID);
            return;
        }
    }
    Memory::MemMgrClient::GetInstance().SetCritical(pid, false, ACCESSIBILITY_MANAGER_SERVICE_ID);
}

void AccessibleAbilityManagerService::UpdateCaptionProperty()
{
    return accessibilitySettings_->UpdateCaptionProperty();
}

void AccessibleAbilityManagerService::UpdateSettingsInAtoHosTask()
{
    HILOG_DEBUG();
    if (!handler_) {
        HILOG_ERROR("UpdateSettingsInAtoHosTask: handler is nullptr!");
        return;
    }
    handler_->PostTask([=]() {
        UpdateSettingsInAtoHos();
        }, "UPDATE_SETTINGS_IN_ATOHOS_TASK");
}

void AccessibleAbilityManagerService::UpdateAutoStartAbilities()
{
    HILOG_DEBUG();
    if (!handler_) {
        HILOG_ERROR("UpdateAutoStartAbilities: handler is nullptr.");
        return;
    }

    handler_->PostTask([=]() {
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("Account data is null");
            return;
        }
        for (auto &abilityId : removedAutoStartAbilities_) {
            if (accountData->GetAbilityAutoStartState(abilityId)) {
                accountData->SetAbilityAutoStartState(abilityId, false);
            }
        }
        accountData->UpdateAutoStartEnabledAbilities();
        accountData->UpdateAbilities();
        }, "UPDATE_AUTO_START_ABILITIES");
}

void AccessibleAbilityManagerService::UpdateSettingsInAtoHos()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr.");
        return;
    }

    if (!accountData->GetConfig()->GetStartToHosState() || currentAccountId_ != DEFAULT_ACCOUNT_ID) {
        HILOG_INFO("Not first start from a to hos.");
        return;
    }
    accessibilitySettings_->UpdateSettingsInAtoHos();
    UpdateAccessibilityState();
}

void AccessibleAbilityManagerService::UpdateInputFilter()
{
    HILOG_DEBUG("start.");

    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("Account data is null");
        return;
    }

    uint32_t flag = accountData->GetInputFilterFlag();
    HILOG_DEBUG("InputInterceptor flag is %{public}d", flag);

    if (flag != 0 || Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
        if (Singleton<ExtendManagerServiceProxy>::GetInstance().LoadExtProxy()) {
            uint32_t clickTime = 0;
            bool state = false;
            uint32_t responseTime = 0;
            sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
            if (accountData && accountData->GetConfig()) {
                clickTime = accountData->GetConfig()->GetIgnoreRepeatClickTime();
                state = accountData->GetConfig()->GetIgnoreRepeatClickState();
                responseTime = accountData->GetConfig()->GetClickResponseTime();
            }
            Singleton<ExtendManagerServiceProxy>::GetInstance().SetClickConfig(
                static_cast<AccessibilityConfig::IGNORE_REPEAT_CLICK_TIME>(clickTime),
                state,
                static_cast<AccessibilityConfig::CLICK_RESPONSE_TIME>(responseTime));
            Singleton<ExtendManagerServiceProxy>::GetInstance().UpdateInputFilter(flag);
        }
        Utils::RecordStartingA11yEvent(flag);
        UpdateCriticalState();
        UpdateAccessibilityState();
    }
}

void AccessibleAbilityManagerService::UpdateAllSetting()
{
    accessibilitySettings_->UpdateAllSetting();
}

void AccessibleAbilityManagerService::UpdateShortKeyRegister()
{
    HILOG_DEBUG();

    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("Account data is null!");
        return;
    }

    bool shortKeyState = accountData->GetConfig()->GetShortKeyState();
    if (shortKeyState) {
        accessibilityShortKey_->Unregister();
        accessibilityShortKey_->Register();
    } else {
        accessibilityShortKey_->Unregister();
    }
}

ErrCode AccessibleAbilityManagerService::SetScreenMagnificationState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetScreenMagnificationState(state);
}

ErrCode AccessibleAbilityManagerService::SetShortKeyState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetShortKeyState(state);
}

ErrCode AccessibleAbilityManagerService::SetMouseKeyState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetMouseKeyState(state);
}

ErrCode AccessibleAbilityManagerService::SetMouseAutoClick(const int32_t time)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetMouseAutoClick(time);
}

ErrCode AccessibleAbilityManagerService::SetShortkeyTarget(const std::string &name)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetShortkeyTarget(name);
}

ErrCode AccessibleAbilityManagerService::SetShortkeyMultiTarget(const std::vector<std::string> &name)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetShortkeyMultiTarget(name);
}

ErrCode AccessibleAbilityManagerService::SetHighContrastTextState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetHighContrastTextState(state);
}

ErrCode AccessibleAbilityManagerService::SetDaltonizationState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetDaltonizationState(state);
}

ErrCode AccessibleAbilityManagerService::SetInvertColorState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetInvertColorState(state);
}

ErrCode AccessibleAbilityManagerService::SetAnimationOffState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    auto ret = accessibilitySettings_->SetAnimationOffState(state);
    return ret;
}

ErrCode AccessibleAbilityManagerService::SetAudioMonoState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    auto ret = accessibilitySettings_->SetAudioMonoState(state);
    UpdateAccessibilityState();
    return ret;
}

ErrCode AccessibleAbilityManagerService::SetDaltonizationColorFilter(const uint32_t filter)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetDaltonizationColorFilter(filter);
}

ErrCode AccessibleAbilityManagerService::SetContentTimeout(const uint32_t time)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetContentTimeout(time);
}

ErrCode AccessibleAbilityManagerService::SetBrightnessDiscount(const float discount)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetBrightnessDiscount(discount);
}

ErrCode AccessibleAbilityManagerService::SetAudioBalance(const float balance)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetAudioBalance(balance);
}

ErrCode AccessibleAbilityManagerService::SetClickResponseTime(const uint32_t time)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetClickResponseTime(time);
}

ErrCode AccessibleAbilityManagerService::SetIgnoreRepeatClickState(const bool state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetIgnoreRepeatClickState(state);
}

ErrCode AccessibleAbilityManagerService::SetIgnoreRepeatClickTime(const uint32_t time)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        HILOG_WARN("SetCaptionProperty permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return accessibilitySettings_->SetIgnoreRepeatClickTime(time);
}

ErrCode AccessibleAbilityManagerService::ControlBlinking(bool start, int32_t mode, int32_t scenario,
    int32_t &result)
{
    PostDelayUnloadTask();
    result = 0;
    if (!IsSystemApp()) {
        return RET_ERR_NOT_SYSTEM_APP;
    }
    if (!CheckPermission(OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG)) {
        return RET_ERR_NO_PERMISSION;
    }
    const int32_t pid = IPCSkeleton::GetCallingPid();
    const uint32_t token = IPCSkeleton::GetCallingTokenID();
    std::shared_ptr<AccessibilityBlinkController> blink;
    {
        std::lock_guard<ffrt::mutex> guard(blinkMutex_);
        if (blinkStopping_ || !accessibilitySettings_) {
            return RET_ERR_FAILED;
        }
        if (!blinkController_) {
            auto settings = accessibilitySettings_;
            blinkController_ = std::make_shared<AccessibilityBlinkController>(
                [settings](bool &enabled) { return settings->GetFlashReminderSwitch(enabled); });
        }
        blink = blinkController_;
    }
    return blink->Control(start, mode, scenario, pid, token, result);
}

ErrCode AccessibleAbilityManagerService::GetScreenMagnificationState(bool &state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetScreenMagnificationState(state);
}

ErrCode AccessibleAbilityManagerService::GetShortKeyState(bool &state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetShortKeyState(state);
}

ErrCode AccessibleAbilityManagerService::GetMouseKeyState(bool &state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetMouseKeyState(state);
}

ErrCode AccessibleAbilityManagerService::GetMouseAutoClick(int32_t &time)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetMouseAutoClick(time);
}

ErrCode AccessibleAbilityManagerService::GetShortkeyTarget(std::string &name)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetShortkeyTarget(name);
}

ErrCode AccessibleAbilityManagerService::GetShortkeyMultiTarget(std::vector<std::string> &name)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetShortkeyMultiTarget(name);
}

ErrCode AccessibleAbilityManagerService::GetHighContrastTextState(bool &state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetHighContrastTextState(state);
}

ErrCode AccessibleAbilityManagerService::GetDaltonizationState(bool &state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetDaltonizationState(state);
}

ErrCode AccessibleAbilityManagerService::GetInvertColorState(bool &state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetInvertColorState(state);
}

ErrCode AccessibleAbilityManagerService::GetAnimationOffState(bool &state)
{
    PostDelayUnloadTask();
    return accessibilitySettings_->GetAnimationOffState(state);
}

ErrCode AccessibleAbilityManagerService::GetAudioMonoState(bool &state)
{
    PostDelayUnloadTask();
    return accessibilitySettings_->GetAudioMonoState(state);
}

ErrCode AccessibleAbilityManagerService::GetDaltonizationColorFilter(uint32_t &type)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetDaltonizationColorFilter(type);
}

ErrCode AccessibleAbilityManagerService::GetContentTimeout(uint32_t &timer)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetContentTimeout(timer);
}

ErrCode AccessibleAbilityManagerService::GetBrightnessDiscount(float &brightness)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetBrightnessDiscount(brightness);
}

ErrCode AccessibleAbilityManagerService::GetAudioBalance(float &balance)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetAudioBalance(balance);
}

ErrCode AccessibleAbilityManagerService::GetClickResponseTime(uint32_t &time)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetClickResponseTime(time);
}

ErrCode AccessibleAbilityManagerService::GetIgnoreRepeatClickState(bool &state)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetIgnoreRepeatClickState(state);
}

ErrCode AccessibleAbilityManagerService::GetIgnoreRepeatClickTime(uint32_t &time)
{
    PostDelayUnloadTask();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }
    return accessibilitySettings_->GetIgnoreRepeatClickTime(time);
}

ErrCode AccessibleAbilityManagerService::GetFlashReminderSwitch(bool &state)
{
    PostDelayUnloadTask();
    return accessibilitySettings_->GetFlashReminderSwitch(state);
}

ErrCode AccessibleAbilityManagerService::GetSeniorModeState(bool &state)
{
    PostDelayUnloadTask();
    return accessibilitySettings_->GetSeniorModeState(state);
}

ErrCode AccessibleAbilityManagerService::GetAllConfigs(AccessibilityConfigData& configData,
    CaptionPropertyParcel& caption)
{
    HILOG_DEBUG();
    if (!IsSystemApp()) {
        HILOG_WARN("Not system app");
        return RET_ERR_NOT_SYSTEM_APP;
    }

    XCollieHelper timer(TIMER_GET_ALL_CONFIG, XCOLLIE_TIMEOUT);
    std::shared_ptr<ffrt::promise<ErrCode>> syncPromise = std::make_shared<ffrt::promise<ErrCode>>();
    std::shared_ptr<AccessibilityConfigData> config = std::make_shared<AccessibilityConfigData>();
    std::shared_ptr<CaptionProperty> captionInfo = std::make_shared<CaptionProperty>(caption);
    if (syncPromise == nullptr || config == nullptr || captionInfo == nullptr) {
        HILOG_WARN("create syncPromise or config failed");
        return ERR_INVALID_DATA;
    }
    ffrt::future syncFuture = syncPromise->get_future();
    actionHandler_->PostTask([this, syncPromise, config, captionInfo]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr");
            syncPromise->set_value(ERR_INVALID_DATA);
            return;
        }

        config->highContrastText_ = accountData->GetConfig()->GetHighContrastTextState();
        config->daltonizationState_ = accountData->GetConfig()->GetDaltonizationState();
        config->invertColor_ = accountData->GetConfig()->GetInvertColorState();
        config->animationOff_ = accountData->GetConfig()->GetAnimationOffState();
        config->audioMono_ = accountData->GetConfig()->GetAudioMonoState();
        config->mouseKey_ = accountData->GetConfig()->GetMouseKeyState();
        config->captionState_ = accountData->GetConfig()->GetCaptionState();
        config->screenMagnifier_ = accountData->GetConfig()->GetScreenMagnificationState();
        config->shortkey_ = accountData->GetConfig()->GetShortKeyState();
        config->mouseAutoClick_ = accountData->GetConfig()->GetMouseAutoClick();
        config->daltonizationColorFilter_ = accountData->GetConfig()->GetDaltonizationColorFilter();
        config->contentTimeout_ = accountData->GetConfig()->GetContentTimeout();
        config->brightnessDiscount_ = accountData->GetConfig()->GetBrightnessDiscount();
        config->audioBalance_ = accountData->GetConfig()->GetAudioBalance();
        config->shortkeyTarget_ = accountData->GetConfig()->GetShortkeyTarget();
        config->shortkeyMultiTarget_ = accountData->GetConfig()->GetShortkeyMultiTarget();
        *captionInfo = accountData->GetConfig()->GetCaptionProperty();
        syncPromise->set_value(ERR_OK);
        }, "TASK_GET_ALL_CONFIGS");

    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_OPERATOR));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("Failed to wait GetAllConfigs result");
        return ERR_TRANSACTION_FAILED;
    }
    configData = *config;
    return syncFuture.get();
}

// LCOV_EXCL_START
bool AccessibleAbilityManagerService::EnableCaptionsAbility(sptr<AccessibilityAccountData> accountData)
{
    HILOG_DEBUG();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return false;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return false;
    }

    if (config->GetSystemDbHandle() == nullptr) {
        HILOG_ERROR("systemDatashare_ is nullptr");
        return false;
    }

    std::string captionState = accountData->GetConfig()->GetSystemDbHandle()->GetStringValue(
        CAPTION_ENABLED_KEY, "false");
    HILOG_DEBUG("captionState is [%{public}s]", captionState.c_str());
    std::string uriAddress = captionState == "true" ? CAPTION_CLOSE_URL_ADDRESS : "";
    AAFwk::Want want;
    want.SetElementName(CAPTION_BUNDLE_NAME, CAPTION_ABILITY_NAME);
    want.SetUri(uriAddress);
    auto abilityManagerClient = AAFwk::AbilityManagerClient::GetInstance();
    if (!abilityManagerClient) {
        HILOG_ERROR("abilityManagerClient is nullptr");
        return false;
    }
    auto ret = abilityManagerClient->StartAbility(want);
    if (ret != ERR_OK) {
        HILOG_ERROR("start aiCaption ability is failed %{public}d", ret);
        return false;
    }
    return true;
}
// LCOV_EXCL_STOP

bool AccessibleAbilityManagerService::EnableShortKeyTargetAbility(const std::string &name)
{
    HILOG_DEBUG();
    HILOG_INFO("EnableShortKeyTargetAbility name = %{public}s", name.c_str());
#ifdef OHOS_BUILD_ENABLE_HITRACE
    HITRACE_METER_NAME(HITRACE_TAG_ACCESSIBILITY_MANAGER, "EnableShortKeyTargetAbility");
#endif // OHOS_BUILD_ENABLE_HITRACE

    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return false;
    }

    if (name == CAPTION_SHORTKEY_NAME) {
        if (!EnableCaptionsAbility(accountData)) {
            return false;
        }
        return true;
    }

    std::string targetAbility;
    if (name != "") {
        targetAbility = name;
    } else {
        targetAbility = accountData->GetConfig()->GetShortkeyTarget();
        HILOG_DEBUG("target ability is [%{public}s]", targetAbility.c_str());
        if (targetAbility == "") {
            HILOG_ERROR("target ability is null");
            return false;
        }
    }

    auto it = AccessibilityConfigTable.find(targetAbility);
    if (it != AccessibilityConfigTable.end()) {
        return SetTargetAbility(it->second);
    }

    uint32_t capabilities = CAPABILITY_GESTURE | CAPABILITY_KEY_EVENT_OBSERVER | CAPABILITY_RETRIEVE |
        CAPABILITY_TOUCH_GUIDE | CAPABILITY_ZOOM;
    RetError enableState = accountData->EnableAbility(targetAbility, capabilities);
    if (enableState == RET_ERR_CONNECTION_EXIST) {
        HILOG_DEBUG();
        Utils::RecordEnableShortkeyAbilityEvent(targetAbility, false);
        return InnerDisableAbility(targetAbility) == RET_OK;
    } else if (enableState == RET_OK) {
        Utils::RecordEnableShortkeyAbilityEvent(targetAbility, true);
        return true;
    } else {
        return false;
    }
}

void AccessibleAbilityManagerService::OnShortKeyProcess()
{
    HILOG_DEBUG();

    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilityShortkeyDialog> shortkeyDialog = std::make_shared<AccessibilityShortkeyDialog>();

    std::shared_ptr<AccessibilitySettingProvider> service =
        AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }
    bool oobeState = false;
    bool userSetupState = false;
    service->GetBoolValue(DEVICE_PROVISIONED, oobeState);
    if (accountData->GetConfig()->GetDbHandle()) {
        userSetupState = accountData->GetConfig()->GetDbHandle()->GetBoolValue(USER_SETUP_COMPLETED, false);
    }
    if (oobeState && userSetupState) {
        int32_t shortKeyTimeout = accountData->GetConfig()->GetShortKeyTimeout();
        if (shortKeyTimeout == SHORT_KEY_TIMEOUT_BEFORE_USE) {
            HILOG_INFO("first use short cut key");
            accountData->GetConfig()->SetShortKeyTimeout(SHORT_KEY_TIMEOUT_AFTER_USE);
            shortkeyDialog->ConnectDialog(ShortKeyDialogType::RECONFIRM);
            return;
        }
    }

    std::vector<std::string> shortkeyMultiTarget = accountData->GetConfig()->GetShortkeyMultiTarget();
    if (shortkeyMultiTarget.size() == 0) {
        EnableShortKeyTargetAbility();
    } else if (shortkeyMultiTarget.size() == 1) {
        EnableShortKeyTargetAbility(shortkeyMultiTarget[0]);
    } else {
        // dialog
        if (shortkeyDialog->ConnectDialog(ShortKeyDialogType::FUNCTION_SELECT)) {
            HILOG_DEBUG("ready to build dialog");
        }
    }
}

bool AccessibleAbilityManagerService::DisableShortKeyTargetAbility()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return false;
    }

    std::string targetAbility = accountData->GetConfig()->GetShortkeyTarget();
    HILOG_DEBUG("target ability is [%{public}s]", targetAbility.c_str());
    if (targetAbility == "") {
        HILOG_ERROR("target ability is null");
        return false;
    }
    return InnerDisableAbility(targetAbility) == RET_OK;
}

ErrCode AccessibleAbilityManagerService::RegisterConfigObserver(
    const sptr<IAccessibleAbilityManagerConfigObserver> &callback)
{
    HILOG_DEBUG();
    if (!callback || !actionHandler_) {
        HILOG_ERROR("Parameters check failed!");
        return ERR_INVALID_VALUE;
    }
    XCollieHelper timer(TIMER_REGISTER_CONFIG_OBSERVER, XCOLLIE_TIMEOUT);
    std::shared_ptr<ffrt::promise<uint32_t>> syncPromisePtr = std::make_shared<ffrt::promise<uint32_t>>();
    ffrt::future syncFuture = syncPromisePtr->get_future();
    actionHandler_->PostTask([this, syncPromisePtr, callback]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("Account data is null");
            syncPromisePtr->set_value(ERR_INVALID_VALUE);
            return;
        }
        if (!configCallbackDeathRecipient_) {
            configCallbackDeathRecipient_ = new(std::nothrow) ConfigCallbackDeathRecipient();
            if (!configCallbackDeathRecipient_) {
                HILOG_ERROR("configCallbackDeathRecipient_ is null");
                syncPromisePtr->set_value(ERR_INVALID_VALUE);
                return;
            }
        }
        if (!callback->AsObject()) {
            HILOG_ERROR("object is null");
            syncPromisePtr->set_value(0);
            return;
        }
        callback->AsObject()->AddDeathRecipient(configCallbackDeathRecipient_);
        accountData->AddConfigCallback(callback);
        HILOG_DEBUG("the size of caption property callbacks is %{public}zu",
            accountData->GetConfigCallbacks().size());
        syncPromisePtr->set_value(NO_ERROR);
        }, "TASK_REGISTER_CONFIG_OBSERVER");

    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_OPERATOR));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("Failed to wait RegisterConfigObserver result");
        return RET_ERR_TIME_OUT;
    }
    return syncFuture.get();
}

void AccessibleAbilityManagerService::ConfigCallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    Singleton<AccessibleAbilityManagerService>::GetInstance().RemoveCallback(CONFIG_CALLBACK, this, remote);
}

void AccessibleAbilityManagerService::UpdateConfigState()
{
    return accessibilitySettings_->UpdateConfigState();
}

void AccessibleAbilityManagerService::UpdateAudioBalance()
{
    return accessibilitySettings_->UpdateAudioBalance();
}

void AccessibleAbilityManagerService::UpdateBrightnessDiscount()
{
    return accessibilitySettings_->UpdateBrightnessDiscount();
}

void AccessibleAbilityManagerService::UpdateContentTimeout()
{
    return accessibilitySettings_->UpdateContentTimeout();
}

void AccessibleAbilityManagerService::UpdateDaltonizationColorFilter()
{
    return accessibilitySettings_->UpdateDaltonizationColorFilter();
}

void AccessibleAbilityManagerService::UpdateMouseAutoClick()
{
    return accessibilitySettings_->UpdateMouseAutoClick();
}

void AccessibleAbilityManagerService::UpdateShortkeyTarget()
{
    return accessibilitySettings_->UpdateShortkeyTarget();
}

void AccessibleAbilityManagerService::UpdateShortkeyMultiTarget()
{
    return accessibilitySettings_->UpdateShortkeyMultiTarget();
}

void AccessibleAbilityManagerService::UpdateClickResponseTime()
{
    return accessibilitySettings_->UpdateClickResponseTime();
}

void AccessibleAbilityManagerService::UpdateIgnoreRepeatClickTime()
{
    return accessibilitySettings_->UpdateIgnoreRepeatClickTime();
}

void AccessibleAbilityManagerService::RemoveCallback(CallBackID callback,
    const sptr<DeathRecipient> &recipient, const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("remove callback[%{public}d]", callback);
    if (!handler_) {
        HILOG_ERROR("handler is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        if (!remote.GetRefPtr()) {
            HILOG_ERROR("remote is null");
            return;
        }
        remote->RemoveDeathRecipient(recipient);

        if (callback == CONFIG_CALLBACK) {
            RemoveSavedConfigCallback(remote);
        }
        auto accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("Current account data is null");
            return;
        }
        switch (callback) {
            case STATE_CALLBACK:
                accountData->RemoveStateObserver(remote);
                break;
            case CAPTION_PROPERTY_CALLBACK:
                accountData->RemoveCaptionPropertyCallback(remote);
                break;
            case ENABLE_ABILITY_LISTS_CALLBACK:
                accountData->RemoveEnableAbilityListsObserver(remote);
                break;
            case CONFIG_CALLBACK:
                accountData->RemoveConfigCallback(remote);
                break;
            case SENIOR_MODE_STATE_CALLBACK:
                accountData->RemoveSeniorModeStateObserver(remote);
                break;
            default:
                break;
        }
        }, "RemoveCallback");
}

void AccessibleAbilityManagerService::RemoveSavedConfigCallback(const wptr<IRemoteObject>& callback)
{
    HILOG_DEBUG("start.");
    for (auto itr = defaultConfigCallbacks_.begin(); itr != defaultConfigCallbacks_.end(); itr++) {
        if ((*itr)->AsObject() == callback) {
            defaultConfigCallbacks_.erase(itr);
            break;
        }
    }
}

ErrCode AccessibleAbilityManagerService::GetFocusedWindowId(int32_t &focusedWindowId)
{
    HILOG_DEBUG();
    return InnerGetFocusedWindowId(focusedWindowId, GetCurrentAccountId());
}

std::vector<AccessibilityWindowInfo> AccessibleAbilityManagerService::GetAccessibilityWindows(int32_t uesrId)
{
    sptr<AccessibilityAccountData> accountData = GetAccountData(uesrId);
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr.");
        return std::vector<AccessibilityWindowInfo>();
    }
    return accountData->GetWindowManager().GetAccessibilityWindows();
}
 
ErrCode AccessibleAbilityManagerService::InnerGetFocusedWindowId(int32_t &focusedWindowId, int32_t userId)
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetAccountData(userId);
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr.");
        return false;
    }
    accountData->GetWindowManager().GetFocusedWindowId(focusedWindowId);
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::GetAccessibilityFocusedWindowId(int32_t &windowId, int32_t accountId)
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetAccountData(accountId);
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr.");
        return false;
    }
    windowId = accountData->GetWindowManager().a11yFocusedWindowId_;
    return RET_OK;
}

void AccessibleAbilityManagerService::OnDeviceProvisioned()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }
    std::shared_ptr<AccessibilitySettingProvider> service =
        AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }
    service->UnregisterObserver(DEVICE_PROVISIONED);
    if (accountData->GetConfig()->GetDbHandle()) {
        accountData->GetConfig()->GetDbHandle()->UnregisterObserver(USER_SETUP_COMPLETED);
    }
    if (accountData->GetDefaultUserScreenReaderState()) {
        HILOG_INFO("Modify shortKeyTimeout and shortKeyOnLockScreenState");
        accountData->GetConfig()->SetShortKeyTimeout(SHORT_KEY_TIMEOUT_AFTER_USE);
        accountData->GetConfig()->SetShortKeyOnLockScreenState(true);
        UpdateConfigState();
    }
}

void AccessibleAbilityManagerService::InitializeShortKeyState()
{
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    bool shortKeyFlag = false;
    if (accountData->GetAccountId() != DEFAULT_ACCOUNT_ID && accountData->GetConfig()->GetDbHandle() != nullptr) {
        if (accountData->GetConfig()->GetDbHandle()->GetIntValue(SHORTCUT_ENABLED, INVALID_SHORTCUT_STATE) ==
            INVALID_SHORTCUT_STATE) {
            HILOG_INFO("Initialize the shortcut key state of PrivateSpace");
            shortKeyFlag = true;
        }
    } else if (accountData->GetAccountId() == DEFAULT_ACCOUNT_ID) {
        HILOG_INFO("Initialize the shortcut key state of MainSpace");
        shortKeyFlag = true;
    }

    if (shortKeyFlag) {
        accountData->GetConfig()->SetShortKeyState(true);
        std::vector<std::string> tmpVec { SCREEN_READER_BUNDLE_ABILITY_NAME };
        accountData->GetConfig()->SetShortkeyMultiTarget(tmpVec);
        UpdateConfigState();
        Singleton<AccessibleAbilityManagerService>::GetInstance().UpdateShortKeyRegister();
    }
}

void AccessibleAbilityManagerService::RegisterProvisionCallback()
{
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingProvider> service =
        AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }
    AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string &state) {
        Singleton<AccessibleAbilityManagerService>::GetInstance().OnDeviceProvisioned();
    };
    service->RegisterObserver(DEVICE_PROVISIONED, func);
    if (accountData->GetConfig()->GetDbHandle() != nullptr) {
        accountData->GetConfig()->GetDbHandle()->RegisterObserver(USER_SETUP_COMPLETED, func);
    }
}

void AccessibleAbilityManagerService::RegisterShortKeyEvent()
{
    HILOG_DEBUG();
    if (!handler_) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }
        std::shared_ptr<AccessibilitySettingProvider> service =
            AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
        if (service == nullptr) {
            HILOG_ERROR("service is nullptr");
            return;
        }
        bool oobeState = false;
        bool userSetupState = false;
        service->GetBoolValue(DEVICE_PROVISIONED, oobeState);
        if (accountData->GetConfig()->GetDbHandle() != nullptr) {
            userSetupState = accountData->GetConfig()->GetDbHandle()->GetBoolValue(USER_SETUP_COMPLETED, false);
        }
        if (accountData->GetAccountId() == DEFAULT_ACCOUNT_ID && (oobeState == false || userSetupState == false)) {
            InitializeShortKeyState();
            RegisterProvisionCallback();
        } else if (accountData->GetAccountId() != DEFAULT_ACCOUNT_ID) {
            InitializeShortKeyState();
        }
        }, "REGISTER_SHORTKEY_OBSERVER");
}

void AccessibleAbilityManagerService::OnScreenMagnificationStateChanged()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }

    if (config->GetDbHandle() == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }

    bool screenMagnificationEnabled = false;
    screenMagnificationEnabled = config->GetDbHandle()->GetBoolValue(SCREEN_MAGNIFICATION_KEY, false);
    config->SetMagnificationState(screenMagnificationEnabled);
    if (!screenMagnificationEnabled) {
        if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
            Singleton<ExtendManagerServiceProxy>::GetInstance().OnScreenMagnificationStateChanged();
        }
    }
    Singleton<AccessibleAbilityManagerService>::GetInstance().UpdateInputFilter();
}

void AccessibleAbilityManagerService::SetConfigScreenMagnificationScale(float scale)
{
    HILOG_DEBUG();

    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }
    config->SetScreenMagnificationScale(scale);
}

void AccessibleAbilityManagerService::OnScreenMagnificationScaleChanged()
{
    HILOG_INFO();

    shared_ptr<AccessibilityDatashareHelper> helper = GetCurrentAcountDatashareHelper();
    if (helper == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }
    float screenMagnificationScale = helper->GetFloatValue(SCREEN_MAGNIFICATION_SCALE, DEFAULT_SCALE);

    SetConfigScreenMagnificationScale(screenMagnificationScale);
}

void AccessibleAbilityManagerService::SetConfigScreenMagnificationTriggerMethod(int32_t triggerMethod)
{
    HILOG_DEBUG();
 
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }
 
    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }
    config->SetScreenMagnificationTriggerMethod(triggerMethod);
}
 
void AccessibleAbilityManagerService::OnScreenMagnificationTriggerMethodChanged()
{
    HILOG_INFO();
 
    shared_ptr<AccessibilityDatashareHelper> helper = GetCurrentAcountDatashareHelper();
    if (helper == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }
    int32_t screenMagnificationTriggerMethod = helper->GetIntValue(SCREEN_MAGNIFICATION_TRIGGER_METHOD,
        THREE_FINGER_DOUBLE_TAP_MODE);
 
    SetConfigScreenMagnificationTriggerMethod(screenMagnificationTriggerMethod);
    if (Singleton<ExtendManagerServiceProxy>::GetInstance().LoadExtProxy()) {
        Singleton<ExtendManagerServiceProxy>::GetInstance().
            OnScreenMagnificationTriggerMethodChanged(screenMagnificationTriggerMethod);
    }
}

void AccessibleAbilityManagerService::RegisterScreenMagnificationState()
{
    HILOG_DEBUG();
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (accountData == nullptr) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }

        AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnScreenMagnificationStateChanged();
        };

        AccessibilitySettingObserver::UpdateFunc scaleFunc = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnScreenMagnificationScaleChanged();
        };
        AccessibilitySettingObserver::UpdateFunc triggerMethodFunc = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnScreenMagnificationTriggerMethodChanged();
        };
        if (accountData->GetConfig()->GetDbHandle()) {
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(SCREEN_MAGNIFICATION_KEY, func);
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(SCREEN_MAGNIFICATION_SCALE, scaleFunc);
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(SCREEN_MAGNIFICATION_TRIGGER_METHOD,
                triggerMethodFunc);
        }
        }, "REGISTER_SCREEN_ZOOM_OBSERVER");
}

void AccessibleAbilityManagerService::OnScreenMagnificationTypeChanged()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }

    if (config->GetDbHandle() == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }

    uint32_t screenMagnificationType = 0;
    screenMagnificationType =
        static_cast<uint32_t>(config->GetDbHandle()->GetIntValue(SCREEN_MAGNIFICATION_TYPE, 0));
    config->SetScreenMagnificationType(screenMagnificationType);
    if (Singleton<ExtendManagerServiceProxy>::GetInstance().LoadExtProxy()) {
        Singleton<ExtendManagerServiceProxy>::GetInstance().OnScreenMagnificationTypeChanged(screenMagnificationType);
    }
}

void AccessibleAbilityManagerService::RegisterScreenMagnificationType()
{
    HILOG_DEBUG();
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (accountData == nullptr) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }

        AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnScreenMagnificationTypeChanged();
        };
        if (accountData->GetConfig()->GetDbHandle()) {
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(SCREEN_MAGNIFICATION_TYPE, func);
        }
        }, "REGISTER_SCREEN_ZOOM_TYPE_OBSERVER");
}

void AccessibleAbilityManagerService::OnFlashReminderSwitchChanged()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }

    if (config->GetDbHandle() == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }

    bool flashReminderSwitch = config->GetDbHandle()->GetBoolValue(FLASH_REMINDER_SWITCH_KEY, false);
    config->SetFlashReminderSwitch(flashReminderSwitch);
    UpdateAccessibilityState();
}

void AccessibleAbilityManagerService::RegisterFlashReminderSwitch()
{
    HILOG_DEBUG();
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (accountData == nullptr) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }

        AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnFlashReminderSwitchChanged();
        };
        if (accountData->GetConfig()->GetDbHandle()) {
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(FLASH_REMINDER_SWITCH_KEY, func);
        }
        }, "FLASH_REMINDER_SWITCH_KEY_OBSERVER");
}

void AccessibleAbilityManagerService::OnSeniorModeStateChanged()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }

    if (config->GetDbHandle() == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }

    bool seniorModeState = config->GetDbHandle()->GetBoolValue(ELDER_CARE_ENABLED_KEY, false);
    config->SetSeniorModeState(seniorModeState);
    UpdateAccessibilityState();
}

void AccessibleAbilityManagerService::RegisterSeniorModeState()
{
    HILOG_DEBUG();
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (accountData == nullptr) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }

        AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnSeniorModeStateChanged();
        };
        if (accountData->GetConfig()->GetDbHandle()) {
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(ELDER_CARE_ENABLED_KEY, func);
        }
        }, "ELDER_CARE_ENABLED_KEY_OBSERVER");
}

void AccessibleAbilityManagerService::UpdateVoiceRecognitionState()
{
    HILOG_INFO();
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        {
            std::lock_guard<ffrt::mutex> lock(subscribeMSDPMutex_);
            if (isSubscribeMSDPCallback_) {
                MsdpManager::GetInstance().UnSubscribeVoiceRecognition();
                isSubscribeMSDPCallback_ = false;
                HILOG_INFO("userstatusClient.Unsubscribe");
            }
        }
        OnVoiceRecognitionChanged();
        }, "UPDATE_VOICE_RECOGNITION_STATE");
}

void AccessibleAbilityManagerService::OnVoiceRecognitionChanged()
{
    HILOG_INFO();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }

    if (config->GetDbHandle() == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }

    bool voiceRecognitionEnabled = config->GetDbHandle()->GetBoolValue(VOICE_RECOGNITION_KEY, false);
    std::string voiceRecognitionTypes = config->GetDbHandle()->GetStringValue(VOICE_RECOGNITION_TYPES, "DEFAULT");

    std::lock_guard<ffrt::mutex> lock(subscribeMSDPMutex_);
    if (!isSubscribeMSDPCallback_ && voiceRecognitionEnabled && voiceRecognitionTypes != "DEFAULT") {
        int32_t ret = MsdpManager::GetInstance().SubscribeVoiceRecognition();
        isSubscribeMSDPCallback_ = true;
        HILOG_INFO("SubscribeVoiceRecognition ret: %{public}d", ret);
        return;
    }

    if (isSubscribeMSDPCallback_ && (!voiceRecognitionEnabled || voiceRecognitionTypes == "DEFAULT")) {
        MsdpManager::GetInstance().UnSubscribeVoiceRecognition();
        isSubscribeMSDPCallback_ = false;
        return;
    }
}

void AccessibleAbilityManagerService::RegisterVoiceRecognitionState()
{
    HILOG_INFO();
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (accountData == nullptr) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }

        AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnVoiceRecognitionChanged();
        };

        if (accountData->GetConfig()->GetDbHandle()) {
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(VOICE_RECOGNITION_KEY, func);
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(VOICE_RECOGNITION_TYPES, func);
        }
        }, "REGISTER_VOICE_RECOGNITION");
}

void AccessibleAbilityManagerService::PostDelayUnloadTask()
{
#ifdef ACCESSIBILITY_WATCH_FEATURE
    auto task = [=]() {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            HILOG_ERROR("failed to get system ability mgr");
            return;
        }
        if (!IsNeedUnload()) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().PostDelayUnloadTask();
            return;
        }
        int32_t ret = systemAbilityManager->UnloadSystemAbility(ACCESSIBILITY_MANAGER_SERVICE_ID);
        if (ret != ERR_OK) {
            HILOG_ERROR("unload system ability failed");
            return;
        }
        SetParameter(SYSTEM_PARAMETER_AAMS_NAME, "false");
    };
    handler_->RemoveTask(DELAY_UNLOAD_TASK);
    handler_->PostTask(task, DELAY_UNLOAD_TASK, UNLOAD_TASK_INTERNAL);
#endif
}

bool AccessibleAbilityManagerService::IsNeedUnload()
{
    HILOG_DEBUG();
#ifndef ACCESSIBILITY_WATCH_FEATURE
    // always return true to avoid stablity problem
    return false;
#else // for watch
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return true;
    }

    // do not unload when any extension is enabled
    std::vector<std::string> enableAbilityList = accountData->GetEnabledAbilities();
    if (enableAbilityList.size() != 0) {
        return false;
    }
    return true;
#endif // ACCESSIBILITY_WATCH_FEATURE
}

ErrCode AccessibleAbilityManagerService::RemoveRequestId(int32_t requestId)
{
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_WARN("accountData is nullptr.");
        return -1;
    }
    accountData->GetElementOperatorManager().RemoveRequestId(requestId);
    return ERR_OK;
}

ErrCode AccessibleAbilityManagerService::GetRootParentId(int32_t windowId, int32_t treeId, int64_t &parentId)
{
    HILOG_INFO("aa search treeParent from aams,  windowId: %{public}d, treeId: %{public}d", windowId, treeId);
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("Get account data failed");
        return RET_ERR_NO_CONNECTION;
    }
    return accountData->GetElementOperatorManager().GetRootParentId(windowId, treeId, parentId);
}

ErrCode AccessibleAbilityManagerService::GetRootParentId(
    int32_t windowId, int32_t treeId, int64_t& parentId, bool systemApi)
{
    if (systemApi && !CheckPermission(OHOS_PERMISSION_ACCESSIBILITY_EXTENSION_ABILITY)) {
        HILOG_WARN("GetRootParentId permission denied.");
        return RET_ERR_NO_PERMISSION;
    }
    return GetRootParentId(windowId, treeId, parentId);
}

void AccessibleAbilityManagerService::OnDataClone()
{
    std::shared_ptr<AccessibilitySettingProvider> service =
        AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }
    bool cloneState = false;
    service->GetBoolValue(ACCESSIBILITY_CLONE_FLAG, cloneState);
    if (cloneState == false) {
        return;
    }
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_WARN("accountData is nullptr.");
        return;
    }
    if (accountData->GetConfig() != nullptr) {
        accountData->GetConfig()->OnDataClone();
        UpdateAllSetting();
        UpdateAutoStartAbilities();
        UpdateInputFilter();
        UpdateVoiceRecognitionState();
        HILOG_INFO("accessibility reload config.");
    } else {
        HILOG_WARN("config_ is nullptr");
    }
}

void AccessibleAbilityManagerService::RecycleEventHandler()
{
    HILOG_DEBUG();
    runner_.reset();
    handler_.reset();
    actionRunner_.reset();
    actionHandler_.reset();
    sendEventRunner_.reset();
    sendEventHandler_.reset();
    channelRunner_.reset();
    channelHandler_.reset();
    hoverEnterRunner_.reset();
    hoverEnterHandler_.reset();
    return;
}

std::shared_ptr<AccessibilityDatashareHelper> AccessibleAbilityManagerService::GetCurrentAcountDatashareHelper()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return nullptr;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return nullptr;
    }

    return config->GetDbHandle();
}

bool AccessibleAbilityManagerService::GetMagnificationState()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return false;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return false;
    }
    return config->GetScreenMagnificationState();
}

uint32_t AccessibleAbilityManagerService::GetMagnificationType()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return 0;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return 0;
    }
    return config->GetScreenMagnificationType();
}

uint32_t AccessibleAbilityManagerService::GetMagnificationMode()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return 0;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return 0;
    }
    return config->GetScreenMagnificationMode();
}

int32_t AccessibleAbilityManagerService::GetMagnificationTriggerMethod()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return 0;
    }
 
    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return 0;
    }
    return config->GetScreenMagnificationTriggerMethod();
}

void AccessibleAbilityManagerService::SetMagnificationMode(int32_t mode)
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }
    config->SetScreenMagnificationMode(mode);

    shared_ptr<AccessibilityDatashareHelper> helper = GetCurrentAcountDatashareHelper();
    if (helper == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }

    helper->PutIntValue(SCREEN_MAGNIFICATION_MODE, mode, false);
}

float AccessibleAbilityManagerService::GetMagnificationScale()
{
    HILOG_DEBUG();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return DEFAULT_SCALE;
    }

    std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
    if (config == nullptr) {
        HILOG_ERROR("config is nullptr");
        return DEFAULT_SCALE;
    }
    return config->GetScreenMagnificationScale();
}

void AccessibleAbilityManagerService::SetMagnificationScale(float scale)
{
    HILOG_DEBUG();
    SetConfigScreenMagnificationScale(scale);

    shared_ptr<AccessibilityDatashareHelper> helper = GetCurrentAcountDatashareHelper();
    if (helper == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }
    helper->PutFloatValue(SCREEN_MAGNIFICATION_SCALE, scale, false);
}

void AccessibleAbilityManagerService::InitResource(bool needReInit)
{
    if (isResourceInit_ && !needReInit) {
        HILOG_WARN("already init.");
        return;
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (resourceManager == nullptr) {
        HILOG_ERROR("resourceManager is null");
        return;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        HILOG_ERROR("resConfig is null");
        return;
    }
    std::map<std::string, std::string> configs;
    OHOS::Global::I18n::LocaleInfo locale(Global::I18n::LocaleConfig::GetSystemLocale(), configs);
    std::string language = locale.GetLanguage();
    std::string script = locale.GetScript();
    std::string region = locale.GetRegion();

    std::lock_guard<ffrt::mutex> lock(resourceMapMutex_);
    resConfig->SetLocaleInfo(language.c_str(), script.c_str(), region.c_str());
    resourceManager->UpdateResConfig(*resConfig);
    if (!resourceManager->AddResource(HAP_PATH)) {
        HILOG_ERROR("AddResource failed");
        return;
    }
    for (auto &iter : ResourceMap) {
        std::string outValue;
        if (iter.first == MAGNIFICATION_SCALE) {
            resourceManager->GetPluralStringByName(iter.first.c_str(), QUANTITY, outValue);
        } else {
            resourceManager->GetStringByName(iter.first.c_str(), outValue);
        }
        ResourceMap[iter.first] = outValue;
    }
    isResourceInit_ = true;
}

std::string &AccessibleAbilityManagerService::GetResource(const std::string &resourceName)
{
    std::lock_guard<ffrt::mutex> lock(resourceMapMutex_);
    return ResourceMap[resourceName];
}

ErrCode AccessibleAbilityManagerService::AnnouncedForAccessibility(const std::string &announcedText)
{
    HILOG_DEBUG();
    AccessibilityEventInfo event(TYPE_VIEW_ANNOUNCE_FOR_ACCESSIBILITY);
    event.SetBundleName(HAP_BUNDLE);
    event.SetTriggerAction(ACCESSIBILITY_ACTION_COMMON);
    event.SetTextAnnouncedForAccessibility(announcedText);
    AccessibilityEventInfoParcel eventParcel(event);
    return InnerSendEvent(eventParcel, 0, GetCurrentAccountId());
}

void AccessibleAbilityManagerService::AnnouncedForMagnification(AnnounceType announceType)
{
    bool state = false;
    GetScreenReaderState(state);
    if (!state) {
        HILOG_WARN("screenReader not enable.");
        return;
    }
    std::string resource = "";
    if (announceType == AnnounceType::ANNOUNCE_MAGNIFICATION_SCALE) {
        resource = GetResource(MAGNIFICATION_SCALE).c_str();
        std::ostringstream oss;
        float scale = GetMagnificationScale();
        oss << std::fixed << std::setprecision(1) << scale;
        std::string scaleStr = oss.str();
        std::string announceStr = Utils::FormatString(resource, scaleStr);
        AnnouncedForAccessibility(announceStr);
        return;
    }

    if (announceType == AnnounceType::ANNOUNCE_MAGNIFICATION_DISABLE) {
        resource = GetResource(MAGNIFICATION_DISABLE).c_str();
        AnnouncedForAccessibility(resource);
        return;
    }

    if (announceType == AnnounceType::ANNOUNCE_SWITCH_FULL_SCREEN) {
        resource = GetResource(SWITCH_FULL_SCREEN).c_str();
        AnnouncedForAccessibility(resource);
        return;
    }

    if (announceType == AnnounceType::ANNOUNCE_SWITCH_WINDOW) {
        resource = GetResource(SWITCH_WINDOW).c_str();
        AnnouncedForAccessibility(resource);
        return;
    }
}

int32_t AccessibleAbilityManagerService::SetEnhanceConfig(const AccessibilitySecCompRawdata& rawData)
{
    HILOG_INFO();
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType != TOKEN_NATIVE) {
        HILOG_ERROR("caller is not native.");
        return RET_ERR_TOKEN_ID;
    }

    if (IPCSkeleton::GetCallingUid() != SECURITY_COMPONENT_UID) {
        HILOG_ERROR("caller is not security component service.");
        return RET_ERR_TOKEN_ID;
    }
    int32_t result = AccessibilitySecurityComponentManager::SetEnhanceConfig(rawData);
    return result;
}

ErrCode AccessibleAbilityManagerService::SearchNeedEvents(std::vector<uint32_t> &needEvents)
{
    HILOG_DEBUG("SearchNeedEvents AAMS");
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("Account data is null");
        return ERR_INVALID_VALUE;
    }
    needEvents = accountData->GetNeedEvents();
    HILOG_DEBUG("GetNeedEvent size is %{public}zu", needEvents.size());
    return NO_ERROR;
}

void AccessibleAbilityManagerService::SubscribeOsAccount()
{
    HILOG_INFO();
    if (accountSubscriber_ != nullptr) {
        HILOG_ERROR("accountSubscriber is already registed!");
        return;
    }
    std::set<AccountSA::OsAccountState> states;
    states.insert(AccountSA::OsAccountState::SWITCHING);
    AccountSA::OsAccountSubscribeInfo info(states, false);
    accountSubscriber_ = std::make_shared<AccountSubscriber>(info);
    if (accountSubscriber_ == nullptr) {
        HILOG_ERROR("create accountSubscriber fail!");
        return;
    }
    auto result = AccountSA::OsAccountManager::SubscribeOsAccount(accountSubscriber_);
    if (result != ERR_OK) {
        HILOG_ERROR("fail to register subscriber, res:%{public}d.", result);
    }
}

void AccessibleAbilityManagerService::UnsubscribeOsAccount()
{
    HILOG_INFO();
    if (accountSubscriber_ == nullptr) {
        HILOG_ERROR("accountSubscriber is nullptr.");
        return;
    }
    auto res = AccountSA::OsAccountManager::UnsubscribeOsAccount(accountSubscriber_);
    if (res != ERR_OK) {
        HILOG_ERROR("unregister account event fail res:%{public}d", res);
    }
}

void AccessibleAbilityManagerService::RegisterPcModeSwitch()
{
    HILOG_INFO();
    if (system::GetParameter(PC_MODE_SUPPORT, "false") != "true") {
        HILOG_ERROR("not support switch pcmode");
        return;
    }
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([this]() {
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (accountData == nullptr) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }

        AccessibilitySettingObserver::UpdateFunc func = [this](const std::string &state) {
            sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
            if (accountData == nullptr) {
                HILOG_ERROR("accountData is nullptr");
                return;
            }
            accountData->GetWindowManager().Init();
        };
        if (accountData->GetConfig()->GetSystemDbHandle()) {
            accountData->GetConfig()->GetSystemDbHandle()->RegisterObserver(PC_MODE_SWITCH, func);
        }
        }, "REGISTER_PC_MODE_SWITCH_OBSERVER");
}

ErrCode AccessibleAbilityManagerService::GetReadableRules(std::string &readableRules)
{
    HILOG_INFO();
    sptr<AccessibilityAccountData> account = GetCurrentAccountData();
    if (!account) {
        HILOG_ERROR("account is nullptr");
        return RET_ERR_NULLPTR;
    }
    return account->GetAccessibleAbilityManager().GetReadableRules(readableRules);
}

ErrCode AccessibleAbilityManagerService::DeRegisterCaptionObserver(
    const sptr<IRemoteObject>& obj)
{
    HILOG_INFO();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    accountData->RemoveCaptionPropertyCallback(obj);
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::DeRegisterEnableAbilityListsObserver(
    const sptr<IRemoteObject>& obj)
{
    HILOG_INFO();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    accountData->RemoveEnableAbilityListsObserver(obj);
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::DeRegisterEnableAbilityCallbackObserver(
    const sptr<IRemoteObject>& obj)
{
    HILOG_INFO();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    accountData->RemoveEnableAbilityCallbackObserver(obj);
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::DeRegisterConfigObserver(
    const sptr<IRemoteObject>& obj)
{
    HILOG_INFO();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    accountData->RemoveConfigCallback(obj);
    return RET_OK;
}

ErrCode AccessibleAbilityManagerService::IsInnerWindowRootElement(int64_t elementId, bool &state)
{
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    state = accountData->GetWindowManager().IsInnerWindowRootElement(elementId);
    return ERR_OK;
}

void AccessibleAbilityManagerService::RegisterNotificationState()
{
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }
    AccessibilitySettingObserver::UpdateFunc transitionAnimationsfunc = [ = ](const std::string &state) {
        HILOG_INFO("on transition_animations_notification changed");
        TransitionAnimationsNotification::DestroyTimers();
    };
    AccessibilitySettingObserver::UpdateFunc ignoreRepeatfunc = [ = ](const std::string &state) {
        HILOG_INFO("on ignore_repeat_notification changed");
        IgnoreRepeatClickNotification::DestroyTimers();
    };
    if (accountData->GetConfig() && accountData->GetConfig()->GetDbHandle()) {
        accountData->GetConfig()->GetDbHandle()->RegisterObserver(IGNORE_REPEAT_CLICK_NOTIFICATION, ignoreRepeatfunc);
        accountData->GetConfig()->GetDbHandle()->RegisterObserver(
            TRANSITION_ANIMATIONS_NOTIFICATION, transitionAnimationsfunc);
    }
}

void AccessibleAbilityManagerService::OnSeniorModeStateForAppChanged()
{
    HILOG_INFO("OnSeniorModeStateForAppChanged");
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is nullptr");
        return;
    }

    std::map<std::string, bool> changes;
    accountData->GetSeniorModeStateForAppChanges(changes);

    for (const auto& item : changes) {
        std::string bundleName;
        int32_t appIndex = 0;
        if (!Utils::ParseSeniorModeStateKey(item.first, bundleName, appIndex)) {
            continue;
        }

        accountData->NotifySeniorModeStateObservers(bundleName, appIndex, item.second);
    }
}

void AccessibleAbilityManagerService::RegisterSeniorModeStateForAppObserver()
{
    HILOG_INFO();
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ is nullptr");
        return;
    }
    handler_->PostTask([=]() {
        HILOG_INFO("RegisterSeniorModeStateForAppObserver");
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (accountData == nullptr) {
            HILOG_ERROR("accountData is nullptr");
            return;
        }

        AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string &state) {
            Singleton<AccessibleAbilityManagerService>::GetInstance().OnSeniorModeStateForAppChanged();
        };

        if (accountData->GetConfig()->GetDbHandle()) {
            accountData->GetConfig()->GetDbHandle()->RegisterObserver(SENIOR_MODE_STATE_KEY, func);
        }
        }, "REGISTER_VOICE_RECOGNITION");
}

ErrCode AccessibleAbilityManagerService::RegisterSeniorModeStateObserver(
    const sptr<IAccessibilityAppSeniorModeStateObserver> &observer)
{
    HILOG_DEBUG();
    if (!observer || !actionHandler_) {
        HILOG_ERROR("Parameters check failed!");
        return ERR_INVALID_DATA;
    }
    XCollieHelper timer(TIMER_REGISTER_ENABLEABILITY_OBSERVER, XCOLLIE_TIMEOUT);
    std::shared_ptr<ffrt::promise<ErrCode>> syncPromisePtr = std::make_shared<ffrt::promise<ErrCode>>();
    ffrt::future syncFuture = syncPromisePtr->get_future();
    actionHandler_->PostTask([this, syncPromisePtr, observer]() {
        HILOG_DEBUG();
        sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
        if (!accountData) {
            HILOG_ERROR("Account data is null");
            syncPromisePtr->set_value(ERR_INVALID_DATA);
            return;
        }
        if (!seniorModeStateObserverDeathRecipient_) {
            seniorModeStateObserverDeathRecipient_ = new(std::nothrow) SeniorModeStateObserverDeathRecipient();
            if (!seniorModeStateObserverDeathRecipient_) {
                HILOG_ERROR("seniorModeStateObserverDeathRecipient_ is null");
                syncPromisePtr->set_value(ERR_INVALID_DATA);
                return;
            }
        }
        if (!observer->AsObject()) {
            HILOG_ERROR("object is null");
            syncPromisePtr->set_value(ERR_OK);
            return;
        }
        observer->AsObject()->AddDeathRecipient(seniorModeStateObserverDeathRecipient_);
        accountData->AddSeniorModeStateObserver(observer);
        syncPromisePtr->set_value(ERR_OK);
        }, "TASK_REGISTER_SENIOR_MODE_STATE_OBSERVER");

    ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIME_OUT_OPERATOR));
    if (wait != ffrt::future_status::ready) {
        HILOG_ERROR("Failed to wait RegisterSeniorModeStateObserver result");
        return ERR_TRANSACTION_FAILED;
    }
    return syncFuture.get();
}

ErrCode AccessibleAbilityManagerService::DeRegisterSeniorModeStateObserver(
    const sptr<IRemoteObject>& obj)
{
    HILOG_INFO();
    sptr<AccessibilityAccountData> accountData = GetCurrentAccountData();
    if (!accountData) {
        HILOG_ERROR("accountData is nullptr");
        return RET_ERR_NULLPTR;
    }
    accountData->RemoveSeniorModeStateObserver(obj);
    return RET_OK;
}

void AccessibleAbilityManagerService::SeniorModeStateObserverDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    Singleton<AccessibleAbilityManagerService>::GetInstance().RemoveCallback(
        SENIOR_MODE_STATE_CALLBACK, this, remote);
}

ErrCode AccessibleAbilityManagerService::GetSeniorModeStateForApp(bool &state)
{
    PostDelayUnloadTask();
    auto id = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::HapTokenInfo info;
    auto result = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(id, info);
    if (result != 0) {
        HILOG_ERROR("get native token info failed!, result: %{public}d", result);
        return RET_ERR_NULLPTR;
    }

    std::string bundleName = info.bundleName;
    auto instIndex = info.instIndex;
    HILOG_INFO("processName: %{public}s, instIndex: %{public}d", bundleName.c_str(), instIndex);
    return accessibilitySettings_->GetSeniorModeStateForApp(bundleName, instIndex, state);
}

ErrCode AccessibleAbilityManagerService::SetSeniorModeStateForApp(const bool state)
{
    PostDelayUnloadTask();
    auto id = IPCSkeleton::GetCallingTokenID();
    HILOG_INFO("id :%{public}d", id);
    Security::AccessToken::HapTokenInfo info;
    auto result = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(id, info);
    if (result != 0) {
        HILOG_ERROR("get native token info failed!, result: %{public}d", result);
    }

    std::string bundleName = info.bundleName;
    auto instIndex = info.instIndex;
    HILOG_INFO("processName: %{public}s, instIndex: %{public}d", bundleName.c_str(), instIndex);
    return accessibilitySettings_->SetSeniorModeStateForApp(bundleName, instIndex, state);
}

ErrCode AccessibleAbilityManagerService::GetSeniorModeStateForApp(const std::string &bundleName, int32_t appIndex,
    bool &state)
{
    HILOG_INFO();
    return accessibilitySettings_->GetSeniorModeStateForApp(bundleName, appIndex, state);
}

ErrCode AccessibleAbilityManagerService::SetSeniorModeStateForApp(
    const std::vector<AccessibilityBundleSeniorModeInfoParcel> &infos)
{
    auto ret = RET_OK;
    for (auto &info : infos) {
        auto tmpRet = accessibilitySettings_->SetSeniorModeStateForApp(info.bundleName_, info.appIndex_,
            info.seniorModeState_);
        if (tmpRet != RET_OK) {
            ret = tmpRet;
        }
    }
    return ret;
}
} // namespace Accessibility
} // namespace OHOS
