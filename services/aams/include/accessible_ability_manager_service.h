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

#ifndef ACCESSIBLE_ABILITY_MANAGER_SERVICE_H
#define ACCESSIBLE_ABILITY_MANAGER_SERVICE_H

#include <map>
#include <memory>
#include <string>

#include "accessibility_dumper.h"
#include "accessibility_def.h"
#include "accessible_ability_manager_service_stub.h"
#include "accessible_ability_manager_service_event_handler.h"
#include "accessibility_account_data.h"
#include "accessibility_common_event.h"
#include "accessibility_element_operator_callback_stub.h"
#include "accessibility_keyevent_filter.h"
#include "accessibility_settings.h"
#include "accessibility_touchEvent_injector.h"
#include "accessibility_window_info.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "input_manager.h"
#include "singleton.h"
#include "system_ability.h"
#include "window_manager.h"
#include "accessibility_short_key.h"
#include "accessibility_resource_bundle_manager.h"
#include "refbase.h"
#include "accessibility_security_component_manager.h"
#include "accessible_extend_manager_service_proxy.h"
#include "element_operator_callback_impl.h"

namespace OHOS {
namespace Accessibility {
class AccessibilityAccountData;
class AccessibilitySettings;
class AccessibilityBlinkController;
class AccessibilityResourceBundleManager;

enum CallBackID {
    STATE_CALLBACK,
    CAPTION_PROPERTY_CALLBACK,
    ENABLE_ABILITY_LISTS_CALLBACK,
    CONFIG_CALLBACK,
    SENIOR_MODE_STATE_CALLBACK
};

constexpr int REQUEST_ID_INIT = 65535;
constexpr uint32_t TIME_OUT_OPERATOR = 5000;
constexpr uint32_t TIME_OUT_1000MS = 1000;
constexpr int32_t WAIT_NOTIFY_DISCONNECT_TIMEOUT = 30 * 1000; // 30s

class AccessibleAbilityManagerService : public SystemAbility, public AccessibleAbilityManagerServiceStub {
    DECLARE_SINGLETON(AccessibleAbilityManagerService)
    DECLEAR_SYSTEM_ABILITY(AccessibleAbilityManagerService)
public:
    /* For system ability */
    void InitHandler();
    void InitActionHandler();
    void InitSendEventHandler();
    void InitChannelHandler();
    void InitHoverEnterHandler();
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    int Dump(int fd, const std::vector<std::u16string>& args) override;
    void PostDelayUnloadTask();

public:
    // for ext so
    void SendAccessibilityEventToAA(EventType eventType, GestureType gestureId, uint64_t displayId);

    /* For AccessibleAbilityManagerServiceStub */
    ErrCode SendEvent(const AccessibilityEventInfoParcel& eventInfoParcel, int32_t flag) override;
    ErrCode InnerSendEvent(const AccessibilityEventInfoParcel& eventInfoParcel, int32_t flag, int32_t userId);

    RetError VerifyingToKenId(const int32_t windowId, const int64_t elementId, int32_t userId, uint32_t tokenId = 0);

    ErrCode RegisterStateObserver(const sptr<IAccessibleAbilityManagerStateObserver> &callback,
        uint32_t &state) override;

    ErrCode RegisterCaptionObserver(const sptr<IAccessibleAbilityManagerCaptionObserver> &callback) override;

    ErrCode RegisterEnableAbilityListsObserver(
        const sptr<IAccessibilityEnableAbilityListsObserver> &observer) override;

    ErrCode RegisterEnableAbilityCallbackObserver(
        const sptr<IAccessibilityEnableAbilityCallbackObserver> &observer) override;

    ErrCode GetAbilityList(uint32_t abilityTypes, int32_t stateType,
        std::vector<AccessibilityAbilityInfoParcel>& infos) override;

    ErrCode RegisterElementOperatorByWindowId(const int32_t windowId,
        const sptr<IAccessibilityElementOperator> &elementOperator, uint64_t displayId) override;

    ErrCode RegisterElementOperatorByParameter(const RegistrationPara& parameter,
        const sptr<IAccessibilityElementOperator>& elementOperator) override;

    ErrCode RegisterSeniorModeStateObserver(
        const sptr<IAccessibilityAppSeniorModeStateObserver> &observer) override;

