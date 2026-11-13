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

#ifndef ACCESSIBILITY_CONFIG_IMPL_H
#define ACCESSIBILITY_CONFIG_IMPL_H

#include <atomic>
#include "accessibility_config.h"
#include "accessibility_enable_ability_lists_observer_stub.h"
#include "accessibility_enable_ability_callback_observer_stub.h"
#include "accessible_ability_manager_caption_observer_stub.h"
#include "accessible_ability_manager_config_observer_stub.h"
#include "accessibility_senior_mode_state_observer_stub.h"
#include "accessible_ability_manager_service_proxy.h"
#include "ffrt.h"
#include "ffrt_inner.h"
#include "event_handler.h"
#include "refbase.h"
#include "system_ability_load_callback_stub.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace AccessibilityConfig {
class AccessibilityConfig::Impl {
public:
    Impl();
    ~Impl();

    bool InitializeContext();

    Accessibility::RetError SubscribeConfigObserver(const CONFIG_ID id,
        const std::shared_ptr<AccessibilityConfigObserver> &observer, const bool retFlag);
    Accessibility::RetError UnsubscribeConfigObserver(const CONFIG_ID id,
        const std::shared_ptr<AccessibilityConfigObserver> &observer);

    Accessibility::RetError SubscribeEnableAbilityListsObserver(
        const std::shared_ptr<AccessibilityEnableAbilityListsObserver> &observer);
    Accessibility::RetError UnsubscribeEnableAbilityListsObserver(
        const std::shared_ptr<AccessibilityEnableAbilityListsObserver> &observer);

    Accessibility::RetError SubscribeEnableAbilityCallbackObserver(
        const std::shared_ptr<AccessibilityEnableAbilityCallbackObserver> &observer);
    Accessibility::RetError UnsubscribeEnableAbilityCallbackObserver(
        const std::shared_ptr<AccessibilityEnableAbilityCallbackObserver> &observer);

    Accessibility::RetError SubscribeAppSeniorModeStateObserver(
        const std::shared_ptr<AccessibilityAppSeniorModeStateObserver> &observer);
    Accessibility::RetError UnsubscribeAppSeniorModeStateObserver(
        const std::shared_ptr<AccessibilityAppSeniorModeStateObserver> &observer);

    Accessibility::RetError EnableAbility(
        const std::string &name, const uint32_t capabilities, const bool connectCallBackFlag);
    Accessibility::RetError DisableAbility(const std::string &name);
    Accessibility::RetError SetMagnificationState(const bool state);

    Accessibility::RetError SetScreenMagnificationState(const bool state);
    Accessibility::RetError SetShortKeyState(const bool state);
    Accessibility::RetError SetMouseKeyState(const bool state);
    Accessibility::RetError SetCaptionsState(const bool state, bool isPermissionRequired);
    Accessibility::RetError SetCaptionsProperty(const CaptionProperty &caption, bool isPermissionRequired);
    Accessibility::RetError SetMouseAutoClick(const int32_t time);
    Accessibility::RetError SetShortkeyTarget(const std::string &name);
    Accessibility::RetError SetShortkeyMultiTarget(const std::vector<std::string> &name);
    Accessibility::RetError SetHighContrastTextState(bool state);
    Accessibility::RetError SetInvertColorState(const bool state);
    Accessibility::RetError SetDaltonizationState(bool state);
    Accessibility::RetError SetDaltonizationColorFilter(const DALTONIZATION_TYPE type);
    Accessibility::RetError SetContentTimeout(const uint32_t timer);
    Accessibility::RetError SetAnimationOffState(const bool state);
    Accessibility::RetError SetBrightnessDiscount(const float brightness);
    Accessibility::RetError SetAudioMonoState(const bool state);
    Accessibility::RetError SetAudioBalance(const float balance);
    Accessibility::RetError SetClickResponseTime(const CLICK_RESPONSE_TIME time);
    Accessibility::RetError SetIgnoreRepeatClickState(const bool state);
    Accessibility::RetError SetIgnoreRepeatClickTime(const IGNORE_REPEAT_CLICK_TIME time);
    Accessibility::RetError ControlBlinking(bool start, int32_t mode, int32_t scenario, int32_t &result);

