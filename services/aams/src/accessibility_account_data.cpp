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

#include "accessibility_account_data.h"
#include "accessibility_element_operator_manager.h"
#include "accessible_ability_manager.h"

#include <any>
#include <dlfcn.h>
#include <ipc_skeleton.h>
#ifdef OHOS_BUILD_ENABLE_HITRACE
#include <hitrace_meter.h>
#endif // OHOS_BUILD_ENABLE_HITRACE
#ifdef OHOS_BUILD_ENABLE_POWER_MANAGER
#include "accessibility_power_manager.h"
#endif
#include "accessible_ability_manager_service.h"
#include "extension_ability_info.h"
#include "hilog_wrapper.h"
#include "utils.h"
#include "system_ability_definition.h"
#include "os_account_manager.h"
#include "accessibility_resource_bundle_manager.h"
#include "accessibility_notification_helper.h"
#include "accessibility_short_key_dialog.h"
#include "nlohmann/json.hpp"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace Accessibility {
namespace {
    constexpr int32_t AUTOCLICK_DELAY_TIME_MIN = 1000; // ms
    constexpr int32_t AUTOCLICK_DELAY_TIME_MAX = 5000; // ms
    constexpr int32_t INIT_DATASHARE_HELPER_SLEEP_TIME = 500;
    constexpr int DEFAULT_ACCOUNT_ID = 100;
    constexpr int SHORT_KEY_TIMEOUT_BEFORE_USE = 3000; // ms
    const std::string HIGH_TEXT_CONTRAST_ENABLED = "high_text_contrast_enabled";
    const std::string ACCESSIBILITY_DISPLAY_INVERSION_ENABLED = "accessibility_display_inversion_enabled";
    const std::string ACCESSIBILITY_DISPLAY_DALTONIZER_ENABLED = "accessibility_display_daltonizer_enabled";
    const std::string MASTER_MONO = "master_mono";
    const std::string ACCESSIBILITY_SCREENREADER_ENABLED = "accessibility_screenreader_enabled";
    const std::string MASTER_BALENCE = "master_balance";
    const std::string CLICK_RESPONSE_TIME = "click_response_time";
    const std::string IGNORE_REPEAT_CLICK_SWITCH = "ignore_repeat_click_switch";
    const std::string IGNORE_REPEAT_CLICK_TIME = "ignore_repeat_click_time";
    const std::string ACCESSIBILITY_DISPLAY_DALTONIZER = "accessibility_display_daltonizer";
    const std::string SCREEN_READER_BUNDLE_ABILITY_NAME = "com.ohos.screenreader/AccessibilityExtAbility";
    const std::string DEVICE_PROVISIONED = "device_provisioned";
    const std::string ENABLED_ACCESSIBILITY_SERVICES = "enabled_accessibility_services";
    const std::string ACCESSIBILITY_SHORTCUT_ENABLED = "accessibility_shortcut_enabled";
    const std::string ACCESSIBILITY_SHORTCUT_ENABLED_ON_LOCK_SCREEN = "accessibility_shortcut_enabled_on_lock_screen";
    const std::string ACCESSIBILITY_SHORTCUT_TIMEOUT = "accessibility_shortcut_timeout";
    const std::string SCREEN_MAGNIFICATION_KEY = "accessibility_display_magnification_enabled";
    const std::string ACCESSIBILITY_CLONE_FLAG = "accessibility_config_clone";
    const std::string ACCESSIBILITY_TOUCH_GUIDE_ENABLED = "enableTouchGuideMode";
    const std::string SCREEN_READER_SINGLE_CLICK_MODE = "screen_reader_single_click_mode";
    const std::string ACCESSIBILITY_PRIVACY_CLONE_OR_UPGRADE = "accessibility_privacy_clone_or_upgrade";
    const std::string SCREEN_READER_BUNDLE_NAME = "com.ohos.screenreader";
    const std::string THP_PATH = "/system/lib64/libthp_extra_innerapi.z.so";
    const std::string IGNORE_REPEATED_CLICK_EXCLUDE_FLAG = "accessibility_ignore_repeat_click_exclude_flag";

    // Feature flag for full screen magnification.
    static constexpr uint32_t FEATURE_SCREEN_MAGNIFICATION = 0x00000001;
 
    // Feature flag for touch exploration.
    static constexpr uint32_t FEATURE_TOUCH_EXPLORATION = 0x00000002;
 
    // Feature flag for filtering key events.
    static constexpr uint32_t FEATURE_FILTER_KEY_EVENTS = 0x00000004;
 
    // Feature flag for inject touch events.
    static constexpr uint32_t FEATURE_INJECT_TOUCH_EVENTS = 0x00000008;
 
    // Feature flag for mouse autoclick.
    static constexpr uint32_t FEATURE_MOUSE_AUTOCLICK = 0x00000010;
 
    // Feature flag for mouse key.
    static constexpr uint32_t FEATURE_MOUSE_KEY = 0x00000040;
 
    // Feature flag for screen touch.
    static constexpr uint32_t FEATURE_SCREEN_TOUCH = 0x00000080;
} // namespace

AccessibilityAccountData::AccessibilityAccountData(int32_t accountId)
{
    id_ = accountId;
    elementOperatorManager_.SetAccountData(accountId, this);
    windowManager_.SetAccountData(accountId, this);
}

AccessibilityAccountData::~AccessibilityAccountData()
{}

int32_t AccessibilityAccountData::GetAccountId()
{
    HILOG_INFO();
    return id_;
}

ElementOperatorManager& AccessibilityAccountData::GetElementOperatorManager()
{
    return elementOperatorManager_;
}
AccessibleAbilityManager& AccessibilityAccountData::GetAccessibleAbilityManager()
{
    return accessibleAbilityManager_;
}
 
AccessibilityWindowManager& AccessibilityAccountData::GetWindowManager()
{
    return windowManager_;
}

uint32_t AccessibilityAccountData::GetAccessibilityState()
{
    uint32_t state = 0;
    if (accessibleAbilityManager_.GetConnectedAbilitiesSize() != 0 ||
        accessibleAbilityManager_.GetConnectingAbilitiesSize() != 0) {
        HILOG_DEBUG("connectingA11yAbilities %{public}zu connectedA11yAbilities %{public}zu",
            accessibleAbilityManager_.GetConnectingAbilitiesSize(),
            accessibleAbilityManager_.GetConnectedAbilitiesSize());
        state |= STATE_ACCESSIBILITY_ENABLED;
        if (!config_->GetEnabledState()) {
            config_->SetEnabled(true);
        }
    } else {
        if (config_->GetEnabledState()) {
            config_->SetEnabled(false);
        }
    }

    if (config_->GetTouchGuideState()) {
        state |= STATE_EXPLORATION_ENABLED;
    }
    
    if (config_->GetKeyEventObserverState()) {
        state |= STATE_KEYEVENT_ENABLED;
    }

    if (config_->GetGestureState()) {
        state |= STATE_GESTURE_ENABLED;
    }

    if (screenReaderState_) {
        state |= STATE_SCREENREADER_ENABLED;
    }

    if (isSingleClickMode_) {
        state |= STATE_SINGLE_CLICK_MODE_ENABLED;
    }
    uint32_t inputFlag = GetInputFilterFlag();
    if (inputFlag > 0) {
        state |= STATE_INPUT_INTERCEPTOR_ENABLED;
    }
    if (config_->GetAnimationOffState()) {
        state |= STATE_ANIMATIONOFF_ENABLED;
    }
    if (config_->GetAudioMonoState()) {
        state |= STATE_AUDIOMONO_ENABLED;
    }
    if (config_->GetFlashReminderSwitch()) {
        state |= STATE_FLASH_REMINDER_ENABLED;
    }
    if (config_->GetSeniorModeState()) {
        state |= STATE_ELDER_CARE_ENABLED;
    }
    HILOG_INFO("GetAccessibilityState [%{public}u]", state);
    return state;
}