    ErrCode DeregisterElementOperatorByWindowId(const int32_t windowId, uint64_t displayId) override;

    ErrCode DeregisterElementOperatorByWindowIdAndTreeId(int32_t windowId, int32_t treeId, uint64_t displayId) override;

    ErrCode InnerDeregisterElementOperatorByWindowId(
        int32_t windowId, int32_t userId, uint64_t displayId, uint32_t tokenId, bool needCheckToken);

    ErrCode InnerDeregisterElementOperatorByWindowIdAndTreeId(
        int32_t windowId, int32_t treeId, int32_t userId, uint64_t displayId, uint32_t tokenId, bool needCheckToken);

    ErrCode DeRegisterCaptionObserver(const sptr<IRemoteObject>& obj) override;

    ErrCode DeRegisterConfigObserver(const sptr<IRemoteObject>& obj) override;

    ErrCode DeRegisterEnableAbilityListsObserver(const sptr<IRemoteObject>& obj) override;

    ErrCode DeRegisterEnableAbilityCallbackObserver(const sptr<IRemoteObject>& obj) override;

    ErrCode DeRegisterSeniorModeStateObserver(const sptr<IRemoteObject>& obj) override;

    ErrCode GetCaptionProperty(CaptionPropertyParcel &caption, bool isPermissionRequired) override;
    ErrCode SetCaptionProperty(const CaptionPropertyParcel &caption, bool isPermissionRequired) override;
    ErrCode SetCaptionState(const bool state, bool isPermissionRequired) override;

    ErrCode GetCaptionState(bool &state, bool isPermissionRequired) override;

    ErrCode EnableAbility(const std::string &name, const uint32_t capabilities,
        const bool connectCallBackFlag) override;
    ErrCode GetEnabledAbilities(std::vector<std::string> &enabledAbilities) override;
    RetError SetCurtainScreenUsingStatus(bool isEnable);
    ErrCode CheckExtensionAbilityPermission(std::string& processName) override;
    ErrCode DisableAbility(const std::string &name) override;
    ErrCode EnableUITestAbility(const sptr<IRemoteObject>& obj, int userId, int32_t& actualUserId) override;
    ErrCode DisableUITestAbility(int userId) override;
    ErrCode SetMagnificationState(const bool state) override;
    ErrCode GetActiveWindow(int32_t &windowId) override;
    ErrCode GetActiveWindow(int32_t &windowId, bool systemApi) override;
    ErrCode InnerGetActiveWindow(int32_t &windowId, int32_t userId);
    bool FindFocusedElement(AccessibilityElementInfo &elementInfo, uint32_t timeout, int32_t userId);
    bool ExecuteActionOnAccessibilityFocused(const ActionType &action, int32_t userId);
    ErrCode GetFocusedWindowId(int32_t &focusedWindowId) override;
    ErrCode InnerGetFocusedWindowId(int32_t &focusedWindowId, int32_t userId);
    ErrCode GetAccessibilityFocusedWindowId(int32_t &windowId, int32_t accountId);
    ErrCode GetRootParentId(int32_t windowId, int32_t treeId, int64_t &parentId) override;
    ErrCode GetRootParentId(int32_t windowId, int32_t treeId, int64_t &parentId, bool systemApi) override;
    bool GetElementOperator(const int32_t windowId,
        const int64_t elementId, sptr<IAccessibilityElementOperator> &elementOperator);
    ErrCode GetScreenReaderState(bool &state) override;
    ErrCode SearchNeedEvents(std::vector<uint32_t> &needEvents) override;
    ErrCode GetReadableRules(std::string &readableRules) override;
    ErrCode IsInnerWindowRootElement(int64_t elementId, bool &state) override;
    std::vector<AccessibilityWindowInfo> GetAccessibilityWindows(int32_t userId);
    bool InnerGetAccessibilityWindow(int32_t windowId, AccessibilityWindowInfo &window, int32_t userId);
private:
    std::atomic<int32_t> focusWindowId_ = -1;
    std::atomic<int64_t> focusElementId_ = -1;
public:
    /* For inner modules */
    bool EnableShortKeyTargetAbility(const std::string &name = "");
    bool EnableCaptionsAbility(sptr<AccessibilityAccountData> accountData);
    bool DisableShortKeyTargetAbility();
    void OnShortKeyProcess();
    void UpdateShortKeyRegister();