    Accessibility::RetError GetScreenMagnificationState(bool &state);
    Accessibility::RetError GetShortKeyState(bool &state);
    Accessibility::RetError GetMouseKeyState(bool &state);
    Accessibility::RetError GetCaptionsState(bool &state, bool isPermissionRequired);
    Accessibility::RetError GetCaptionsProperty(CaptionProperty &caption, bool isPermissionRequired);
    Accessibility::RetError GetMouseAutoClick(int32_t &time);
    Accessibility::RetError GetShortkeyTarget(std::string &name);
    Accessibility::RetError GetShortkeyMultiTarget(std::vector<std::string> &name);
    Accessibility::RetError GetInvertColorState(bool &state);
    Accessibility::RetError GetHighContrastTextState(bool &state);
    Accessibility::RetError GetDaltonizationState(bool &state);
    Accessibility::RetError GetDaltonizationColorFilter(DALTONIZATION_TYPE &type);
    Accessibility::RetError GetContentTimeout(uint32_t &timer);
    Accessibility::RetError GetAnimationOffState(bool &state);
    Accessibility::RetError GetBrightnessDiscount(float &brightness);
    Accessibility::RetError GetAudioMonoState(bool &state);
    Accessibility::RetError GetAudioBalance(float &balance);
    Accessibility::RetError GetClickResponseTime(CLICK_RESPONSE_TIME &time);
    Accessibility::RetError GetIgnoreRepeatClickState(bool &state);
    Accessibility::RetError GetIgnoreRepeatClickTime(IGNORE_REPEAT_CLICK_TIME &time);
    Accessibility::RetError SetEnhanceConfig(uint8_t *cfg, uint32_t cfgLen);
    Accessibility::RetError GetSeniorModeStateForApp(const std::string &bundleName, int32_t appIndex, bool &state);
    Accessibility::RetError SetSeniorModeStateForApp(const std::vector<AccessibilityBundleSeniorModeInfo> &infos);

    void ResetService(const wptr<IRemoteObject> &remote);
    void OnAccessibleAbilityManagerCaptionPropertyChanged(const CaptionProperty &property);
    void OnAccessibleAbilityManagerConfigStateChanged(const uint32_t stateType);
    void OnAccessibleAbilityManagerAudioBalanceChanged(const float audioBalance);
    void OnAccessibleAbilityManagerBrightnessDiscountChanged(const float brightnessDiscount);
    void OnAccessibleAbilityManagerContentTimeoutChanged(const uint32_t contentTimeout);
    void OnAccessibleAbilityManagerDaltonizationColorFilterChanged(const uint32_t filterType);
    void OnAccessibleAbilityManagerMouseAutoClickChanged(const int32_t mouseAutoClick);
    void OnAccessibleAbilityManagerShortkeyTargetChanged(const std::string &shortkeyTarget);
    void OnAccessibleAbilityManagerShortkeyMultiTargetChanged(const std::vector<std::string> &shortkeyMultiTarget);
    void OnAccessibleAbilityManagerClickResponseTimeChanged(const uint32_t clickResponseTime);
    void OnAccessibleAbilityManagerIgnoreRepeatClickTimeChanged(const uint32_t ignoreRepeatClickTime);
    void OnAccessibilityEnableAbilityListsChanged();
    void OnAccessibilityInstallAbilityListsChanged();
    void OnEnableAbilityRemoteDied(const std::string& name);
private:
    class AccessibilityEnableAbilityListsObserverImpl :
        public Accessibility::AccessibilityEnableAbilityListsObserverStub {
    public:
        explicit AccessibilityEnableAbilityListsObserverImpl(Impl &client)
            : client_(client) {}
        ~AccessibilityEnableAbilityListsObserverImpl() = default;

        virtual void OnAccessibilityEnableAbilityListsChanged() override
        {
            if (enableAbilityClientDeleted_ == false) {
                client_.OnAccessibilityEnableAbilityListsChanged();
            }
        }

        virtual void OnAccessibilityInstallAbilityListsChanged() override
        {
            if (enableAbilityClientDeleted_ == false) {
                client_.OnAccessibilityInstallAbilityListsChanged();
            }
        }

        void OnclientDeleted()
        {
            enableAbilityClientDeleted_ = true;
        }
    private:
        Impl &client_;
        std::atomic<bool> enableAbilityClientDeleted_ = false;
    };

    class AccessibilityEnableAbilityCallbackObserverImpl :
        public Accessibility::AccessibilityEnableAbilityCallbackObserverStub {
    public:
        explicit AccessibilityEnableAbilityCallbackObserverImpl(Impl &client)
            : client_(client) {}
        ~AccessibilityEnableAbilityCallbackObserverImpl() = default;

        virtual void OnEnableAbilityRemoteDied(const std::string& name)
        {
            client_.OnEnableAbilityRemoteDied(name);
        }

    private:
        Impl &client_;
    };

    class AccessibleAbilityManagerCaptionObserverImpl
        : public Accessibility::AccessibleAbilityManagerCaptionObserverStub {
    public:
        explicit AccessibleAbilityManagerCaptionObserverImpl(Impl &config_)
            : config_(config_) {}
        ~AccessibleAbilityManagerCaptionObserverImpl() = default;

        virtual void OnPropertyChanged(const CaptionProperty &property) override
        {
            if (captionClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerCaptionPropertyChanged(property);
            }
        }

        void OnclientDeleted()
        {
            captionClientDeleted_ = true;
        }
    private:
        Impl &config_;
        std::atomic<bool> captionClientDeleted_ = false;
    };