void AccessibilityAccountData::OnAccountSwitched()
{
    HILOG_INFO();
    accessibleAbilityManager_.ClearConnectedAbilities();
    std::vector<sptr<AccessibleAbilityConnection>> connectionList;
    accessibleAbilityManager_.GetConnectedAbilities(connectionList);
    for (auto& connection : connectionList) {
        if (connection) {
            connection->Disconnect();
        }
    }

    accessibleAbilityManager_.Clear();
    Singleton<AccessibilityPowerManager>::GetInstance().UnholdRunningLock();
    elementOperatorManager_.Clear();
    if (!config_) {
        return;
    }
    if (config_->GetIgnoreRepeatClickState()) {
        IgnoreRepeatClickNotification::CancelNotification();
    }
    if (config_->GetAnimationOffState()) {
        TransitionAnimationsNotification::CancelNotification();
    }
    if (config_->GetDbHandle()) {
        config_->GetDbHandle()->ClearObservers();
    }
    if (config_->GetSystemDbHandle()) {
        config_->GetSystemDbHandle()->ClearObservers();
    }
}

void AccessibilityAccountData::AddConnectedAbility(sptr<AccessibleAbilityConnection>& connection)
{
    if (!connection) {
        HILOG_ERROR("connection is nullptr");
        return;
    }
    std::string uri = Utils::GetUri(connection->GetElementName());
    std::string bundleName = "";
    size_t pos = uri.find('/');
    if (pos != std::string::npos) {
        bundleName = uri.substr(0, pos);
    }
    accessibleAbilityManager_.AddConnectedAbility(connection);
    
    std::vector<uint32_t> events {};
    UpdateAbilityNeedEvent(bundleName, events);
}

void AccessibilityAccountData::RemoveConnectedAbility(const AppExecFwk::ElementName &element)
{
    accessibleAbilityManager_.RemoveConnectedAbility(element);
}

void AccessibilityAccountData::AddCaptionPropertyCallback(
    const sptr<IAccessibleAbilityManagerCaptionObserver>& callback)
{
    HILOG_DEBUG();
    std::lock_guard<ffrt::mutex> lock(captionPropertyCallbacksMutex_);
    captionPropertyCallbacks_.push_back(callback);
}

void AccessibilityAccountData::RemoveCaptionPropertyCallback(const wptr<IRemoteObject>& callback)
{
    HILOG_DEBUG();
    std::lock_guard<ffrt::mutex> lock(captionPropertyCallbacksMutex_);
    for (auto itr = captionPropertyCallbacks_.begin(); itr != captionPropertyCallbacks_.end(); itr++) {
        if ((*itr)->AsObject() == callback) {
            captionPropertyCallbacks_.erase(itr);
            break;
        }
    }
}

void AccessibilityAccountData::AddEnableAbilityListsObserver(
    const sptr<IAccessibilityEnableAbilityListsObserver>& observer)
{
    accessibleAbilityManager_.AddEnableAbilityListsObserver(observer);
}

void AccessibilityAccountData::RemoveEnableAbilityListsObserver(const wptr<IRemoteObject>& observer)
{
    accessibleAbilityManager_.RemoveEnableAbilityListsObserver(observer);
}

void AccessibilityAccountData::AddEnableAbilityCallbackObserver(
    const sptr<IAccessibilityEnableAbilityCallbackObserver>& observer)
{
    accessibleAbilityManager_.AddEnableAbilityCallbackObserver(observer);
}

void AccessibilityAccountData::RemoveEnableAbilityCallbackObserver(const wptr<IRemoteObject>& observer)
{
    accessibleAbilityManager_.RemoveEnableAbilityCallbackObserver(observer);
}

void AccessibilityAccountData::UpdateEnableAbilityListsState()
{
    accessibleAbilityManager_.UpdateEnableAbilityListsState();
}

void AccessibilityAccountData::AddAccessibilityWindowConnection(
    const int32_t windowId, const sptr<AccessibilityWindowConnection>& interactionConnection)
{
    elementOperatorManager_.AddAccessibilityWindowConnection(windowId, interactionConnection);
}

void AccessibilityAccountData::RemoveAccessibilityWindowConnection(const int32_t windowId)
{
    elementOperatorManager_.RemoveAccessibilityWindowConnection(windowId);
}

void AccessibilityAccountData::AddConnectingA11yAbility(const std::string &uri,
    const sptr<AccessibleAbilityConnection> &connection)
{
    accessibleAbilityManager_.AddConnectingAbility(uri, connection);
}

void AccessibilityAccountData::RemoveConnectingA11yAbility(const std::string &uri)
{
    accessibleAbilityManager_.RemoveConnectingAbility(uri);
}

void AccessibilityAccountData::AddEnabledAbility(const std::string &name)
{
    accessibleAbilityManager_.AddEnabledAbility(name);
}

RetError AccessibilityAccountData::RemoveEnabledAbility(const std::string &name)
{
    RetError ret = accessibleAbilityManager_.RemoveEnabledAbility(name);
    if (ret == RET_OK) {
        RemoveNeedEvent(name);
    }
    return ret;
}

void AccessibilityAccountData::AddInstalledAbility(AccessibilityAbilityInfo& abilityInfo)
{
    accessibleAbilityManager_.AddInstalledAbility(abilityInfo);
}

void AccessibilityAccountData::RemoveInstalledAbility(const std::string &bundleName)
{
    accessibleAbilityManager_.RemoveInstalledAbility(bundleName);
}

void AccessibilityAccountData::ClearInstalledAbility()
{
    accessibleAbilityManager_.ClearInstalledAbility();
}

const sptr<AccessibleAbilityConnection> AccessibilityAccountData::GetAccessibleAbilityConnection(
    const std::string &elementName)
{
    return accessibleAbilityManager_.GetConnectedAbilityByName(elementName);
}

const sptr<AccessibilityWindowConnection> AccessibilityAccountData::GetAccessibilityWindowConnection(
    const int32_t windowId)
{
    return elementOperatorManager_.GetAccessibilityWindowConnection(windowId);
}

const std::map<std::string, sptr<AccessibleAbilityConnection>> AccessibilityAccountData::GetConnectedA11yAbilities()
{
    HILOG_DEBUG("GetConnectedA11yAbilities start.");
    std::map<std::string, sptr<AccessibleAbilityConnection>> connectionMap;
    accessibleAbilityManager_.GetConnectedAbilitiesMap(connectionMap);
    return connectionMap;
}

const sptr<AccessibleAbilityConnection> AccessibilityAccountData::GetWaitDisConnectAbility(
    const std::string &elementName)
{
    return accessibleAbilityManager_.GetWaitDisConnectAbility(elementName);
}

void AccessibilityAccountData::AddWaitDisconnectAbility(sptr<AccessibleAbilityConnection>& connection)
{
    accessibleAbilityManager_.AddWaitDisconnectAbility(connection);
}

void AccessibilityAccountData::RemoveWaitDisconnectAbility(const std::string &uri)
{
    HILOG_INFO();
    accessibleAbilityManager_.RemoveWaitDisconnectAbility(uri);
}

const std::map<int32_t, sptr<AccessibilityWindowConnection>> AccessibilityAccountData::GetAsacConnections()
{
    return elementOperatorManager_.GetAsacConnections();
}

const CaptionPropertyCallbacks AccessibilityAccountData::GetCaptionPropertyCallbacks()
{
    HILOG_DEBUG("GetCaptionPropertyCallbacks start.");
    std::lock_guard<ffrt::mutex> lock(captionPropertyCallbacksMutex_);
    CaptionPropertyCallbacks rtnVec = captionPropertyCallbacks_;
    return rtnVec;
}

sptr<AccessibleAbilityConnection> AccessibilityAccountData::GetConnectingA11yAbility(const std::string &uri)
{
    return accessibleAbilityManager_.GetConnectingAbility(uri);
}

const std::vector<std::string> &AccessibilityAccountData::GetEnabledAbilities()
{
    return accessibleAbilityManager_.GetEnabledAbilities();
}