    /* For DisplayResize */
    void NotifyDisplayResizeStateChanged(int32_t displayId, Rect& rect, float scale, float centerX, float centerY);

    inline bool IsServiceReady()
    {
        return isReady_;
    }

    inline int32_t GetCurrentAccountId()
    {
        return currentAccountId_;
    }

    inline std::shared_ptr<AAMSEventHandler> &GetMainHandler()
    {
        return handler_;
    }

    inline std::shared_ptr<AppExecFwk::EventRunner> &GetMainRunner()
    {
        return runner_;
    }

    inline std::shared_ptr<AppExecFwk::EventRunner> &GetChannelRunner()
    {
        return channelRunner_;
    }

    sptr<AccessibilityAccountData> GetAccountData(int32_t accountId);
    sptr<AccessibilityAccountData> GetCurrentAccountData();
    std::vector<int32_t> GetAllAccountIds();

    /* For common event */
    void AddedUser(int32_t accountId);
    void RemovedUser(int32_t accountId);
    void SwitchedUser(int32_t accountId);
    void PackageChanged(const std::string &bundleName);
    void PackageRemoved(const std::string &bundleName);
    void PackageAdd(const std::string &bundleName);

    void UpdateAccessibilityManagerService();
    bool CheckWindowIdEventExist(int32_t windowId);

    // used for arkui windowId 1 map to WMS windowId
    ErrCode SetScreenMagnificationState(const bool state) override;
    ErrCode SetShortKeyState(const bool state) override;
    ErrCode SetMouseKeyState(const bool state) override;
    ErrCode SetMouseAutoClick(const int32_t time) override;
    ErrCode SetShortkeyTarget(const std::string &name) override;
    ErrCode SetShortkeyMultiTarget(const std::vector<std::string> &name) override;
    ErrCode SetHighContrastTextState(const bool state) override;
    ErrCode SetInvertColorState(const bool state) override;
    ErrCode SetAnimationOffState(const bool state) override;
    ErrCode SetAudioMonoState(const bool state) override;
    ErrCode SetDaltonizationState(const bool state) override;
    ErrCode SetDaltonizationColorFilter(const uint32_t filter) override;
    ErrCode SetContentTimeout(const uint32_t time) override;
    ErrCode SetBrightnessDiscount(const float discount) override;
    ErrCode SetAudioBalance(const float balance) override;
    ErrCode SetClickResponseTime(const uint32_t time) override;
    ErrCode SetIgnoreRepeatClickState(const bool state) override;
    ErrCode SetIgnoreRepeatClickTime(const uint32_t time) override;
    ErrCode SetSeniorModeStateForApp(const bool state) override; // asac
    ErrCode SetSeniorModeStateForApp(const std::vector<AccessibilityBundleSeniorModeInfoParcel> &infos) override; // ac

    ErrCode GetScreenMagnificationState(bool &state) override;
    ErrCode GetShortKeyState(bool &state) override;
    ErrCode GetMouseKeyState(bool &state) override;
    ErrCode GetMouseAutoClick(int32_t &time) override;
    ErrCode GetShortkeyTarget(std::string &name) override;
    ErrCode GetShortkeyMultiTarget(std::vector<std::string> &name) override;
    ErrCode GetHighContrastTextState(bool &state) override;
    ErrCode GetInvertColorState(bool &state) override;
    ErrCode GetAnimationOffState(bool &state) override;
    ErrCode GetAudioMonoState(bool &state) override;
    ErrCode GetDaltonizationState(bool &state) override;
    ErrCode GetDaltonizationColorFilter(uint32_t &type) override;
    ErrCode GetContentTimeout(uint32_t &timer) override;
    ErrCode GetBrightnessDiscount(float &brightness) override;
    ErrCode GetAudioBalance(float &balance) override;
    ErrCode GetClickResponseTime(uint32_t &time) override;
    ErrCode GetIgnoreRepeatClickState(bool &state) override;
    ErrCode GetIgnoreRepeatClickTime(uint32_t &time) override;
    ErrCode GetFlashReminderSwitch(bool &state) override;
    ErrCode ControlBlinking(bool start, int32_t mode, int32_t scenario, int32_t &result) override;
    ErrCode GetAllConfigs(AccessibilityConfigData& configData, CaptionPropertyParcel& caption) override;
    ErrCode GetSeniorModeState(bool &state) override;
    ErrCode GetSeniorModeStateForApp(bool &state) override; // asac
    ErrCode GetSeniorModeStateForApp(const std::string &bundleName, int32_t appIndex, bool &state) override; // ac

