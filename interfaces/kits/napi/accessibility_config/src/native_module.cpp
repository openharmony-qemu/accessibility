/*
 * Copyright (C) 2022-2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "napi_accessibility_config.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "accessibility_config.h"
#include "napi_blink_control.h"

EXTERN_C_START
/*
 * function for module exports
 */
static napi_property_descriptor configDesc[] = {
    DECLARE_NAPI_FUNCTION("set", NAccessibilityConfig::SetConfig),
    DECLARE_NAPI_FUNCTION("get", NAccessibilityConfig::GetConfig),
    DECLARE_NAPI_FUNCTION("on", NAccessibilityConfig::SubscribeConfigObserver),
    DECLARE_NAPI_FUNCTION("off", NAccessibilityConfig::UnSubscribeConfigObserver),
};

static napi_value InitHighContrastText(napi_env env)
{
    napi_value highContrastTextValue = nullptr;
    napi_create_object(env, &highContrastTextValue);
    if (highContrastTextValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return highContrastTextValue;
    }
    NAPI_CALL(env, napi_define_properties(env, highContrastTextValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_HIGH_CONTRAST_TEXT);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, highContrastTextValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return highContrastTextValue;
}

static napi_value InitInvertColor(napi_env env)
{
    napi_value invertColorValue = nullptr;
    napi_create_object(env, &invertColorValue);
    if (invertColorValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return invertColorValue;
    }
    NAPI_CALL(env, napi_define_properties(env, invertColorValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_INVERT_COLOR);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, invertColorValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return invertColorValue;
}

static napi_value InitDaltonizationState(napi_env env)
{
    napi_value daltonizationStateValue = nullptr;
    napi_create_object(env, &daltonizationStateValue);
    if (daltonizationStateValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return daltonizationStateValue;
    }
    NAPI_CALL(env, napi_define_properties(env, daltonizationStateValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_DALTONIZATION_STATE);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, daltonizationStateValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return daltonizationStateValue;
}

static napi_value InitDaltonizationColorFilter(napi_env env)
{
    napi_value daltonizationColorFilterValue = nullptr;
    napi_create_object(env, &daltonizationColorFilterValue);
    if (daltonizationColorFilterValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return daltonizationColorFilterValue;
    }
    NAPI_CALL(env, napi_define_properties(env, daltonizationColorFilterValue,
                                          sizeof(configDesc) /
                                          sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_DALTONIZATION_COLOR_FILTER);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, daltonizationColorFilterValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return daltonizationColorFilterValue;
}

static napi_value InitContentTimeout(napi_env env)
{
    napi_value contentTimeoutValue = nullptr;
    napi_create_object(env, &contentTimeoutValue);
    if (contentTimeoutValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return contentTimeoutValue;
    }
    NAPI_CALL(env, napi_define_properties(env, contentTimeoutValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_CONTENT_TIMEOUT);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, contentTimeoutValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return contentTimeoutValue;
}

static napi_value InitAnimationOff(napi_env env)
{
    napi_value animationOffValue = nullptr;
    napi_create_object(env, &animationOffValue);
    if (animationOffValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return animationOffValue;
    }
    NAPI_CALL(env, napi_define_properties(env, animationOffValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_ANIMATION_OFF);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, animationOffValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return animationOffValue;
}

static napi_value InitBrightnessDiscount(napi_env env)
{
    napi_value brightnessDiscountValue = nullptr;
    napi_create_object(env, &brightnessDiscountValue);
    if (brightnessDiscountValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return brightnessDiscountValue;
    }
    NAPI_CALL(env, napi_define_properties(env, brightnessDiscountValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_BRIGHTNESS_DISCOUNT);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, brightnessDiscountValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return brightnessDiscountValue;
}

static napi_value InitScreenMagnifier(napi_env env)
{
    napi_value screenMagnifierValue = nullptr;
    napi_create_object(env, &screenMagnifierValue);
    if (screenMagnifierValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return screenMagnifierValue;
    }
    NAPI_CALL(env, napi_define_properties(env, screenMagnifierValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_SCREEN_MAGNIFICATION);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, screenMagnifierValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return screenMagnifierValue;
}

static napi_value InitAudioMono(napi_env env)
{
    napi_value audioMonoValue = nullptr;
    napi_create_object(env, &audioMonoValue);
    if (audioMonoValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return audioMonoValue;
    }
    NAPI_CALL(env, napi_define_properties(env, audioMonoValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_AUDIO_MONO);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, audioMonoValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return audioMonoValue;
}

static napi_value InitAudioBalance(napi_env env)
{
    napi_value audioBalanceValue = nullptr;
    napi_create_object(env, &audioBalanceValue);
    if (audioBalanceValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return audioBalanceValue;
    }
    NAPI_CALL(env, napi_define_properties(env, audioBalanceValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_AUDIO_BALANCE);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, audioBalanceValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return audioBalanceValue;
}

static napi_value InitMouseKey(napi_env env)
{
    napi_value mouseKeyValue = nullptr;
    napi_create_object(env, &mouseKeyValue);
    if (mouseKeyValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return mouseKeyValue;
    }
    NAPI_CALL(env, napi_define_properties(env, mouseKeyValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_MOUSE_KEY);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, mouseKeyValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return mouseKeyValue;
}

static napi_value InitMouseAutoClick(napi_env env)
{
    napi_value mouseAutoClickValue = nullptr;
    napi_create_object(env, &mouseAutoClickValue);
    if (mouseAutoClickValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return mouseAutoClickValue;
    }
    NAPI_CALL(env, napi_define_properties(env, mouseAutoClickValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_MOUSE_AUTOCLICK);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, mouseAutoClickValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return mouseAutoClickValue;
}

static napi_value InitShortKey(napi_env env)
{
    napi_value shortKeyValue = nullptr;
    napi_create_object(env, &shortKeyValue);
    if (shortKeyValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return shortKeyValue;
    }
    NAPI_CALL(env, napi_define_properties(env, shortKeyValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_SHORT_KEY);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, shortKeyValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return shortKeyValue;
}

static napi_value InitShortKeyTarget(napi_env env)
{
    napi_value shortKeyTargetValue = nullptr;
    napi_create_object(env, &shortKeyTargetValue);
    if (shortKeyTargetValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return shortKeyTargetValue;
    }
    NAPI_CALL(env, napi_define_properties(env, shortKeyTargetValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_SHORT_KEY_TARGET);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, shortKeyTargetValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return shortKeyTargetValue;
}

static napi_value InitShortKeyMultiTarget(napi_env env)
{
    napi_value shortKeyMultiTargetValue = nullptr;
    napi_create_object(env, &shortKeyMultiTargetValue);
    if (shortKeyMultiTargetValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return shortKeyMultiTargetValue;
    }
    NAPI_CALL(env, napi_define_properties(env, shortKeyMultiTargetValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_SHORT_KEY_MULTI_TARGET);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, shortKeyMultiTargetValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return shortKeyMultiTargetValue;
}

static napi_value InitCaptionsState(napi_env env)
{
    napi_value captionsValue = nullptr;
    napi_create_object(env, &captionsValue);
    if (captionsValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return captionsValue;
    }
    NAPI_CALL(env, napi_define_properties(env, captionsValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_CAPTION_STATE);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, captionsValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return captionsValue;
}

static napi_value InitCaptionsStyle(napi_env env)
{
    napi_value captionsStyleValue = nullptr;
    napi_create_object(env, &captionsStyleValue);
    if (captionsStyleValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return captionsStyleValue;
    }
    NAPI_CALL(env, napi_define_properties(env, captionsStyleValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_CAPTION_STYLE);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, captionsStyleValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return captionsStyleValue;
}

static napi_value InitClickResponseTime(napi_env env)
{
    napi_value clickResponseTimeValue = nullptr;
    napi_create_object(env, &clickResponseTimeValue);
    if (clickResponseTimeValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return clickResponseTimeValue;
    }
    NAPI_CALL(env, napi_define_properties(env, clickResponseTimeValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_CLICK_RESPONSE_TIME);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, clickResponseTimeValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return clickResponseTimeValue;
}

static napi_value InitIgnoreRepeatClickState(napi_env env)
{
    napi_value ignoreRepeatClickStateValue = nullptr;
    napi_create_object(env, &ignoreRepeatClickStateValue);
    if (ignoreRepeatClickStateValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return ignoreRepeatClickStateValue;
    }
    NAPI_CALL(env, napi_define_properties(env, ignoreRepeatClickStateValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_IGNORE_REPEAT_CLICK_STATE);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, ignoreRepeatClickStateValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return ignoreRepeatClickStateValue;
}

static napi_value InitIgnoreRepeatClickTime(napi_env env)
{
    napi_value ignoreRepeatClickTimeValue = nullptr;
    napi_create_object(env, &ignoreRepeatClickTimeValue);
    if (ignoreRepeatClickTimeValue == nullptr) {
        HILOG_ERROR("napi_create_object fail.");
        return ignoreRepeatClickTimeValue;
    }
    NAPI_CALL(env, napi_define_properties(env, ignoreRepeatClickTimeValue,
                                          sizeof(configDesc) / sizeof(configDesc[0]),
                                          configDesc));
    NAccessibilityConfigClass* nativeObj =
        new(std::nothrow) NAccessibilityConfigClass(OHOS::AccessibilityConfig::CONFIG_IGNORE_REPEAT_CLICK_TIME);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to create nativeObj.");
        return nullptr;
    }
    nativeObj->SetEnv(env);
    napi_status sts = napi_wrap(env, ignoreRepeatClickTimeValue, reinterpret_cast<void*>(nativeObj),
        NAccessibilityConfigClass::Destructor, nullptr, nullptr);
    if (sts != napi_ok) {
        delete nativeObj;
        nativeObj = nullptr;
        HILOG_ERROR("failed to wrap JS object");
        return nullptr;
    }
    return ignoreRepeatClickTimeValue;
}

static napi_value RegisterPublicBlinkEnums(napi_env env, napi_value exports)
{
    struct Member { const char *name; int32_t value; };
    const Member modes[] = {
        { "SINGLE_BLINK", 1 }, { "CONTINUOUS_BLINK", 2 },
    };
    const Member scenarios[] = {
        { "ALARM", 1 }, { "NOTIFICATION", 2 }, { "CALL", 3 }, { "TESTING", 4 },
    };
    const Member results[] = {
        { "SUCCESS", 0 }, { "ALREADY_FLASHING", 1 }, { "DEVICE_IN_USE", 2 },
        { "FLASH_BLINKING_UNSUPPORTED", 3 }, { "SCREEN_BLINKING_UNSUPPORTED", 4 },
        { "FEATURE_DISABLED", 5 },
    };
    const struct { const char *name; const Member *members; size_t count; } contracts[] = {
        { "BlinkingMode", modes, sizeof(modes) / sizeof(modes[0]) },
        { "BlinkingScenario", scenarios, sizeof(scenarios) / sizeof(scenarios[0]) },
        { "BlinkResultCode", results, sizeof(results) / sizeof(results[0]) },
    };
    for (const auto &contract : contracts) {
        napi_value object = nullptr;
        NAPI_CALL(env, napi_create_object(env, &object));
        for (size_t index = 0; index < contract.count; ++index) {
            napi_value value = nullptr;
            NAPI_CALL(env, napi_create_int32(env, contract.members[index].value, &value));
            NAPI_CALL(env, napi_set_named_property(env, object, contract.members[index].name, value));
        }
        NAPI_CALL(env, napi_set_named_property(env, exports, contract.name, object));
    }
    return exports;
}

static napi_value InitConfigModule(napi_env env, napi_value exports)
{
    NAPI_ASSERT(env, RegisterPublicBlinkEnums(env, exports) != nullptr, "failed to create blinking enum objects");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startBlinking", OHOS::Accessibility::StartBlinkingNapi),
        DECLARE_NAPI_FUNCTION("stopBlinking", OHOS::Accessibility::StopBlinkingNapi),
        DECLARE_NAPI_FUNCTION("on", NAccessibilityConfig::SubscribeState),
        DECLARE_NAPI_FUNCTION("off", NAccessibilityConfig::UnsubscribeState),
        DECLARE_NAPI_FUNCTION("enableAbility", NAccessibilityConfig::EnableAbility),
        DECLARE_NAPI_FUNCTION(
            "enableAbilityWithCallback", NAccessibilityConfig::EnableAbilityWithCallback),
        DECLARE_NAPI_FUNCTION("disableAbility", NAccessibilityConfig::DisableAbility),
        DECLARE_NAPI_FUNCTION("setMagnificationState", NAccessibilityConfig::SetMagnificationState),
        DECLARE_NAPI_FUNCTION("onSeniorModeStateChangeForApp", NAccessibilityConfig::SubscribeSelfSeniorMode),
        DECLARE_NAPI_FUNCTION("offSeniorModeStateChangeForApp", NAccessibilityConfig::UnsubscribeSelfSeniorMode),
        DECLARE_NAPI_FUNCTION("getSeniorModeStateForApp", NAccessibilityConfig::GetSeniorModeStateForApp),
        DECLARE_NAPI_FUNCTION("setSeniorModeStateForApp", NAccessibilityConfig::SetSeniorModeStateForApp),
        DECLARE_NAPI_STATIC_PROPERTY("highContrastText", InitHighContrastText(env)),
        DECLARE_NAPI_STATIC_PROPERTY("invertColor", InitInvertColor(env)),
        DECLARE_NAPI_STATIC_PROPERTY("daltonizationState", InitDaltonizationState(env)),
        DECLARE_NAPI_STATIC_PROPERTY("daltonizationColorFilter", InitDaltonizationColorFilter(env)),
        DECLARE_NAPI_STATIC_PROPERTY("contentTimeout", InitContentTimeout(env)),
        DECLARE_NAPI_STATIC_PROPERTY("animationOff", InitAnimationOff(env)),
        DECLARE_NAPI_STATIC_PROPERTY("brightnessDiscount", InitBrightnessDiscount(env)),
        DECLARE_NAPI_STATIC_PROPERTY("screenMagnification", InitScreenMagnifier(env)),
        DECLARE_NAPI_STATIC_PROPERTY("audioMono", InitAudioMono(env)),
        DECLARE_NAPI_STATIC_PROPERTY("audioBalance", InitAudioBalance(env)),
        DECLARE_NAPI_STATIC_PROPERTY("mouseKey", InitMouseKey(env)),
        DECLARE_NAPI_STATIC_PROPERTY("mouseAutoClick", InitMouseAutoClick(env)),
        DECLARE_NAPI_STATIC_PROPERTY("shortkey", InitShortKey(env)),
        DECLARE_NAPI_STATIC_PROPERTY("shortkeyTarget", InitShortKeyTarget(env)),
        DECLARE_NAPI_STATIC_PROPERTY("shortkeyMultiTargets", InitShortKeyMultiTarget(env)),
        DECLARE_NAPI_STATIC_PROPERTY("captions", InitCaptionsState(env)),
        DECLARE_NAPI_STATIC_PROPERTY("captionsStyle", InitCaptionsStyle(env)),
        DECLARE_NAPI_STATIC_PROPERTY("clickResponseTime", InitClickResponseTime(env)),
        DECLARE_NAPI_STATIC_PROPERTY("ignoreRepeatClick", InitIgnoreRepeatClickState(env)),
        DECLARE_NAPI_STATIC_PROPERTY("repeatClickInterval", InitIgnoreRepeatClickTime(env)),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    auto &instance = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
    (void)instance.InitializeContext();
    NAccessibilityConfig::configObservers_->SubscribeToFramework();
    NAccessibilityConfig::enableAbilityListsObservers_->SubscribeToFramework();
    NAccessibilityConfig::enableAbilityCallbackObservers_->SubscribeToFramework();
    NAccessibilityConfig::seniorModeStateObservers_->SubscribeToFramework();

    HILOG_INFO("-----Init config module end------");
    return exports;
}
EXTERN_C_END

/*
 * Module define
 */
static napi_module _config_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitConfigModule,
    .nm_modname = "accessibility.config",
    .nm_priv = ((void*)0),
    .reserved = {0},
};
/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterNapiAccessibilityConfigModule(void)
{
    napi_module_register(&_config_module);
}