const std::vector<AccessibilityAbilityInfo> &AccessibilityAccountData::GetInstalledAbilities() const
{
    HILOG_DEBUG("GetInstalledAbilities start.");
    return accessibleAbilityManager_.GetInstalledAbilities();
}

void AccessibilityAccountData::GetAbilitiesByState(AbilityStateType state,
    std::vector<AccessibilityAbilityInfo> &abilities)
{
    HILOG_DEBUG("get abilities by state %{public}d", state);
    if (state == ABILITY_STATE_ENABLE) {
        accessibleAbilityManager_.GetConnectedAbilitiesInfo(abilities);
    } else if (state == ABILITY_STATE_DISABLE) {
        GetDisableAbilities(abilities);
        HILOG_DEBUG("the size of disable abilities is %{public}zu", abilities.size());
    } else {
        abilities = accessibleAbilityManager_.GetInstalledAbilities();
    }
}

void AccessibilityAccountData::GetDisableAbilities(std::vector<AccessibilityAbilityInfo> &disabledAbilities)
{
    HILOG_DEBUG("get disable abilities");
    disabledAbilities = accessibleAbilityManager_.GetInstalledAbilities();
    accessibleAbilityManager_.GetDisableAbilities(disabledAbilities);
}

void AccessibilityAccountData::UpdateAccountCapabilities()
{
    HILOG_DEBUG("start.");
    UpdateFilteringKeyEventsCapability();
    UpdateEventTouchGuideCapability();
    UpdateGesturesSimulationCapability();
    UpdateMagnificationCapability();
    config_->SetTouchGuideState(isEventTouchGuideState_);
    config_->SetGestureState(isGesturesSimulation_);
    config_->SetKeyEventObserverState(isFilteringKeyEvents_);
}

void AccessibilityAccountData::UpdateEventTouchGuideCapability()
{
    auto touchGuideState = true;
    if (config_->GetDbHandle() == nullptr) {
        HILOG_WARN("helper is null!");
    } else {
        touchGuideState = config_->GetDbHandle()->GetBoolValue(ACCESSIBILITY_TOUCH_GUIDE_ENABLED, true);
    }

    if (accessibleAbilityManager_.IsExistCapability(Capability::CAPABILITY_TOUCH_GUIDE)) {
        isEventTouchGuideState_ = true;
        return;
    }

    isEventTouchGuideState_ = false;
}

void AccessibilityAccountData::UpdateGesturesSimulationCapability()
{
    if (accessibleAbilityManager_.IsExistCapability(Capability::CAPABILITY_GESTURE)) {
        isGesturesSimulation_ = true;
        return;
    }

    isGesturesSimulation_ = false;
}

void AccessibilityAccountData::UpdateFilteringKeyEventsCapability()
{
    if (accessibleAbilityManager_.IsExistCapability(Capability::CAPABILITY_KEY_EVENT_OBSERVER)) {
        isFilteringKeyEvents_ = true;
        return;
    }

    isFilteringKeyEvents_ = false;
}

void AccessibilityAccountData::UpdateMagnificationCapability()
{
    if (accessibleAbilityManager_.IsExistCapability(Capability::CAPABILITY_ZOOM)) {
        isScreenMagnification_ = true;
        return;
    }

    isScreenMagnification_ = false;
}

void AccessibilityAccountData::SetScreenReaderExtInAllAccounts(const bool state)
{
    RetError rtn = RET_OK;
    std::vector<int32_t> accountIds = Singleton<AccessibleAbilityManagerService>::GetInstance().GetAllAccountIds();
    for (auto accountId : accountIds) {
        auto accountData = Singleton<AccessibleAbilityManagerService>::GetInstance().GetAccountData(accountId);
        if (accountData == nullptr) {
            HILOG_WARN("accountData is nullptr, accountId = %{public}d", accountId);
            continue;
        }
        std::shared_ptr<AccessibilitySettingsConfig> config = accountData->GetConfig();
        if (config == nullptr) {
            HILOG_WARN("config is nullptr, accountId = %{public}d", accountId);
            continue;
        }
        if (state) {
            rtn = config->AddEnabledAccessibilityService(SCREEN_READER_BUNDLE_ABILITY_NAME);
        } else {
            rtn = config->RemoveEnabledAccessibilityService(SCREEN_READER_BUNDLE_ABILITY_NAME);
        }
        HILOG_INFO("set screenReader state = %{public}d, rtn = %{public}d, accountId = %{public}d", state, rtn,
            accountId);
    }
}

void AccessibilityAccountData::SetAbilityAutoStartState(const std::string &name, const bool state)
{
    RetError rtn = RET_OK;
    if (name == SCREEN_READER_BUNDLE_ABILITY_NAME) {
        SetScreenReaderState(screenReaderKey_, state ? "1" : "0");
        SetScreenReaderExtInAllAccounts(state);
        return;
    }
    if (!config_) {
        HILOG_WARN("conig_ is nullptr.");
        return;
    }
    if (state) {
        rtn = config_->AddEnabledAccessibilityService(name);
    } else {
        rtn = config_->RemoveEnabledAccessibilityService(name);
    }
    if (rtn != RET_OK) {
        HILOG_WARN("handle service %{public}s state = %{public}d failed, rtn = %{public}d", name.c_str(), state, rtn);
    }
}

void AccessibilityAccountData::SetAccessibilityStateToTP(bool state)
{
    HILOG_INFO("set accessibility state to TP, state = %{public}d", state);

    void *handle = nullptr;
    handle = dlopen(THP_PATH.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        HILOG_ERROR("thp handle is null!");
        return;
    }

    typedef const char* (*ThpExtraRunCommandFunc)(const char* command, const char* parameters);
    const char* (*ThpExtraRunCommand)(const char* command, const char* parameters) {};
    ThpExtraRunCommand = reinterpret_cast<ThpExtraRunCommandFunc>(dlsym(handle, "ThpExtraRunCommand"));
    if (ThpExtraRunCommand == nullptr) {
        HILOG_ERROR("ThpExtraRunCommand is null");
        dlclose(handle);
        return;
    }

    std::string stateValue = state ? "1" : "0";
    const std::string param = std::string("THP#").append(stateValue);
    ThpExtraRunCommand("THP_SetAccessibilityMode", param.c_str());
    dlclose(handle);
}

void AccessibilityAccountData::SetScreenReaderState(const std::string &name, const std::string &state)
{
    HILOG_DEBUG("set screen reader key [%{public}s], state = [%{public}s].", name.c_str(), state.c_str());
    std::shared_ptr<AccessibilitySettingProvider> service =
        AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }
    ErrCode ret = service->PutStringValue(name, state, true);
    if (ret != ERR_OK) {
        HILOG_ERROR("set failed, ret=%{public}d", ret);
    } else {
        screenReaderState_ = (state == "1");
        SetAccessibilityStateToTP(state == "1");
    }
    if (screenReaderState_) {
        Singleton<AccessibleAbilityManagerService>::GetInstance().InitResource(false);
    }
}

bool AccessibilityAccountData::GetScreenReaderState()
{
    HILOG_DEBUG("screen reader sstate is %{public}d", screenReaderState_);
    return screenReaderState_;
}

bool AccessibilityAccountData::GetDefaultUserScreenReaderState()
{
    HILOG_DEBUG();
    std::vector<std::string> services = config_->GetEnabledAccessibilityServices();
    auto iter = std::find(services.begin(), services.end(), SCREEN_READER_BUNDLE_ABILITY_NAME);
    return iter != services.end();
}