    ErrCode RegisterConfigObserver(const sptr<IAccessibleAbilityManagerConfigObserver> &callback) override;
    void UpdateConfigState();
    void UpdateAudioBalance();
    void UpdateBrightnessDiscount();
    void UpdateContentTimeout();
    void UpdateDaltonizationColorFilter();
    void UpdateMouseAutoClick();
    void UpdateShortkeyTarget();
    void UpdateShortkeyMultiTarget();
    void UpdateClickResponseTime();
    void UpdateIgnoreRepeatClickTime();

    void UpdateInputFilter();
    void UpdateAccessibilityState();
    ErrCode RemoveRequestId(int32_t requestId) override;
    void OnDataClone();
    bool CheckPermission(const std::string &permission) const;
    int32_t SetEnhanceConfig(const AccessibilitySecCompRawdata& rawData) override;
    void RegisterPcModeSwitch();

    // for magnification
    bool GetMagnificationState();
    uint32_t GetMagnificationType();
    uint32_t GetMagnificationMode();
    int32_t GetMagnificationTriggerMethod();
    void SetMagnificationMode(int32_t mode);
    float GetMagnificationScale();
    void SetMagnificationScale(float scale);
    ErrCode AnnouncedForAccessibility(const std::string &announcedText);
    void InitResource(bool needReInit);
    std::string &GetResource(const std::string &resourceName);
    void AnnouncedForMagnification(AnnounceType announceType);
private:
    int32_t InnerGetCallingUid();
    bool IsApp() const;
    bool IsSystemApp() const;
    bool IsBroker() const;
    ErrCode CheckDeregisterTokenId(
        int32_t windowId, int32_t treeId, uint32_t tokenId, sptr<AccessibilityAccountData> &accountData);
    bool SetTargetAbility(const int32_t targetAbilityValue);
    bool SetHighContrastTextAbility(bool state);
    void PublishAccessibilityCommonEvent(const std::string &event);
    int32_t GetUserIdByDisplayId(uint64_t displayId);
    class StateCallbackDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        StateCallbackDeathRecipient() = default;
        ~StateCallbackDeathRecipient() final = default;
        DISALLOW_COPY_AND_MOVE(StateCallbackDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;
    };

    class CaptionPropertyCallbackDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        CaptionPropertyCallbackDeathRecipient() = default;
        ~CaptionPropertyCallbackDeathRecipient() final = default;
        DISALLOW_COPY_AND_MOVE(CaptionPropertyCallbackDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;
    };

    class EnableAbilityListsObserverDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        EnableAbilityListsObserverDeathRecipient() = default;
        ~EnableAbilityListsObserverDeathRecipient() final = default;
        DISALLOW_COPY_AND_MOVE(EnableAbilityListsObserverDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;
    };

    bool Init();
    void InitInnerResource();

    class ConfigCallbackDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        ConfigCallbackDeathRecipient() = default;
        ~ConfigCallbackDeathRecipient() final = default;
        DISALLOW_COPY_AND_MOVE(ConfigCallbackDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;
    };

    class BundleManagerDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        BundleManagerDeathRecipient() = default;
        ~BundleManagerDeathRecipient() final = default;
        DISALLOW_COPY_AND_MOVE(BundleManagerDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;
    };

    class SeniorModeStateObserverDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        SeniorModeStateObserverDeathRecipient() = default;
        ~SeniorModeStateObserverDeathRecipient() final = default;
        DISALLOW_COPY_AND_MOVE(SeniorModeStateObserverDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;
    };

    RetError InnerEnableAbility(const std::string &name, const uint32_t capabilities,
        const std::string callerBundleName = "");
    RetError InnerDisableAbility(const std::string &name);

    void ClearFocus(int32_t windowId);
    void OutsideTouch(int32_t windowId);

    void UpdateCaptionProperty();
    void UpdateSettingsInAtoHosTask();
    void UpdateSettingsInAtoHos();
    void UpdateAutoStartAbilities();
    void UpdateAllSetting();
    void UpdateCriticalState();

