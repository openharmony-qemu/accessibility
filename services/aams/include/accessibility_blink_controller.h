// SPDX-License-Identifier: Apache-2.0
#ifndef OHOS_ACCESSIBILITY_BLINK_CONTROLLER_H
#define OHOS_ACCESSIBILITY_BLINK_CONTROLLER_H

#include <cstdint>
#include <functional>
#include <memory>
#include "accessibility_def.h"

namespace OHOS {
namespace Accessibility {
// Internal screen backend. Caller identity must come from synchronous Binder,
// after the service's existing system-app and WRITE_ACCESSIBILITY_CONFIG checks.
// It never represents camera torch support or accepts app-supplied identity.
class AccessibilityBlinkController final {
public:
    using ReadEnabled = std::function<RetError(bool &)>;
    explicit AccessibilityBlinkController(ReadEnabled readEnabled);
    ~AccessibilityBlinkController();
    RetError Control(bool start, int32_t mode, int32_t scenario, int32_t callerPid,
        uint32_t callerToken, int32_t &result);
    void Shutdown();
    void CancelForAccountChange();
    AccessibilityBlinkController(const AccessibilityBlinkController &) = delete;
    AccessibilityBlinkController &operator=(const AccessibilityBlinkController &) = delete;
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace Accessibility
} // namespace OHOS
#endif