void AccessibilityAccountData::DelAutoStartPrefKeyInRemovePkg(const std::string &bundleName)
{
    HILOG_ERROR("start and bundleName[%{public}s].", bundleName.c_str());
    const std::vector<AccessibilityAbilityInfo>& installedAbilities = accessibleAbilityManager_.GetInstalledAbilities();
    if (installedAbilities.empty()) {
        HILOG_DEBUG("There is no installed abilities.");
        return;
    }
    for (auto &installAbility : installedAbilities) {
        if (installAbility.GetPackageName() == bundleName) {
            std::string abilityName = installAbility.GetName();
            std::string uri = Utils::GetUri(bundleName, abilityName);
            SetAbilityAutoStartState(uri, false);
            break;
        }
    }
}

bool AccessibilityAccountData::GetAbilityAutoStartState(const std::string &name)
{
    if (name == SCREEN_READER_BUNDLE_ABILITY_NAME && GetAccountType() == AccountSA::OsAccountType::PRIVATE) {
        return GetDefaultUserScreenReaderState();
    }
    if (!config_) {
        HILOG_WARN("config_ is nullptr.");
        return false;
    }
    std::vector<std::string> serviceList = config_->GetEnabledAccessibilityServices();
    auto iter = std::find(serviceList.begin(), serviceList.end(), name);
    if (iter != serviceList.end()) {
        return true;
    } else {
        return false;
    }
}

void AccessibilityAccountData::GetConfigValueAtoHos(ConfigValueAtoHosUpdate &value)
{
    HILOG_DEBUG();
    if (config_ == nullptr) {
        HILOG_ERROR("config_ is nullptr");
        return;
    }

    if (config_->GetDbHandle() == nullptr) {
        HILOG_INFO("helper is null, retry.");
        std::this_thread::sleep_for(std::chrono::milliseconds(INIT_DATASHARE_HELPER_SLEEP_TIME));
        if (config_->GetDbHandle() == nullptr) {
            HILOG_ERROR("helper is null");
            return;
        }
    }

    value.highContrastText = config_->GetDbHandle()->GetBoolValue(HIGH_TEXT_CONTRAST_ENABLED, false);
    value.invertColor = config_->GetDbHandle()->GetBoolValue(ACCESSIBILITY_DISPLAY_INVERSION_ENABLED, false);
    value.daltonizationState = config_->GetDbHandle()->GetBoolValue(ACCESSIBILITY_DISPLAY_DALTONIZER_ENABLED, false);
    value.displayDaltonizer = config_->GetDbHandle()->GetIntValue(ACCESSIBILITY_DISPLAY_DALTONIZER, 0);
    value.shortcutEnabled = config_->GetDbHandle()->GetBoolValue(ACCESSIBILITY_SHORTCUT_ENABLED, true);
    value.shortcutEnabledOnLockScreen = config_->GetDbHandle()->GetBoolValue(
        ACCESSIBILITY_SHORTCUT_ENABLED_ON_LOCK_SCREEN, true);
    value.shortcutTimeout = config_->GetDbHandle()->GetIntValue(ACCESSIBILITY_SHORTCUT_TIMEOUT,
        SHORT_KEY_TIMEOUT_BEFORE_USE);
    value.clickResponseTime = config_->GetDbHandle()->GetIntValue(CLICK_RESPONSE_TIME, 0);
    value.ignoreRepeatClickState = config_->GetDbHandle()->GetBoolValue(IGNORE_REPEAT_CLICK_SWITCH, false);
    value.ignoreRepeatClickTime = config_->GetDbHandle()->GetIntValue(IGNORE_REPEAT_CLICK_TIME, 0);
    value.screenMagnificationState = config_->GetDbHandle()->GetBoolValue(SCREEN_MAGNIFICATION_KEY, false);

    // In Aos, the audio configuration is stored in SYSTEM and it should be copied to SECURE
    config_->CloneAudioState();
    value.audioMono = config_->GetDbHandle()->GetBoolValue(MASTER_MONO, false);
    value.audioBalance = config_->GetDbHandle()->GetFloatValue(MASTER_BALENCE, 0);

    std::shared_ptr<AccessibilitySettingProvider> service =
        AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }
    service->PutBoolValue(ACCESSIBILITY_PRIVACY_CLONE_OR_UPGRADE, true);
    service->GetBoolValue(ACCESSIBILITY_SCREENREADER_ENABLED, value.isScreenReaderEnabled);
    service->DeleteInstance();
}

RetError AccessibilityAccountData::EnableAbility(const std::string &name, const uint32_t capabilities,
    const std::string &callerBundleName)
{
    HILOG_DEBUG("start and name[%{public}s] capabilities[%{public}d]", name.c_str(), capabilities);
    RetError ret = accessibleAbilityManager_.UpdateInstalledAbility(name, capabilities);
    if (ret == RET_ERR_NOT_INSTALLED) {
        GetInstalledAbilitiesFromBMS();
        ret = accessibleAbilityManager_.UpdateInstalledAbility(name, capabilities);
    }
    if (ret != RET_OK) {
        HILOG_ERROR("the ability[%{public}s] is not installed", name.c_str());
        return ret;
    }

    // Add enabled ability
    const std::vector<std::string>& enabledAbilities = accessibleAbilityManager_.GetEnabledAbilities();
    if (std::any_of(enabledAbilities.begin(), enabledAbilities.end(),
        [name](const std::string &abilityName) {
            return abilityName == name;
        })) {
        HILOG_ERROR("The ability[%{public}s] is already enabled", name.c_str());
        return RET_ERR_CONNECTION_EXIST;
    }

    if (name == screenReaderAbilityName_ && !DealWithScreenReaderState()) {
        return RET_OK;
    }

    if (GetWaitDisConnectAbility(name)) {
        sptr<AccessibleAbilityConnection> connection = GetWaitDisConnectAbility(name);
        if (connection != nullptr && connection->GetIsRegisterDisconnectCallback()) {
            connection->DisconnectAbility();
        }
        RemoveWaitDisconnectAbility(name);
    }

#ifdef OHOS_BUILD_ENABLE_HITRACE
    HITRACE_METER_NAME(HITRACE_TAG_ACCESSIBILITY_MANAGER, "EnableAbility:" + name);
#endif // OHOS_BUILD_ENABLE_HITRACE

    accessibleAbilityManager_.AddEnabledAbility(name);
    SetAbilityAutoStartState(name, true);
    if (name == screenReaderAbilityName_) {
        SetScreenReaderState(screenReaderKey_, "1");
    }
    UpdateAbilities(callerBundleName);
    Utils::RecordStartingA11yEvent(name);
    return RET_OK;
}

bool AccessibilityAccountData::DealWithScreenReaderState()
{
    bool ignoreStateCache = config_->GetDbHandle()->GetBoolValue(IGNORE_REPEATED_CLICK_EXCLUDE_FLAG, false);
    bool ignoreRepeatClickState = config_->GetIgnoreRepeatClickState();
    HILOG_DEBUG("DealWithScreenReaderState ignoreStateCache: [%{public}d], ignoreRepeatClickState: [%{public}d] ",
        ignoreStateCache, ignoreRepeatClickState);
    if (!ignoreStateCache && !ignoreRepeatClickState) {
        return true;
    }
 
    std::shared_ptr<AccessibilityShortkeyDialog> shortkeyDialog = std::make_shared<AccessibilityShortkeyDialog>();
    // dialog
    if (shortkeyDialog->ConnectDialog(ShortKeyDialogType::READER_EXCLUSIVE)) {
        HILOG_DEBUG("ready to build screen reader exclusive dialog");
    }
    return false;
}

bool AccessibilityAccountData::GetInstalledAbilitiesFromBMS()
{
    HILOG_DEBUG("start.");
#ifdef OHOS_BUILD_ENABLE_HITRACE
    HITRACE_METER_NAME(HITRACE_TAG_ACCESSIBILITY_MANAGER, "QueryInstalledAbilityInfo");
#endif // OHOS_BUILD_ENABLE_HITRACE
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    Singleton<AccessibilityResourceBundleManager>::GetInstance().QueryExtensionAbilityInfos(
        AppExecFwk::ExtensionAbilityType::ACCESSIBILITY, id_, extensionInfos);
    HILOG_DEBUG("query extensionAbilityInfos' size is %{public}zu.", extensionInfos.size());
    for (auto& info : extensionInfos) {
        AccessibilityAbilityInitParams initParams;
        Utils::Parse(info, initParams);
        std::shared_ptr<AccessibilityAbilityInfo> accessibilityInfo =
            std::make_shared<AccessibilityAbilityInfo>(initParams);
        AddInstalledAbility(*accessibilityInfo);
    }
    return true;
}