    void RemoveCallback(CallBackID callback, const sptr<DeathRecipient> &recipient, const wptr<IRemoteObject> &remote);
    void RemoveSavedConfigCallback(const wptr<IRemoteObject>& callback);
    
    void OnDeviceProvisioned();
    void InitializeShortKeyState();
    void RegisterProvisionCallback();
    void RegisterShortKeyEvent();
    bool IsNeedUnload();
    void OnScreenMagnificationStateChanged();
    void RegisterScreenMagnificationState();
    void OnScreenMagnificationTypeChanged();
    void SetConfigScreenMagnificationScale(float scale);
    void OnScreenMagnificationTriggerMethodChanged();
    void SetConfigScreenMagnificationTriggerMethod(int32_t triggerMethod);
    void OnScreenMagnificationScaleChanged();
    void RegisterScreenMagnificationType();
    void OnFlashReminderSwitchChanged();
    void RegisterFlashReminderSwitch();
    void OnSeniorModeStateChanged();
    void RegisterSeniorModeState();

    void OnVoiceRecognitionChanged();
    void RegisterVoiceRecognitionState();
    void UpdateVoiceRecognitionState();
    void SubscribeOsAccount();
    void UnsubscribeOsAccount();
    void RegisterNotificationState();
    void OnSeniorModeStateForAppChanged();
    void RegisterSeniorModeStateForAppObserver();

    void RecycleEventHandler();
    std::shared_ptr<AccessibilityDatashareHelper> GetCurrentAcountDatashareHelper();

    bool isReady_ = false;
    bool isPublished_ = false;
    std::map<int32_t, bool> dependentServicesStatus_;
    int32_t currentAccountId_ = -1;
    uint32_t currentAccessibilityState_ = 0;
    AccessibilityAccountDataMap  a11yAccountsData_;

    sptr<AccessibilityDumper> accessibilityDumper_ = nullptr;

    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<AAMSEventHandler> handler_;

    std::shared_ptr<AppExecFwk::EventRunner> actionRunner_;
    std::shared_ptr<AAMSEventHandler> actionHandler_;

    std::shared_ptr<AppExecFwk::EventRunner> sendEventRunner_;
    std::shared_ptr<AAMSEventHandler> sendEventHandler_;

    std::shared_ptr<AppExecFwk::EventRunner> channelRunner_;
    std::shared_ptr<AAMSEventHandler> channelHandler_;

    std::shared_ptr<AppExecFwk::EventRunner> hoverEnterRunner_;
    std::shared_ptr<AAMSEventHandler> hoverEnterHandler_;

    int64_t ipcTimeoutNum_ = 0; // count ipc timeout number

    sptr<IRemoteObject::DeathRecipient> stateObserversDeathRecipient_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> captionPropertyCallbackDeathRecipient_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> enableAbilityListsObserverDeathRecipient_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> enableAbilityCallbackObserverDeathRecipient_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> seniorModeStateObserverDeathRecipient_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> configCallbackDeathRecipient_ = nullptr;
    ffrt::mutex mutex_; // current used for register state observer
    std::vector<sptr<IAccessibleAbilityManagerConfigObserver>> defaultConfigCallbacks_;
    std::shared_ptr<AccessibilitySettings> accessibilitySettings_ = nullptr;
    ffrt::mutex blinkMutex_;
    bool blinkStopping_ = false;
    std::shared_ptr<AccessibilityBlinkController> blinkController_;
    std::shared_ptr<AccessibilityShortKey> accessibilityShortKey_ = nullptr;
    std::vector<std::string> removedAutoStartAbilities_ {};

    std::map<int32_t, std::map<int32_t, std::set<int32_t>>> windowRequestIdMap_ {}; // windowId->treeId->requestId
    std::map<int32_t, sptr<IAccessibilityElementOperatorCallback>> requestIdMap_ {}; // requestId->callback

    bool isSubscribeMSDPCallback_ = false;
    ffrt::mutex subscribeMSDPMutex_;
    bool isResourceInit_ = false;
    std::shared_ptr<AccountSubscriber> accountSubscriber_ = nullptr;
    ffrt::mutex resourceMapMutex_;
};
} // namespace Accessibility
} // namespace OHOS
#endif // ACCESSIBLE_ABILITY_MANAGER_SERVICE_H
