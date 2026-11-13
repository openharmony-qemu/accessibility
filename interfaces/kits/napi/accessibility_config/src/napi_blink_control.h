// SPDX-License-Identifier: Apache-2.0
#ifndef OHOS_NAPI_BLINK_CONTROL_H
#define OHOS_NAPI_BLINK_CONTROL_H
#include <cmath>
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "napi_accessibility_config.h"
#include "accessibility_config.h"

namespace OHOS::Accessibility {
inline napi_value BlinkFailure(napi_env env, RetError code)
{
    napi_value error = AccessibilityNapi::CreateBusinessError(env, code);
    if (error != nullptr) {
        napi_throw(env, error);
    } else {
        bool pending = false;
        napi_is_exception_pending(env, &pending);
        if (!pending) {
            napi_throw_error(env, nullptr, "Unable to construct accessibility BusinessError");
        }
    }
    return nullptr;
}

inline napi_value ControlBlinkingNapi(napi_env env, napi_callback_info info, bool start)
{
    using namespace Security::AccessToken;
    if (!AccessTokenKit::IsSystemAppByFullTokenID(IPCSkeleton::GetCallingFullTokenID())) {
        return BlinkFailure(env, RET_ERR_NOT_SYSTEM_APP);
    }
    if (AccessTokenKit::VerifyAccessToken(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.WRITE_ACCESSIBILITY_CONFIG") != PERMISSION_GRANTED) {
        return BlinkFailure(env, RET_ERR_NO_PERMISSION);
    }
    size_t count = 3;
    napi_value args[3] = {nullptr, nullptr, nullptr};
    if (napi_get_cb_info(env, info, &count, args, nullptr, nullptr) != napi_ok || count != 2) {
        return BlinkFailure(env, RET_ERR_INVALID_PARAM);
    }
    double mode = 0, scenario = 0;
    if (napi_get_value_double(env, args[0], &mode) != napi_ok ||
        napi_get_value_double(env, args[1], &scenario) != napi_ok ||
        !std::isfinite(mode) || !std::isfinite(scenario) ||
        (mode != 1 && mode != 2) || scenario < 1 || scenario > 4 || std::floor(scenario) != scenario) {
        return BlinkFailure(env, RET_ERR_INVALID_PARAM);
    }
    int32_t result = 0;
    auto &config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    RetError error = config.ControlBlinking(start, static_cast<int32_t>(mode),
        static_cast<int32_t>(scenario), result);
    if (error != RET_OK) {
        return BlinkFailure(env, error);
    }
    napi_value value = nullptr;
    if (napi_create_int32(env, result, &value) != napi_ok) {
        return BlinkFailure(env, RET_ERR_FAILED);
    }
    return value;
}
inline napi_value StartBlinkingNapi(napi_env env, napi_callback_info info)
{
    return ControlBlinkingNapi(env, info, true);
}
inline napi_value StopBlinkingNapi(napi_env env, napi_callback_info info)
{
    return ControlBlinkingNapi(env, info, false);
}
} // namespace OHOS::Accessibility
#endif