void AccessibilityAccountData::InitScreenReaderStateObserver()
{
    if (!config_) {
        HILOG_ERROR("config is nullptr!");
        return;
    }
    if (!config_->GetDbHandle()) {
        HILOG_ERROR("helper is null!");
        return;
    }

    AccessibilitySettingObserver::UpdateFunc callback = [this](const std::string& state) {
        OnTouchGuideStateChanged();
    };
    RetError ret = config_->GetDbHandle()->RegisterObserver(ACCESSIBILITY_TOUCH_GUIDE_ENABLED, callback);
    if (ret != RET_OK) {
        HILOG_ERROR("register touch guide observer failed, ret = %{public}d", ret);
    }

    isSingleClickMode_ = config_->GetDbHandle()->GetBoolValue(SCREEN_READER_SINGLE_CLICK_MODE, false, true);
    AccessibilitySettingObserver::UpdateFunc func = [this](const std::string& state) {
        OnSingleClickModeChanged();
    };
    ret = config_->GetDbHandle()->RegisterObserver(SCREEN_READER_SINGLE_CLICK_MODE, func);
    if (ret != RET_OK) {
        HILOG_ERROR("register touch mode observer failed, ret = %{public}d", ret);
    }
}

void AccessibilityAccountData::Init()
{
    HILOG_DEBUG("Init start.");
    if (!config_) {
        config_ = std::make_shared<AccessibilitySettingsConfig>(id_);
        config_->Init();
    } else {
        config_->InitSetting();
    }

    GetWindowManager().RegisterWindowListener(
        Singleton<AccessibleAbilityManagerService>::GetInstance().GetMainHandler());
    bool result = GetWindowManager().Init();
    HILOG_ERROR("wms init result is %{public}d", result);

    ErrCode rtn = AccountSA::OsAccountManager::GetOsAccountType(id_, accountType_);
    if (rtn != ERR_OK) {
        HILOG_ERROR("get account type failed for accountId [%{public}d]", id_);
    }

    InitScreenReaderStateObserver();

    std::shared_ptr<AccessibilitySettingProvider> service =
        AccessibilitySettingProvider::GetInstance(POWER_MANAGER_SERVICE_ID);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }
    bool cloneState = false;
    service->GetBoolValue(ACCESSIBILITY_CLONE_FLAG, cloneState);
    if (cloneState == true) {
        service->PutBoolValue(ACCESSIBILITY_CLONE_FLAG, false);
    }
    if (id_ != DEFAULT_ACCOUNT_ID) {
        HILOG_WARN("id != default_account_id.");
        return;
    }

    HILOG_INFO("register clone observer.");
    AccessibilitySettingObserver::UpdateFunc func = [ = ](const std::string& state) {
        Singleton<AccessibleAbilityManagerService>::GetInstance().OnDataClone();
    };
    RetError ret = service->RegisterObserver(ACCESSIBILITY_CLONE_FLAG, func);
    if (ret != RET_OK) {
        HILOG_WARN("register clone observer failed %{public}d.", ret);
    }
}

void AccessibilityAccountData::AddConfigCallback(
    const sptr<IAccessibleAbilityManagerConfigObserver>& callback)
{
    HILOG_DEBUG("AddConfigCallback start.");
    std::lock_guard<ffrt::mutex> lock(configCallbacksMutex_);
    configCallbacks_.push_back(callback);
}

const std::vector<sptr<IAccessibleAbilityManagerConfigObserver>> AccessibilityAccountData::GetConfigCallbacks()
{
    HILOG_DEBUG("GetConfigCallbacks start.");
    std::lock_guard<ffrt::mutex> lock(configCallbacksMutex_);
    std::vector<sptr<IAccessibleAbilityManagerConfigObserver>> rtnVec = configCallbacks_;
    return rtnVec;
}

void AccessibilityAccountData::SetConfigCallbacks(std::vector<sptr<IAccessibleAbilityManagerConfigObserver>>& observer)
{
    HILOG_DEBUG("SetConfigCallbacks start.");
    std::lock_guard<ffrt::mutex> lock(configCallbacksMutex_);
    configCallbacks_ = observer;
}

void AccessibilityAccountData::RemoveConfigCallback(const wptr<IRemoteObject>& callback)
{
    HILOG_DEBUG("RemoveConfigCallback start.");
    std::lock_guard<ffrt::mutex> lock(configCallbacksMutex_);
    for (auto itr = configCallbacks_.begin(); itr != configCallbacks_.end(); itr++) {
        if ((*itr)->AsObject() == callback) {
            configCallbacks_.erase(itr);
            break;
        }
    }
}

std::shared_ptr<AccessibilitySettingsConfig> AccessibilityAccountData::GetConfig()
{
    HILOG_DEBUG("GetConfig start.");
    return config_;
}

void AccessibilityAccountData::GetImportantEnabledAbilities(
    std::map<std::string, uint32_t> &importantEnabledAbilities) const
{
    accessibleAbilityManager_.GetImportantEnabledAbilities(importantEnabledAbilities);
}

void AccessibilityAccountData::UpdateImportantEnabledAbilities(
    std::map<std::string, uint32_t> &importantEnabledAbilities)
{
    accessibleAbilityManager_.UpdateImportantEnabledAbilities(importantEnabledAbilities);
}

void AccessibilityAccountData::UpdateAutoStartEnabledAbilities()
{
    HILOG_DEBUG();
    if (id_ == -1) {
        HILOG_DEBUG("Current user is -1.");
        return;
    }
    accessibleAbilityManager_.UpdateAutoStartEnabledAbilities(
        [this](const std::string &name) { return GetAbilityAutoStartState(name); });
}

uint32_t AccessibilityAccountData::GetInputFilterFlag() const
{
    HILOG_DEBUG();
    if (!config_) {
        HILOG_ERROR("config is null");
        return 0;
    }
    uint32_t flag = 0;
    if (config_->GetScreenMagnificationState()) {
        flag |= FEATURE_SCREEN_MAGNIFICATION;
    }
    if (isEventTouchGuideState_) {
        flag |= FEATURE_TOUCH_EXPLORATION;
    }
    if (isFilteringKeyEvents_) {
        flag |= FEATURE_FILTER_KEY_EVENTS;
    }
    if (isGesturesSimulation_) {
        flag |= FEATURE_INJECT_TOUCH_EVENTS;
    }
    if (config_->GetMouseKeyState()) {
        flag |= FEATURE_MOUSE_KEY;
    }

    int32_t autoclickTime = config_->GetMouseAutoClick();
    if (autoclickTime >= AUTOCLICK_DELAY_TIME_MIN && autoclickTime <= AUTOCLICK_DELAY_TIME_MAX) {
        flag |= FEATURE_MOUSE_AUTOCLICK;
    }

    uint32_t clickResponseTime = config_->GetClickResponseTime();
    bool ignoreRepeatClickState = config_->GetIgnoreRepeatClickState();
    if (clickResponseTime > 0 || ignoreRepeatClickState == true) {
        flag |= FEATURE_SCREEN_TOUCH;
    }

    return flag;
}

void AccessibilityAccountData::NotifyExtensionServiceDeath(const std::string& uri)
{
    HILOG_DEBUG("Notifying extension service death: uri=%{public}s", uri.c_str());
    CallEnableAbilityCallback(uri);
}

void AccessibilityAccountData::CallEnableAbilityCallback(const std::string &uri)
{
    accessibleAbilityManager_.CallEnableAbilityCallback(uri);
}

