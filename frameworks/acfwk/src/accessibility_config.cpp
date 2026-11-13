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

#include "accessibility_config.h"
#include "accessibility_config_impl.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AccessibilityConfig {
#define CHECK_IMPL_PTR(ret_expr)            \
    if (!pImpl_) {                          \
        HILOG_ERROR("pImpl_ is nullptr");   \
        return ret_expr;                    \
    }

ACCESSIBILITY_DEFINE_SINGLETON(AccessibilityConfig)

AccessibilityConfig::AccessibilityConfig() : pImpl_(std::make_unique<Impl>())
{
    if (!pImpl_) {
        HILOG_ERROR("Create AccessibilityConfigImpl Failed");
    }
}

AccessibilityConfig::~AccessibilityConfig()
{
}

bool AccessibilityConfig::InitializeContext()
{
    CHECK_IMPL_PTR(false)
    return pImpl_->InitializeContext();
}

Accessibility::RetError AccessibilityConfig::SubscribeConfigObserver(const CONFIG_ID id,
    const std::shared_ptr<AccessibilityConfigObserver> &observer, const bool retFlag)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SubscribeConfigObserver(id, observer, retFlag);
}

Accessibility::RetError AccessibilityConfig::UnsubscribeConfigObserver(const CONFIG_ID id,
    const std::shared_ptr<AccessibilityConfigObserver> &observer)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->UnsubscribeConfigObserver(id, observer);
}

Accessibility::RetError AccessibilityConfig::SubscribeEnableAbilityCallbackObserver(
    const std::shared_ptr<AccessibilityEnableAbilityCallbackObserver> &observer)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SubscribeEnableAbilityCallbackObserver(observer);
}

Accessibility::RetError AccessibilityConfig::UnsubscribeEnableAbilityCallbackObserver(
    const std::shared_ptr<AccessibilityEnableAbilityCallbackObserver> &observer)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->UnsubscribeEnableAbilityCallbackObserver(observer);
}

Accessibility::RetError AccessibilityConfig::SubscribeEnableAbilityListsObserver(
    const std::shared_ptr<AccessibilityEnableAbilityListsObserver> &observer)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SubscribeEnableAbilityListsObserver(observer);
}

Accessibility::RetError AccessibilityConfig::UnsubscribeEnableAbilityListsObserver(
    const std::shared_ptr<AccessibilityEnableAbilityListsObserver> &observer)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->UnsubscribeEnableAbilityListsObserver(observer);
}

Accessibility::RetError AccessibilityConfig::EnableAbility(
    const std::string &name, const uint32_t capabilities, const bool connectCallBackFlag)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->EnableAbility(name, capabilities, connectCallBackFlag);
}

Accessibility::RetError AccessibilityConfig::DisableAbility(const std::string &name)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->DisableAbility(name);
}

Accessibility::RetError AccessibilityConfig::SetMagnificationState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetMagnificationState(state);
}

Accessibility::RetError AccessibilityConfig::SetScreenMagnificationState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetScreenMagnificationState(state);
}

Accessibility::RetError AccessibilityConfig::SetShortKeyState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetShortKeyState(state);
}

Accessibility::RetError AccessibilityConfig::SetMouseKeyState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetMouseKeyState(state);
}

Accessibility::RetError AccessibilityConfig::SetCaptionsState(const bool state, bool isPermissionRequired)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetCaptionsState(state, isPermissionRequired);
}

Accessibility::RetError AccessibilityConfig::SetCaptionsProperty(const CaptionProperty &caption,
    bool isPermissionRequired)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetCaptionsProperty(caption, isPermissionRequired);
}

Accessibility::RetError AccessibilityConfig::SetMouseAutoClick(const int32_t time)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetMouseAutoClick(time);
}

Accessibility::RetError AccessibilityConfig::SetShortkeyTarget(const std::string &name)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetShortkeyTarget(name);
}

Accessibility::RetError AccessibilityConfig::SetShortkeyMultiTarget(const std::vector<std::string> &name)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetShortkeyMultiTarget(name);
}

Accessibility::RetError AccessibilityConfig::SetHighContrastTextState(bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetHighContrastTextState(state);
}

Accessibility::RetError AccessibilityConfig::SetInvertColorState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetInvertColorState(state);
}

Accessibility::RetError AccessibilityConfig::SetDaltonizationState(bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetDaltonizationState(state);
}

Accessibility::RetError AccessibilityConfig::SetDaltonizationColorFilter(const DALTONIZATION_TYPE type)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetDaltonizationColorFilter(type);
}

Accessibility::RetError AccessibilityConfig::SetContentTimeout(const uint32_t timer)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetContentTimeout(timer);
}

Accessibility::RetError AccessibilityConfig::SetAnimationOffState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetAnimationOffState(state);
}

Accessibility::RetError AccessibilityConfig::SetBrightnessDiscount(const float brightness)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetBrightnessDiscount(brightness);
}

Accessibility::RetError AccessibilityConfig::SetAudioMonoState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetAudioMonoState(state);
}

Accessibility::RetError AccessibilityConfig::SetAudioBalance(const float balance)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetAudioBalance(balance);
}

Accessibility::RetError AccessibilityConfig::SetClickResponseTime(const CLICK_RESPONSE_TIME time)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetClickResponseTime(time);
}