    class AccessibilityLoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        explicit AccessibilityLoadCallback(Impl* config) : config_(config) {}
        ~AccessibilityLoadCallback() = default;
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
            const sptr<IRemoteObject> &remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    private:
        Impl* config_ = nullptr;
    };

    class AccessibleAbilityManagerConfigObserverImpl
        : public Accessibility::AccessibleAbilityManagerConfigObserverStub {
    public:
        explicit AccessibleAbilityManagerConfigObserverImpl(Impl &config) : config_(config) {}
        ~AccessibleAbilityManagerConfigObserverImpl() = default;

        virtual void OnConfigStateChanged(const uint32_t stateType) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerConfigStateChanged(stateType);
            }
        }
        virtual void OnAudioBalanceChanged(const float audioBalance) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerAudioBalanceChanged(audioBalance);
            }
        }
        virtual void OnBrightnessDiscountChanged(const float brightnessDiscount) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerBrightnessDiscountChanged(brightnessDiscount);
            }
        }
        virtual void OnContentTimeoutChanged(const uint32_t contentTimeout) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerContentTimeoutChanged(contentTimeout);
            }
        }
        virtual void OnDaltonizationColorFilterChanged(const uint32_t filterType) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerDaltonizationColorFilterChanged(filterType);
            }
        }
        virtual void OnMouseAutoClickChanged(const int32_t mouseAutoClick) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerMouseAutoClickChanged(mouseAutoClick);
            }
        }
        virtual void OnShortkeyTargetChanged(const std::string &shortkeyTarget) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerShortkeyTargetChanged(shortkeyTarget);
            }
        }
        virtual void OnShortkeyMultiTargetChanged(const std::vector<std::string> &shortkeyMultiTarget) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerShortkeyMultiTargetChanged(shortkeyMultiTarget);
            }
        }
        virtual void OnClickResponseTimeChanged(const uint32_t clickResponseTime) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerClickResponseTimeChanged(clickResponseTime);
            }
        }
        virtual void OnIgnoreRepeatClickTimeChanged(const uint32_t ignoreRepeatClickTime) override
        {
            if (configClientDeleted_ == false) {
                config_.OnAccessibleAbilityManagerIgnoreRepeatClickTimeChanged(ignoreRepeatClickTime);
            }
        }
        
        void OnclientDeleted()
        {
            configClientDeleted_ = true;
        }

    private:
        Impl &config_;
        std::atomic<bool> configClientDeleted_ = false;
    };

    class DeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit DeathRecipient(Impl &config_) : config_(config_) {}
        ~DeathRecipient() = default;
        DISALLOW_COPY_AND_MOVE(DeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote)
        {
            config_.ResetService(remote);
        }
    private:
        Impl &config_;
    };

    class AccessibilityAppSeniorModeStateObserverImpl
        : public Accessibility::AccessibilityAppSeniorModeStateObserverStub {
    public:
        explicit AccessibilityAppSeniorModeStateObserverImpl(Impl &config) : config_(config) {}
        ~AccessibilityAppSeniorModeStateObserverImpl() = default;

        virtual void OnSeniorModeStateChanged(const std::string& bundleName, int32_t appIndex, bool state) override
        {
            if (seniorModeClientDeleted_ == false) {
                config_.OnSeniorModeStateChanged(bundleName, appIndex, state);
            }
        }

        void OnclientDeleted()
        {
            seniorModeClientDeleted_ = true;
        }
    private:
        Impl &config_;
        std::atomic<bool> seniorModeClientDeleted_ = false;
    };

    bool ConnectToService();
    bool ConnectToServiceAsync();

    bool RegisterToService();
    bool InitAccessibilityServiceProxy();

    bool LoadAccessibilityService();
    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();
    sptr<Accessibility::IAccessibleAbilityManagerService> GetServiceProxy();

    void NotifyCaptionStateChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyCaptionChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const CaptionProperty &captionProperty);
    void NotifyScreenMagnificationChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyShortKeyChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyMouseKeyChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyShortkeyTargetChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const std::string &shortkey_target);
    void NotifyShortkeyMultiTargetChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const std::vector<std::string> &shortkeyMultiTarget);
    void NotifyMouseAutoClickChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const uint32_t mouseAutoClick);
    void NotifyAudioBalanceChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const float audioBalance);
    void NotifyBrightnessDiscountChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const float brightnessDiscount);
    void NotifyContentTimeoutChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const uint32_t contentTimeout);
    void NotifyDaltonizationColorFilterChanged(
        const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const uint32_t daltonizationColorFilter);
    void NotifyAudioMonoChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyAnimationOffChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyInvertColorChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyHighContrastTextChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyDaltonizationStateChanged(
        const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);
    void NotifyClickResponseTimeChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const uint32_t clickResponseTime);
    void NotifyIgnoreRepeatClickTimeChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const uint32_t time);
    void NotifyIgnoreRepeatClickStateChanged(const std::vector<std::shared_ptr<AccessibilityConfigObserver>> &observers,
        const bool state);

    void UpdateScreenMagnificationEnabled(const bool enabled);
    void UpdateShortKeyEnabled(const bool enabled);
    void UpdateMouseKeyEnabled(const bool enabled);
    void UpdateCaptionEnabled(const bool enabled);
    void UpdateAudioMonoEnabled(const bool enabled);
    void UpdateAnimationOffEnabled(const bool enabled);
    void UpdateInvertColorEnabled(const bool enabled);
    void UpdateHighContrastTextEnabled(const bool enabled);
    void UpdateDaltonizationStateEnabled(const bool enabled);
    void UpdateIgnoreRepeatClickStateEnabled(const bool enabled);
    void NotifyDefaultConfigs();
    void NotifyDefaultDaltonizationConfigs();
    void NotifyDefaultScreenTouchConfigs();
    void NotifyDefaultShortKeyConfigs();
    void NotifyDefaultShortKeyMultiConfigs();
    void NotifyImmediately(const CONFIG_ID id, const std::shared_ptr<AccessibilityConfigObserver> &observer);
    void InitConfigValues();
    uint32_t InvertDaltonizationColorInAtoHos(uint32_t filter);
    static void OnParameterChanged(const char *key, const char *value, void *context);
    void OnSeniorModeStateChanged(const std::string& bundleName, int32_t appIndex, bool state);

    void OnIgnoreRepeatClickStateChanged(const uint32_t stateType);
    bool CheckSaStatus();

    sptr<AccessibilityAppSeniorModeStateObserverImpl> seniorModeStateObserver_ = nullptr;
    sptr<Accessibility::IAccessibleAbilityManagerService> serviceProxy_ = nullptr;
    sptr<AccessibleAbilityManagerCaptionObserverImpl> captionObserver_ = nullptr;
    sptr<AccessibleAbilityManagerConfigObserverImpl> configObserver_ = nullptr;
    sptr<AccessibilityEnableAbilityListsObserverImpl> enableAbilityListsObserver_ = nullptr;
    sptr<AccessibilityEnableAbilityCallbackObserverImpl> enableAbilityCallbackObserver_ = nullptr;

    std::atomic<bool> isInitialized_ {false};
    bool shortkey_ = false;
    bool highContrastText_ = false;
    bool screenMagnifier_ = false;
    bool invertColor_ = false;
    bool captionState_ = false;
    bool animationOff_ = false;
    bool audioMono_ = false;
    bool mouseKey_ = false;
    bool daltonizationState_ = false;
    int32_t mouseAutoClick_ = 0;
    uint32_t contentTimeout_ = 0;
    uint32_t daltonizationColorFilter_ = 0;
    float audioBalance_ = 0.0;
    float brightnessDiscount_ = 0.0;
    std::string shortkeyTarget_ = "";
    uint32_t clickResponseTime_ = 0;
    uint32_t ignoreRepeatClickTime_ = 0;
    bool ignoreRepeatClickState_ = false;
    CaptionProperty captionProperty_ = {};

    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;

    std::vector<std::string> shortkeyMultiTarget_ {};
    std::vector<std::shared_ptr<AccessibilityEnableAbilityListsObserver>> enableAbilityListsObservers_;
    ffrt::mutex enableAbilityListsObserversMutex_;
    std::vector<std::shared_ptr<AccessibilityEnableAbilityCallbackObserver>> enableAbilityCallbackObservers_;
    ffrt::mutex enableAbilityCallbackObserversMutex_;
    std::map<CONFIG_ID, std::vector<std::shared_ptr<AccessibilityConfigObserver>>> configObservers_;
    ffrt::mutex configObserversMutex_;
    std::vector<std::shared_ptr<AccessibilityAppSeniorModeStateObserver>> seniorModeStateObservers_;
    ffrt::mutex seniorModeStateObserversMutex_;
    
    // use write-first-rwLock to protect serviceProxy_ to make sure when serviceProxy_ multi-thread used
    // and one thread which calls interface like SetAudioMonoState timeout,
    // InitializeContext will get mutex first, in case main-thread block.
    // InitializeContext always called in main thread
    ffrt::shared_mutex rwLock_;

    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // namespace AccessibilityConfig
} // namespace OHOS
#endif // ACCESSIBILITY_CONFIG_IMPL_H