void AccessibilityAccountData::AddAppStateObserverAbility(
    const std::string& uri, const sptr<AccessibleAbilityConnection>& connection)
{
    accessibleAbilityManager_.AddAppStateObserverAbility(uri, connection);
}

void AccessibilityAccountData::RemoveAppStateObserverAbility(const std::string& uri)
{
    accessibleAbilityManager_.RemoveAppStateObserverAbility(uri);
}

sptr<AccessibleAbilityConnection> AccessibilityAccountData::GetAppStateObserverAbility(const std::string& uri)
{
    return accessibleAbilityManager_.GetAppStateObserverAbility(uri);
}

void AccessibilityAccountData::UpdateAbilities(std::string callerBundleName)
{
    accessibleAbilityManager_.UpdateAbilities(
        callerBundleName,
        id_,
        connectCounter_,
        [this](int32_t accountId, int32_t counter, AccessibilityAbilityInfo& info) {
            return new(std::nothrow) AccessibleAbilityConnection(accountId, counter, info, this);
        });
}

bool AccessibilityAccountData::RemoveAbility(const std::string &bundleName)
{
    HILOG_DEBUG("bundleName(%{public}s)", bundleName.c_str());
    bool result = accessibleAbilityManager_.RemoveAbility(bundleName);
    if (result) {
        UpdateAbilities();
    }
    return result;
}

void AccessibilityAccountData::AddAbility(const std::string &bundleName)
{
    HILOG_DEBUG("bundleName(%{public}s)", bundleName.c_str());

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    Singleton<AccessibilityResourceBundleManager>::GetInstance().QueryExtensionAbilityInfos(
        AppExecFwk::ExtensionAbilityType::ACCESSIBILITY, id_, extensionInfos);
    HILOG_DEBUG("query extensionAbilityInfos' size is %{public}zu.", extensionInfos.size());
    
    std::vector<AccessibilityAbilityInfo> accessibilityInfos;
    for (auto &newAbility : extensionInfos) {
        if (newAbility.bundleName == bundleName) {
            HILOG_DEBUG("The package%{public}s added", (bundleName + "/" + newAbility.name).c_str());
            AccessibilityAbilityInitParams initParams;
            Utils::Parse(newAbility, initParams);
            accessibilityInfos.emplace_back(initParams);
        }
    }

    if (!accessibilityInfos.empty()) {
        accessibleAbilityManager_.AddAbility(
            bundleName,
            accessibilityInfos,
            [this](const std::string& name) { return GetAbilityAutoStartState(name); }
        );
        UpdateAbilities();
    }
}

void AccessibilityAccountData::ChangeAbility(const std::string &bundleName)
{
    HILOG_DEBUG("bundleName(%{public}s)", bundleName.c_str());

    std::vector<AccessibilityAbilityInfo> installedAbilities = accessibleAbilityManager_.GetInstalledAbilities();
    if (installedAbilities.empty()) {
        HILOG_DEBUG("There is no installed abilities, refreshing added bundle.");
        AddAbility(bundleName);
        return;
    }
    
    std::vector<std::string> autoStartAbilities;
    for (auto &ability : installedAbilities) {
        if (ability.GetPackageName() != bundleName) {
            continue;
        }
        if (GetAbilityAutoStartState(ability.GetId())) {
            autoStartAbilities.push_back(ability.GetId());
        }
    }
    
    accessibleAbilityManager_.ChangeAbility(
        bundleName,
        [this](const std::string& name) { return GetAbilityAutoStartState(name); },
        [this](const std::string& name, bool state) { SetAbilityAutoStartState(name, state); }
    );
    
    AddAbility(bundleName);
}

void AccessibilityAccountData::AddUITestClient(const sptr<IRemoteObject> &obj,
    const std::string &bundleName, const std::string &abilityName)
{
    HILOG_DEBUG();
    // Add installed ability
    std::shared_ptr<AccessibilityAbilityInfo> abilityInfo = std::make_shared<AccessibilityAbilityInfo>();
    if (abilityInfo == nullptr) {
        return;
    }
    abilityInfo->SetPackageName(bundleName);
    abilityInfo->SetName(abilityName);
    uint32_t capabilities = CAPABILITY_RETRIEVE;
    abilityInfo->SetCapabilityValues(capabilities);
    abilityInfo->SetAccessibilityAbilityType(ACCESSIBILITY_ABILITY_TYPE_ALL);
    abilityInfo->SetEventTypes(EventType::TYPES_ALL_MASK);
    AddInstalledAbility(*abilityInfo);

    // Add connected ability
    sptr<AppExecFwk::ElementName> elementName = new(std::nothrow) AppExecFwk::ElementName();
    if (!elementName) {
        HILOG_ERROR("elementName is null");
        return;
    }
    elementName->SetBundleName(bundleName);
    elementName->SetAbilityName(abilityName);
    sptr<AccessibleAbilityConnection> connection = new(std::nothrow) AccessibleAbilityConnection(
        id_, connectCounter_++, *abilityInfo, this);
    if (!connection) {
        HILOG_ERROR("connection is null");
        return;
    }
    connection->OnAbilityConnectDoneSync(*elementName, obj);
}

void AccessibilityAccountData::RemoveUITestClient(sptr<AccessibleAbilityConnection> &connection,
    const std::string &bundleName)
{
    HILOG_DEBUG();
    if (!connection) {
        HILOG_ERROR("connection is nullptr");
        return;
    }
    RemoveInstalledAbility(bundleName);
    RemoveConnectedAbility(connection->GetElementName());
    connection->OnAbilityDisconnectDoneSync(connection->GetElementName());
}

AccountSA::OsAccountType AccessibilityAccountData::GetAccountType()
{
    return accountType_;
}

void AccessibilityAccountData::OnTouchGuideStateChanged()
{
    HILOG_INFO();

    Singleton<AccessibleAbilityManagerService>::GetInstance().UpdateAccessibilityManagerService();
    if (config_->GetDbHandle() == nullptr) {
        HILOG_ERROR("helper is nullptr!");
        return;
    }
    if (!config_->GetDbHandle()->GetBoolValue(ACCESSIBILITY_TOUCH_GUIDE_ENABLED, true)) {
        GetElementOperatorManager().ExecuteActionOnAccessibilityFocused(ACCESSIBILITY_ACTION_CLEAR_ACCESSIBILITY_FOCUS);
    }
}

void AccessibilityAccountData::OnSingleClickModeChanged()
{
    if (!config_) {
        HILOG_ERROR("config is nullptr!");
        return;
    }
    if (config_->GetDbHandle() == nullptr) {
        HILOG_ERROR("helper is nullptr!");
        return;
    }

    isSingleClickMode_ = config_->GetDbHandle()->GetBoolValue(SCREEN_READER_SINGLE_CLICK_MODE, false, true);
    HILOG_INFO("screen reader single click mode = %{public}d", isSingleClickMode_);
    Singleton<AccessibleAbilityManagerService>::GetInstance().UpdateAccessibilityState();
}

sptr<AccessibilityAccountData> AccessibilityAccountDataMap::AddAccountData(
    int32_t accountId)
{
    std::lock_guard<ffrt::mutex> lock(accountDataMutex_);
    auto iter = accountDataMap_.find(accountId);
    if (iter != accountDataMap_.end()) {
        HILOG_WARN("accountId is existed");
        return iter->second;
    }

    sptr<AccessibilityAccountData> accountData = new(std::nothrow) AccessibilityAccountData(accountId);
    if (accountData == nullptr) {
        HILOG_ERROR("accountData is null");
        return nullptr;
    }

    accountData->Init();
    accountDataMap_[accountId] = accountData;
    return accountData;
}

sptr<AccessibilityAccountData> AccessibilityAccountDataMap::GetCurrentAccountData(
    int32_t accountId)
{
    std::lock_guard<ffrt::mutex> lock(accountDataMutex_);
    auto iter = accountDataMap_.find(accountId);
    if (iter != accountDataMap_.end()) {
        return iter->second;
    }

    sptr<AccessibilityAccountData> accountData = new(std::nothrow) AccessibilityAccountData(accountId);
    if (!accountData) {
        HILOG_ERROR("accountData is null");
        return nullptr;
    }

    accountDataMap_[accountId] = accountData;
    return accountData;
}