Accessibility::RetError AccessibilityConfig::SetIgnoreRepeatClickState(const bool state)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetIgnoreRepeatClickState(state);
}

Accessibility::RetError AccessibilityConfig::SetIgnoreRepeatClickTime(const IGNORE_REPEAT_CLICK_TIME time)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetIgnoreRepeatClickTime(time);
}

Accessibility::RetError AccessibilityConfig::ControlBlinking(bool start, int32_t mode, int32_t scenario,
    int32_t &result)
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->ControlBlinking(start, mode, scenario, result);
}

Accessibility::RetError AccessibilityConfig::GetScreenMagnificationState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetScreenMagnificationState(state);
}

Accessibility::RetError AccessibilityConfig::GetShortKeyState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetShortKeyState(state);
}

Accessibility::RetError AccessibilityConfig::GetMouseKeyState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetMouseKeyState(state);
}

Accessibility::RetError AccessibilityConfig::GetCaptionsState(bool &state, bool isPermissionRequired) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetCaptionsState(state, isPermissionRequired);
}

Accessibility::RetError AccessibilityConfig::GetCaptionsProperty(CaptionProperty &caption,
    bool isPermissionRequired) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetCaptionsProperty(caption, isPermissionRequired);
}

Accessibility::RetError AccessibilityConfig::GetMouseAutoClick(int32_t &time) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetMouseAutoClick(time);
}

Accessibility::RetError AccessibilityConfig::GetShortkeyTarget(std::string &name) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetShortkeyTarget(name);
}

Accessibility::RetError AccessibilityConfig::GetShortkeyMultiTarget(std::vector<std::string> &name) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetShortkeyMultiTarget(name);
}

Accessibility::RetError AccessibilityConfig::GetInvertColorState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetInvertColorState(state);
}

Accessibility::RetError AccessibilityConfig::GetHighContrastTextState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetHighContrastTextState(state);
}

Accessibility::RetError AccessibilityConfig::GetDaltonizationState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetDaltonizationState(state);
}

Accessibility::RetError AccessibilityConfig::GetDaltonizationColorFilter(DALTONIZATION_TYPE &type) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetDaltonizationColorFilter(type);
}

Accessibility::RetError AccessibilityConfig::GetContentTimeout(uint32_t &timer) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetContentTimeout(timer);
}

Accessibility::RetError AccessibilityConfig::GetAnimationOffState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetAnimationOffState(state);
}

Accessibility::RetError AccessibilityConfig::GetBrightnessDiscount(float &brightness) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetBrightnessDiscount(brightness);
}

Accessibility::RetError AccessibilityConfig::GetAudioMonoState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetAudioMonoState(state);
}

Accessibility::RetError AccessibilityConfig::GetAudioBalance(float &balance) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetAudioBalance(balance);
}

Accessibility::RetError AccessibilityConfig::GetClickResponseTime(CLICK_RESPONSE_TIME &time) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetClickResponseTime(time);
}

Accessibility::RetError AccessibilityConfig::GetIgnoreRepeatClickState(bool &state) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetIgnoreRepeatClickState(state);
}

Accessibility::RetError AccessibilityConfig::GetIgnoreRepeatClickTime(IGNORE_REPEAT_CLICK_TIME &time) const
{
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetIgnoreRepeatClickTime(time);
}

Accessibility::RetError AccessibilityConfig::SetEnhanceConfig(uint8_t *cfg, uint32_t cfgLen) const
{
    HILOG_INFO();
    if (cfg == nullptr || cfgLen <= 0) {
        HILOG_ERROR("SecCompEnhance cfg info is empty");
        return Accessibility::RET_ERR_INVALID_PARAM;
    }
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetEnhanceConfig(cfg, cfgLen);
}

Accessibility::RetError AccessibilityConfig::GetSeniorModeStateForApp(const std::string &bundleName,
    int32_t appIndex, bool &state) const
{
    HILOG_INFO();
    if (appIndex < 0) {
        HILOG_ERROR("appIndex < 0");
        return Accessibility::RET_ERR_INVALID_PARAM;
    }
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->GetSeniorModeStateForApp(bundleName, appIndex, state);
}

Accessibility::RetError AccessibilityConfig::SetSeniorModeStateForApp(
    const std::vector<AccessibilityBundleSeniorModeInfo> &infos)
{
    HILOG_INFO();
    if (infos.size() == 0) {
        HILOG_ERROR("infos is empty");
        return Accessibility::RET_ERR_INVALID_PARAM;
    }
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SetSeniorModeStateForApp(infos);
}

Accessibility::RetError AccessibilityConfig::SubscribeAppSeniorModeStateObserver(
    const std::shared_ptr<AccessibilityAppSeniorModeStateObserver> &observer)
{
    HILOG_INFO();
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->SubscribeAppSeniorModeStateObserver(observer);
}

Accessibility::RetError AccessibilityConfig::UnsubscribeAppSeniorModeStateObserver(
    const std::shared_ptr<AccessibilityAppSeniorModeStateObserver> &observer)
{
    HILOG_INFO();
    CHECK_IMPL_PTR(Accessibility::RET_ERR_NULLPTR)
    return pImpl_->UnsubscribeAppSeniorModeStateObserver(observer);
}
} // Accessibility
} // OHOS