sptr<AccessibilityAccountData> AccessibilityAccountDataMap::GetAccountData(
    int32_t accountId)
{
    std::lock_guard<ffrt::mutex> lock(accountDataMutex_);
    auto iter = accountDataMap_.find(accountId);
    if (iter != accountDataMap_.end()) {
        return iter->second;
    }

    sptr<AccessibilityAccountData> accountData = new(std::nothrow) AccessibilityAccountData(accountId);
    if (!accountData) {
        HILOG_ERROR("accountData is null");
        return nullptr;
    }

    accountDataMap_[accountId] = accountData;
    return accountData;
}

sptr<AccessibilityAccountData> AccessibilityAccountDataMap::RemoveAccountData(
    int32_t accountId)
{
    sptr<AccessibilityAccountData> accountData = nullptr;
    std::lock_guard<ffrt::mutex> lock(accountDataMutex_);
    auto iter = accountDataMap_.find(accountId);
    if (iter != accountDataMap_.end()) {
        accountData = iter->second;
        accountDataMap_.erase(iter);
    }

    return accountData;
}

std::vector<int32_t> AccessibilityAccountDataMap::GetAllAccountIds()
{
    std::lock_guard<ffrt::mutex> lock(accountDataMutex_);
    std::vector<int32_t> accountIds;
    for (auto it = accountDataMap_.begin(); it != accountDataMap_.end(); it++) {
        accountIds.push_back(it->first);
    }
    return accountIds;
}

void AccessibilityAccountDataMap::Clear()
{
    std::lock_guard<ffrt::mutex> lock(accountDataMutex_);
    accountDataMap_.clear();
}

void AccessibilityAccountData::isSendEvent(const AccessibilityEventInfo &eventInfo)
{
    std::map<std::string, sptr<AccessibleAbilityConnection>> abilities = GetConnectedA11yAbilities();
    uint32_t eventType = eventInfo.GetEventType();
    std::string bundleName = "";
    std::lock_guard<ffrt::mutex> lock(abilityNeedEventsMutex_);
    for (auto &ability : abilities) {
        size_t pos = ability.first.find('/');
        if (pos != std::string::npos) {
            bundleName = ability.first.substr(0, pos);
        }

        HILOG_DEBUG("send event type is %{public}d, bundle name is %{public}s", eventType, bundleName.c_str());
        auto it = abilityNeedEvents_.find(bundleName);
        if (it != abilityNeedEvents_.end()) {
            uint32_t needEventSize = it->second.size();
            if (needEventSize == 0) { // default
                ability.second->OnAccessibilityEvent(const_cast<AccessibilityEventInfo&>(eventInfo));
            } else {
                uint32_t event = it->second.at(0);
                if (event == TYPES_ALL_MASK) {  // all event
                    ability.second->OnAccessibilityEvent(const_cast<AccessibilityEventInfo&>(eventInfo));
                    continue;
                }

                if (event == TYPE_VIEW_INVALID) {  // none event
                    continue;
                }

                auto isEvent = std::find(it->second.begin(), it->second.end(), eventType);
                if (isEvent != it->second.end()) {
                    ability.second->OnAccessibilityEvent(const_cast<AccessibilityEventInfo&>(eventInfo));
                }
            }
        }
    }
}

void AccessibilityAccountData::UpdateAbilityNeedEvent(const std::string &name, std::vector<uint32_t> needEvents)
{
    std::string packageName = "";
    std::lock_guard<ffrt::mutex> lock(abilityNeedEventsMutex_);
    if (name == SCREEN_READER_BUNDLE_NAME) {
        abilityNeedEvents_[name].push_back(TYPES_ALL_MASK);
    } else if (name == UI_TEST_ABILITY_NAME) {
        abilityNeedEvents_[name].clear();
        abilityNeedEvents_[name] = needEvents;
    } else {
        abilityNeedEvents_[name] = needEvents;
        std::vector<AccessibilityAbilityInfo> installedAbilities = accessibleAbilityManager_.GetInstalledAbilities();
        for (auto &installAbility : installedAbilities) {
            packageName = installAbility.GetPackageName();
            if (packageName == name) {
                installAbility.GetEventConfigure(abilityNeedEvents_[name]);
            }
        }
    }
    HILOG_DEBUG("abilityNeedEvents_ size is %{public}zu, needEvent size is %{public}zu",
        abilityNeedEvents_.size(), abilityNeedEvents_[name].size());
    UpdateNeedEvents();
}

void AccessibilityAccountData::RemoveNeedEvent(const std::string &name)
{
    size_t pos = name.find('/');
    if (pos != std::string::npos) {
        std::lock_guard<ffrt::mutex> lock(abilityNeedEventsMutex_);
        std::string bundleName = name.substr(0, pos);
        HILOG_DEBUG("RemoveNeedEvent bundleName is %{public}s, abilityNeedEvents_ size is %{public}zu",
            bundleName.c_str(), abilityNeedEvents_.size());
        abilityNeedEvents_.erase(bundleName);
        UpdateNeedEvents();
    }
}

std::vector<uint32_t> AccessibilityAccountData::UpdateNeedEvents()
{
    needEvents_.clear();
    std::vector<uint32_t> needEvents = {};
    for (const auto& pair : abilityNeedEvents_) {
        const std::vector<uint32_t>& events = pair.second;
        if (events.size() == 0) { // A certain extension service is in the default state.
            needEvents_ = needEvents;
            HILOG_DEBUG("default state, needEvent size is %{public}zu",
                needEvents_.size());
            return needEvents_;
        }

        for (uint32_t event : events) {
            if (std::find(needEvents.begin(), needEvents.end(), event) == needEvents.end()) {
                needEvents.push_back(event);
            }
        }
    }

    if (std::find(needEvents.begin(), needEvents.end(), TYPES_ALL_MASK) != needEvents.end()) {
        needEvents_.push_back(TYPES_ALL_MASK);
    } else if ((needEvents.size() == 1) &&
        std::find(needEvents.begin(), needEvents.end(), TYPE_VIEW_INVALID) != needEvents.end()) {
        needEvents_.push_back(TYPE_VIEW_INVALID);
    } else {
        needEvents_ = needEvents;
    }
    HILOG_INFO("needEvents size is %{public}zu", needEvents_.size());
    return needEvents_;
}

std::vector<uint32_t> AccessibilityAccountData::GetNeedEvents()
{
    return needEvents_;
}

RetError AccessibilityAccountData::ConfigureEvents(std::vector<uint32_t> needEvents)
{
    UpdateAbilityNeedEvent(UI_TEST_ABILITY_NAME, needEvents);
    uint32_t state = GetAccessibilityState();
    state |= STATE_CONFIG_EVENT_CHANGE;
    stateObservers_.OnStateObservers(state);
    return RET_OK;
}

void AccountSubscriber::OnStateChanged(const AccountSA::OsAccountStateData &data)
{
    if (data.state == AccountSA::OsAccountState::SWITCHING) {
        HILOG_INFO("account switching.");
        if (Singleton<ExtendManagerServiceProxy>::GetInstance().CheckExtProxyStatus()) {
            if (Singleton<ExtendManagerServiceProxy>::GetInstance().LoadExtProxy()) {
                Singleton<ExtendManagerServiceProxy>::GetInstance().OffZoomGesture();
            }
        }
    }
}

RetError AccessibilityAccountData::RegisterStateObserver(
    const sptr<IAccessibleAbilityManagerStateObserver> &stateObserver, uint32_t &state)
{
    HILOG_DEBUG();
    if (!stateObserver) {
        HILOG_ERROR("parameters check failed!");
        return RET_ERR_INVALID_PARAM;
    }
    auto id = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::HapTokenInfo info;
    auto result = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(id, info);
    if (result != 0) {
        HILOG_ERROR("get native token info failed!, result: %{public}d", result);
        return RET_ERR_INVALID_PARAM;
    }

    std::string bundleName = info.bundleName;
    auto instIndex = info.instIndex;
    stateObservers_.AddStateObserver(stateObserver, Utils::GetSeniorModeStateKey(bundleName, instIndex));
    state = GetAccessibilityState();
    return RET_OK;
}

uint32_t AccessibilityAccountData::UpdateAccessibilityState()
{
    uint32_t state = GetAccessibilityState();
    if (!(state & STATE_ACCESSIBILITY_ENABLED)) {
        GetWindowManager().ClearAccessibilityFocused();
    }
    stateObservers_.OnStateObservers(state);
    return state;
}
 
void AccessibilityAccountData::RemoveStateObserver(const wptr<IRemoteObject> &remote)
{
    stateObservers_.RemoveStateObserver(remote);
}
 
void AccessibilityAccountData::StateObservers::AddStateObserver(
    const sptr<IAccessibleAbilityManagerStateObserver>& stateObserver, const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> lock(stateObserversMutex_);
    auto iter = observersMap_.find(bundleName);
    if (iter == observersMap_.end()) {
        observersMap_[bundleName] = stateObserver;
        HILOG_DEBUG("register state observer successfully");
        return;
    }
    HILOG_INFO("state observer is existed");
}
 
void AccessibilityAccountData::StateObservers::OnStateObservers(uint32_t state)
{
    HILOG_INFO("state is %{public}d", state);
    std::lock_guard<ffrt::mutex> lock(stateObserversMutex_);

    for (auto& stateObserver : observersMap_) {
        if (stateObserver.second) {
            stateObserver.second->OnStateChanged(state);
        }
    }
}

void AccessibilityAccountData::StateObservers::OnSeniorModeStateObservers(const std::string &bundleName,
    int32_t appIndex, bool seniorModeStateForApp)
{
    HILOG_INFO("bundleName: %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
    std::string mapKdy = Utils::GetSeniorModeStateKey(bundleName, appIndex);

    auto iter = observersMap_.find(mapKdy);
    if (iter != observersMap_.end()) {
        auto stateObserver = observersMap_[mapKdy];
        if (stateObserver) {
            sptr<AccessibilityAccountData> accountData =
                Singleton<AccessibleAbilityManagerService>::GetInstance().GetCurrentAccountData();
            uint32_t state = accountData->GetAccessibilityState();
            if (seniorModeStateForApp) {
                state |= STATE_SELF_SENIOR_MODE_STATE_ENABLED;
            }
            stateObserver->OnStateChanged(state);
            HILOG_INFO("OnSeniorModeStateObservers, state: %{public}d", state);
        }
        return;
    }
}

void AccessibilityAccountData::StateObservers::RemoveStateObserver(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<ffrt::mutex> lock(stateObserversMutex_);
    for (auto iter = observersMap_.begin(); iter != observersMap_.end(); ++iter) {
        if (iter->second && iter->second->AsObject() == remote) {
            HILOG_DEBUG("RemoveStateObserver");
            observersMap_.erase(iter);
            return;
        }
    }
}
 
void AccessibilityAccountData::StateObservers::Clear()
{
    std::lock_guard<ffrt::mutex> lock(stateObserversMutex_);
    observersMap_.clear();
}

void AccessibilityAccountData::AddSeniorModeStateObserver(
    const sptr<IAccessibilityAppSeniorModeStateObserver>& observer)
{
    std::lock_guard<ffrt::mutex> lock(seniorModeStateObserversMutex_);
    HILOG_INFO();
    if (std::any_of(seniorModeStateObservers_.begin(), seniorModeStateObservers_.end(),
        [observer](const sptr<IAccessibilityAppSeniorModeStateObserver> &listObserver) {
            return listObserver == observer;
        })) {
        HILOG_ERROR("observer is already exist");
        return;
    }
    seniorModeStateObservers_.push_back(observer);
    HILOG_DEBUG("observer's size is %{public}zu", seniorModeStateObservers_.size());
}

void AccessibilityAccountData::RemoveSeniorModeStateObserver(const wptr<IRemoteObject>& observer)
{
    std::lock_guard<ffrt::mutex> lock(seniorModeStateObserversMutex_);
    HILOG_DEBUG();
    auto itr = seniorModeStateObservers_.begin();
    for (; itr != seniorModeStateObservers_.end(); itr++) {
        if ((*itr)->AsObject() == observer) {
            HILOG_DEBUG("erase observer");
            seniorModeStateObservers_.erase(itr);
            HILOG_DEBUG("observer's size is %{public}zu", seniorModeStateObservers_.size());
            return;
        }
    }
}

void AccessibilityAccountData::NotifySeniorModeStateObservers(const std::string& bundleName,
    int32_t appIndex, bool state)
{
    std::lock_guard<ffrt::mutex> lock(seniorModeStateObserversMutex_);
    HILOG_INFO("observer's size is %{public}zu", seniorModeStateObservers_.size());
    for (auto &observer : seniorModeStateObservers_) {
        if (observer) {
            observer->OnSeniorModeStateChanged(bundleName, appIndex, state);
        }
    }
    stateObservers_.OnSeniorModeStateObservers(bundleName, appIndex, state);
}

void AccessibilityAccountData::GetSeniorModeStateForAppChanges(std::map<std::string, bool>& changes)
{
    constexpr char SENIOR_MODE_STATE_KEY[] = "accessibility_senior_mode_state_for_app";

    if (config_ == nullptr) {
        HILOG_ERROR("config is nullptr");
        return;
    }

    if (config_->GetDbHandle() == nullptr) {
        HILOG_ERROR("datashareHelper is nullptr");
        return;
    }

    std::string seniorModeStateInfo = config_->GetDbHandle()->GetStringValue(SENIOR_MODE_STATE_KEY, "{}");
    HILOG_INFO("seniorModeStateForApp from db: %{public}s", seniorModeStateInfo.c_str());

    std::map<std::string, bool> newMap;
    ParseSeniorModeStateJson(seniorModeStateInfo, newMap);

    std::map<std::string, bool> oldMap = config_->GetSeniorModeStateForAppMap();
    CompareSeniorModeStateMap(newMap, oldMap, changes);

    config_->SetSeniorModeStateForAppMap(newMap);
}

void AccessibilityAccountData::ParseSeniorModeStateJson(const std::string& jsonStr, std::map<std::string, bool>& map)
{
    if (!nlohmann::json::accept(jsonStr)) {
        HILOG_ERROR("Invalid json format");
        return;
    }

    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr);
    if (!jsonObj.is_object()) {
        HILOG_ERROR("Json is not an object");
        return;
    }

    for (auto& item : jsonObj.items()) {
        if (item.value().is_boolean()) {
            map[item.key()] = item.value().get<bool>();
        }
    }
}

void AccessibilityAccountData::CompareSeniorModeStateMap(const std::map<std::string, bool>& newMap,
    const std::map<std::string, bool>& oldMap, std::map<std::string, bool>& changes)
{
    for (const auto& item : newMap) {
        const std::string& key = item.first;
        bool newValue = item.second;

        bool isChanged = false;
        auto oldIter = oldMap.find(key);
        if (oldIter == oldMap.end()) {
            isChanged = true;
        } else if (oldIter->second != newValue) {
            isChanged = true;
        }

        if (isChanged) {
            HILOG_INFO("Senior mode state changed for %{public}s: %{public}d", key.c_str(), newValue);
            changes[key] = newValue;
        }
    }

    for (const auto& item : oldMap) {
        const std::string& key = item.first;
        if (newMap.find(key) == newMap.end()) {
            HILOG_INFO("Senior mode state removed for %{public}s", key.c_str());
            changes[key] = false;
        }
    }
}
// LCOV_EXCL_STOP
} // namespace Accessibility
} // namespace OHOS
