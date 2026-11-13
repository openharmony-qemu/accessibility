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

// LCOV_EXCL_START
#include <iostream>
#include <charconv>
#include <string>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "accessibility_config.h"
#include "accessibility_system_ability_client.h"
#include "accessibility_def.h"
#include "nativetoken_kit.h"
#include "privacy_kit.h"
#include "ipc_skeleton.h"
#include "token_setproc.h"

using namespace OHOS;
using namespace Accessibility;

constexpr int32_t ARGC_COUNT_TWO = 2;

bool PrepareRootAccessibilityToken()
{
    if (getuid() != 0) {
        return true;
    }

    static const char *permissions[] = {"ohos.permission.WRITE_ACCESSIBILITY_CONFIG"};
    NativeTokenInfoParams tokenInfo{};
    tokenInfo.permsNum = 1;
    tokenInfo.perms = permissions;
    tokenInfo.processName = "qemu_accessibility_cli";
    tokenInfo.aplStr = "system_core";
    tokenInfo.uid = 0;
    uint64_t tokenId = GetAccessTokenId(&tokenInfo);
    return tokenId != 0 && SetSelfTokenID(tokenId) == 0;
}

void PrintHelp(const std::string& subcommand = "")
{
    if (subcommand.empty()) {
        std::cout << "ohos-a11yManager - Accessibility management utility for configuring screen reader, " \
                "magnification, high contrast, and other accessibility features on the system" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  ohos-a11yManager [command] [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "Parameters:" << std::endl;
        std::cout << "  --help                  Display this help message" << std::endl;
        std::cout << std::endl;
        std::cout << "SubCommands:" << std::endl;
        std::cout << "  state-is-screen-reader-enabled      Check if screen reader is enabled" << std::endl;
        std::cout << "  ability-enable                       Enable an accessibility ability" << std::endl;
        std::cout << "  ability-disable                      Disable an accessibility ability" << std::endl;
        std::cout << "  ability-enable-screen-reader          Enable screen reader" << std::endl;
        std::cout << "  ability-disable-screen-reader         Disable screen reader" << std::endl;
        std::cout << "  magnification-set-state               Set screen magnification state" << std::endl;
        std::cout << "  magnification-get-state               Get screen magnification state" << std::endl;
        std::cout << "  shortkey-set-state                    Set shortkey state" << std::endl;
        std::cout << "  shortkey-get-state                    Get shortkey state" << std::endl;
        std::cout << "  high-contrast-set-state               Set high contrast text state" << std::endl;
        std::cout << "  high-contrast-get-state               Get high contrast text state" << std::endl;
        std::cout << "  invert-color-set-state                Set invert color state" << std::endl;
        std::cout << "  invert-color-get-state                Get invert color state" << std::endl;
        std::cout << "  animation-off-set-state               Set animation off state" << std::endl;
        std::cout << "  animation-off-get-state               Get animation off state" << std::endl;
        std::cout << "  audio-set-mono                        Set audio mono state" << std::endl;
        std::cout << "  audio-get-mono                        Get audio mono state" << std::endl;
        std::cout << "  audio-set-balance                     Set audio balance" << std::endl;
        std::cout << "  audio-get-balance                     Get audio balance" << std::endl;
        std::cout << "  daltonization-set-state               Set daltonization state" << std::endl;
        std::cout << "  daltonization-get-state               Get daltonization state" << std::endl;
        std::cout << "  daltonization-set-filter              Set daltonization filter type" << std::endl;
        std::cout << "  daltonization-get-filter              Get daltonization filter type" << std::endl;
        std::cout << "  click-set-response-time               Set click response time" << std::endl;
        std::cout << "  click-get-response-time               Get click response time" << std::endl;
        std::cout << "  ignore-repeat-click-set-state         Set ignore repeat click state" << std::endl;
        std::cout << "  ignore-repeat-click-get-state         Get ignore repeat click state" << std::endl;
        std::cout << "  ignore-repeat-click-set-time          Set ignore repeat click interval" << std::endl;
        std::cout << "  ignore-repeat-click-get-time          Get ignore repeat click time" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  # Check if screen reader is enabled" << std::endl;
        std::cout << "  ohos-a11yManager state-is-screen-reader-enabled" << std::endl;
        std::cout << std::endl;
        std::cout << "  # Enable screen reader" << std::endl;
        std::cout << "  ohos-a11yManager ability-enable-screen-reader" << std::endl;
        std::cout << std::endl;
        std::cout << "  # Enable an installed accessibility extension" << std::endl;
        std::cout << "  ohos-a11yManager ability-enable --name com.example.app/AccessibilityExtAbility " \
            "--capabilities 31" << std::endl;
        std::cout << std::endl;
        std::cout << "  # Set magnification state" << std::endl;
        std::cout << "  ohos-a11yManager magnification-set-state --state true" << std::endl;
        std::cout << std::endl;
        std::cout << "  # View subcommand help" << std::endl;
        std::cout << "  ohos-a11yManager magnification-set-state --help" << std::endl;
    } else {
        std::cout << "ohos-a11yManager " << subcommand;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  ohos-a11yManager " << subcommand << " [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "Parameters:" << std::endl;

        if (subcommand == "magnification-set-state" || subcommand == "shortkey-set-state" ||
            subcommand == "high-contrast-set-state" || subcommand == "invert-color-set-state" ||
            subcommand == "animation-off-set-state" || subcommand == "audio-set-mono" ||
            subcommand == "daltonization-set-state" || subcommand == "ignore-repeat-click-set-state") {
            std::cout << "  --state     <value>    Enable or disable the feature (required, values: [true, false])" \
                << std::endl;
        } else if (subcommand == "ability-enable") {
            std::cout << "  --name          <value>    bundleName/abilityName (required)" << std::endl;
            std::cout << "  --capabilities  <value>    Capability bit mask (optional, range: 1 to 31, default: 31)" \
                << std::endl;
        } else if (subcommand == "ability-disable") {
            std::cout << "  --name          <value>    bundleName/abilityName (required)" << std::endl;
        } else if (subcommand == "audio-set-balance") {
            std::cout << "  --balance   <value>    Audio balance value (required, range: -1.0 to 1.0)" << std::endl;
        } else if (subcommand == "daltonization-set-filter") {
            std::cout << "  --type      <value>    Filter type (required, values: [0=Normal, 1=Protanomaly, " \
                "2=Deuteranomaly, 3=Tritanomaly])" << std::endl;
        } else if (subcommand == "click-set-response-time") {
            std::cout << "  --time      <value>    Click response time (required, values: [0=short(default), " \
                "1=medium, 2=long])" << std::endl;
        } else if (subcommand == "ignore-repeat-click-set-time") {
            std::cout << "  --interval  <value>    Interval value (required, values: [0=0.1s, 1=0.4s, 2=0.7s, " \
                "3=1.0s, 4=1.3s])" << std::endl;
        }
        std::cout << "  --help                  Display this help message" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;

        if (subcommand == "magnification-set-state" || subcommand == "shortkey-set-state" ||
            subcommand == "high-contrast-set-state" || subcommand == "invert-color-set-state" ||
            subcommand == "animation-off-set-state" || subcommand == "audio-set-mono" ||
            subcommand == "daltonization-set-state" || subcommand == "ignore-repeat-click-set-state") {
            std::cout << "  # Enable the feature" << std::endl;
            std::cout << "  ohos-a11yManager " << subcommand << " --state true" << std::endl;
            std::cout << std::endl;
            std::cout << "  # Disable the feature" << std::endl;
            std::cout << "  ohos-a11yManager " << subcommand << " --state false" << std::endl;
        } else if (subcommand == "ability-enable") {
            std::cout << "  # Enable an installed accessibility extension" << std::endl;
            std::cout << "  ohos-a11yManager ability-enable --name com.example.app/AccessibilityExtAbility " \
                "--capabilities 31" << std::endl;
        } else if (subcommand == "ability-disable") {
            std::cout << "  # Disable an installed accessibility extension" << std::endl;
            std::cout << "  ohos-a11yManager ability-disable --name " \
                "com.example.app/AccessibilityExtAbility" << std::endl;
        } else if (subcommand == "audio-set-balance") {
            std::cout << "  # Set audio balance to center" << std::endl;
            std::cout << "  ohos-a11yManager audio-set-balance --balance 0.0" << std::endl;
            std::cout << std::endl;
            std::cout << "  # Set audio balance to left" << std::endl;
            std::cout << "  ohos-a11yManager audio-set-balance --balance -1.0" << std::endl;
        } else if (subcommand == "daltonization-set-filter") {
            std::cout << "  # Set filter to Normal" << std::endl;
            std::cout << "  ohos-a11yManager daltonization-set-filter --type 0" << std::endl;
            std::cout << std::endl;
            std::cout << "  # Set filter to Protanomaly" << std::endl;
            std::cout << "  ohos-a11yManager daltonization-set-filter --type 1" << std::endl;
        } else if (subcommand == "click-set-response-time") {
            std::cout << "  # Set click response time to short" << std::endl;
            std::cout << "  ohos-a11yManager click-set-response-time --time 0" << std::endl;
            std::cout << std::endl;
            std::cout << "  # Set click response time to long" << std::endl;
            std::cout << "  ohos-a11yManager click-set-response-time --time 2" << std::endl;
        } else if (subcommand == "ignore-repeat-click-set-time") {
            std::cout << "  # Set interval to 0.1s" << std::endl;
            std::cout << "  ohos-a11yManager ignore-repeat-click-set-time --interval 0" << std::endl;
            std::cout << std::endl;
            std::cout << "  # Set interval to 1.0s" << std::endl;
            std::cout << "  ohos-a11yManager ignore-repeat-click-set-time --interval 3" << std::endl;
        } else {
            std::cout << "  # Get current state" << std::endl;
            std::cout << "  ohos-a11yManager " << subcommand << std::endl;
        }
    }
}

bool ConvertStringToInt64(std::string &str, int32_t &value)
{
    auto [ptr, errCode] = std::from_chars(str.data(), str.data() + str.size(), value);
    return errCode == std::errc{} && ptr == str.data() + str.size();
}
 
bool StringToFloat(const std::string& value, float& balance)
{
    errno = 0;
    char* pEnd = nullptr;
    float result = std::strtof(value.c_str(), &pEnd);
    if (pEnd == value.c_str() || errno == ERANGE) {
        return false;
    } else {
        balance = result;
        return true;
    }
}

// Parsing parameters
std::string GetOption(const std::vector<std::string>& args, const std::string& option)
{
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == option && i + 1 < args.size()) {
            return args[i + 1];
        }
    }
    return "";
}

bool HasOption(const std::vector<std::string>& args, const std::string& option)
{
    for (const auto& arg : args) {
        if (arg == option) {
            return true;
        }
    }
    return false;
}

// JSON Output Helper Function
int OutputSuccess(const std::string& data)
{
    nlohmann::json root;
    root["type"] = "result";
    root["status"] = "success";

    if (nlohmann::json::accept(data)) {
        root["data"] = nlohmann::json::parse(data);
    } else {
        root["data"] = data;
    }

    std::cout << root.dump() << std::endl;
    return 0;
}

int OutputError(const std::string& code, const std::string& message, const std::string& suggestion = "")
{
    nlohmann::json root;
    root["type"] = "result";
    root["status"] = "failed";
    root["data"] = "";
    root["errCode"] = code;
    root["errMsg"] = message;
    root["suggestion"] = suggestion;

    std::cout << root.dump() << std::endl;
    return 1;
}

int CommandStateIsScreenReaderEnabled(const std::vector<std::string>& args)
{
    auto client = AccessibilitySystemAbilityClient::GetInstance();
    bool enabled = false;
    RetError ret = client->IsScreenReaderEnabled(enabled);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"enabled\":" + std::string(enabled ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to check screen reader state. Error: " << ret << std::endl;
    return OutputError("ERR_CHECK_SCREEN_READER", "Failed to check screen reader state", \
        "Please check if accessibility service is running");
}

bool IsValidAbilityName(const std::string& name)
{
    size_t separator = name.find('/');
    if (separator == std::string::npos || separator == 0 || separator == name.size() - 1 ||
        name.find('/', separator + 1) != std::string::npos) {
        return false;
    }
    for (const char value : name) {
        if ((value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
            (value >= '0' && value <= '9') || value == '.' || value == '_' || value == '-' || value == '/') {
            continue;
        }
        return false;
    }
    return true;
}

int EnableAccessibilityAbility(const std::string& name, uint32_t capabilities)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    RetError ret = config.EnableAbility(name, capabilities);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"name\":\"" + name + "\",\"capabilities\":" + std::to_string(capabilities) + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to enable accessibility ability. Error: " << ret << std::endl;
    if (ret == RetError::RET_ERR_CONNECTION_EXIST) {
        return OutputError("ERR_ENABLE_ABILITY", "Accessibility ability is already enabled.", "Do not enable again.");
    }
    if (ret == RetError::RET_ERR_NO_PERMISSION || ret == RetError::RET_ERR_NOT_SYSTEM_APP) {
        return OutputError("ERR_ENABLE_ABILITY", "Failed to enable accessibility ability", \
            "Please check if you have sufficient permissions.");
    }
    return OutputError("ERR_ENABLE_ABILITY", "Failed to enable accessibility ability due to internal error.", \
        "Please try again later.");
}

int CommandAbilityEnable(const std::vector<std::string>& args)
{
    std::string name = GetOption(args, "--name");
    if (!IsValidAbilityName(name)) {
        return OutputError("ERR_ARG_INVALID", "Invalid or missing parameter: --name", \
            "Please provide bundleName/abilityName using --name");
    }

    uint32_t capabilities = 31;
    std::string capabilitiesStr = GetOption(args, "--capabilities");
    if (!capabilitiesStr.empty()) {
        int32_t value = 0;
        if (!ConvertStringToInt64(capabilitiesStr, value) || value < 1 || value > 31) {
            return OutputError("ERR_ARG_INVALID", "Invalid parameter value: --capabilities", \
                "Please provide a capability bit mask from 1 to 31");
        }
        capabilities = static_cast<uint32_t>(value);
    }
    if (!PrepareRootAccessibilityToken()) {
        return OutputError("ERR_TOKEN_INIT", "Failed to initialize the QEMU accessibility token", \
            "Run the command from a root HDC shell in an RD QEMU image");
    }
    return EnableAccessibilityAbility(name, capabilities);
}

int CommandAbilityEnableScreenReader(const std::vector<std::string>& args)
{
    return EnableAccessibilityAbility("com.ohos.screenreader/AccessibilityExtAbility", 31);
}

int DisableAccessibilityAbility(const std::string& name)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    RetError ret = config.DisableAbility(name);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"name\":\"" + name + "\"}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to disable accessibility ability. Error: " << ret << std::endl;
    if (ret == RetError::RET_ERR_NOT_ENABLED) {
        return OutputError("ERR_DISABLE_ABILITY", "Accessibility ability is not enabled.", "Do not disable again.");
    }
    if (ret == RetError::RET_ERR_NO_PERMISSION || ret == RetError::RET_ERR_NOT_SYSTEM_APP) {
        return OutputError("ERR_DISABLE_ABILITY", "Failed to disable accessibility ability", \
            "Please check if you have sufficient permissions.");
    }
    return OutputError("ERR_DISABLE_ABILITY", "Failed to disable accessibility ability due to internal error.", \
        "Please try again later.");
}

int CommandAbilityDisable(const std::vector<std::string>& args)
{
    std::string name = GetOption(args, "--name");
    if (!IsValidAbilityName(name)) {
        return OutputError("ERR_ARG_INVALID", "Invalid or missing parameter: --name", \
            "Please provide bundleName/abilityName using --name");
    }
    if (!PrepareRootAccessibilityToken()) {
        return OutputError("ERR_TOKEN_INIT", "Failed to initialize the QEMU accessibility token", \
            "Run the command from a root HDC shell in an RD QEMU image");
    }
    return DisableAccessibilityAbility(name);
}

int CommandAbilityDisableScreenReader(const std::vector<std::string>& args)
{
    return DisableAccessibilityAbility("com.ohos.screenreader/AccessibilityExtAbility");
}

int CommandMagnificationSetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set magnification state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set magnification state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetScreenMagnificationState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to set screen magnification state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_MAGNIFICATION", "Failed to set screen magnification state", \
        "Please check if accessibility service is running");
}

int CommandMagnificationGetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetScreenMagnificationState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get screen magnification state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_MAGNIFICATION", "Failed to get screen magnification state", \
        "Please check if accessibility service is running");
}

int CommandShortkeySetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set shortkey state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set shortkey state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetShortKeyState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to set shortkey state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_SHORTKEY", "Failed to set shortkey state", \
        "Please check if accessibility service is running");
}

int CommandShortkeyGetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetShortKeyState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get shortkey state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_SHORTKEY", "Failed to get shortkey state", \
        "Please check if accessibility service is running");
}

int CommandContrastSetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set high contrast state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set high contrast state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetHighContrastTextState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"state\":\"" + std::string(state ? "true" : "false") + "\"}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to set high contrast state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_HIGH_CONTRAST", "Failed to set high contrast state", \
        "Please check if accessibility service is running");
}

int CommandContrastGetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetHighContrastTextState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":\"" + std::string(state ? "true" : "false") + "\"}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get high contrast state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_HIGH_CONTRAST", "Failed to get high contrast state", \
        "Please check if accessibility service is running");
}

int CommandInvertSetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set invert color state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set invert color state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetInvertColorState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"state\":\"" + std::string(state ? "true" : "false") + "\"}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to set invert color state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_INVERT_COLOR", "Failed to set invert color state", \
        "Please check if accessibility service is running");
}

int CommandInvertGetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetInvertColorState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":\"" + std::string(state ? "true" : "false") + "\"}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get invert color state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_INVERT_COLOR", "Failed to get invert color state", \
        "Please check if accessibility service is running");
}

int CommandAnimationSetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set animation off state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set animation off state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetAnimationOffState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"state\":\"" + std::string(state ? "true" : "false") + "\"}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to set animation off state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_ANIMATION_OFF", "Failed to set animation off state", \
        "Please check if accessibility service is running");
}

int CommandAnimationGetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetAnimationOffState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":\"" + std::string(state ? "true" : "false") + "\"}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get animation off state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_ANIMATION_OFF", "Failed to get animation off state", \
        "Please check if accessibility service is running");
}

int CommandAudioSetMono(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set audio mono state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set audio mono state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetAudioMonoState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_HEARING", 1, 0);
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_HEARING", 0, 1);
    std::cout << "Failed to set audio mono state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_AUDIO_MONO", "Failed to set audio mono state", \
        "Please check if accessibility service is running");
}

int CommandAudioGetMono(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetAudioMonoState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get audio mono state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_AUDIO_MONO", "Failed to get audio mono state", \
        "Please check if accessibility service is running");
}

int CommandAudioSetBalance(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string balanceStr = GetOption(args, "--balance");
    if (balanceStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set audio balance failed: Missing required parameter. Parameter: --balance", \
            "Please provide balance value using --balance parameter");
    }

    float balance = 0.0f;
    if (!StringToFloat(balanceStr, balance) || balance < -1.0f || balance > 1.0f) {
        return OutputError("ERR_ARG_INVALID",
            "Set audio balance failed: Invalid parameter value. Parameter: --balance", \
            "Please provide valid balance value using --balance parameter (range: -1.0 to 1.0)");
    }

    RetError ret = config.SetAudioBalance(balance);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_HEARING", 1, 0);
        std::string data = "{\"balance\":" + balanceStr + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_HEARING", 0, 1);
    std::cout << "Failed to set audio balance. Error: " << ret << std::endl;
    return OutputError("ERR_SET_AUDIO_BALANCE", "Failed to set audio balance", \
        "Please check if the balance value is valid");
}

int CommandAudioGetBalance(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    float balance = 0.0f;
    RetError ret = config.GetAudioBalance(balance);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"balance\":" + std::to_string(balance) + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get audio balance. Error: " << ret << std::endl;
    return OutputError("ERR_GET_AUDIO_BALANCE", "Failed to get audio balance", \
        "Please check if accessibility service is running");
}

int CommandDaltonizationSetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set daltonization state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set daltonization state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetDaltonizationState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to set daltonization state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_DALTONIZATION","Failed to set daltonization state", \
        "Please check if accessibility service is running");
}

int CommandDaltonizationGetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetDaltonizationState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get daltonization state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_DALTONIZATION", "Failed to get daltonization state", \
        "Please check if accessibility service is running");
}

int CommandDaltonizationSetFilter(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string typeStr = GetOption(args, "--type");
    if (typeStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set daltonization filter failed: Missing required parameter. Parameter: --type", \
            "Please provide filter type using --type parameter");
    }

    int32_t type = 0;
    if (!ConvertStringToInt64(typeStr, type) ||
    type < static_cast<int>(AccessibilityConfig::DALTONIZATION_TYPE::Normal) ||
    type > static_cast<int>(AccessibilityConfig::DALTONIZATION_TYPE::Tritanomaly)) {
        return OutputError("ERR_ARG_INVALID",
            "Set daltonization filter failed: Invalid parameter value. Parameter: --type", \
            "Please provide valid filter type using --type parameter " \
            "(0=Normal, 1=Protanomaly, 2=Deuteranomaly, 3=Tritanomaly)");
    }

    AccessibilityConfig::DALTONIZATION_TYPE typeValue = static_cast<AccessibilityConfig::DALTONIZATION_TYPE>(type);
    RetError ret = config.SetDaltonizationColorFilter(typeValue);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 1, 0);
        std::string data = "{\"type\":" + typeStr + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_VISION", 0, 1);
    std::cout << "Failed to set daltonization filter type. Error: " << ret << std::endl;
    return OutputError("ERR_SET_DALTONIZATION_FILTER", "Failed to set daltonization filter type", \
        "Please check if the filter type is valid");
}

int CommandDaltonizationGetFilter(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    AccessibilityConfig::DALTONIZATION_TYPE type = AccessibilityConfig::DALTONIZATION_TYPE::Normal;
    RetError ret = config.GetDaltonizationColorFilter(type);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"type\":" + std::to_string(static_cast<int>(type)) + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get daltonization filter type. Error: " << ret << std::endl;
    return OutputError("ERR_GET_DALTONIZATION_FILTER", "Failed to get daltonization filter type", \
        "Please check if accessibility service is running");
}

int CommandClickSetResponseTime(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string timeStr = GetOption(args, "--time");
    if (timeStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set click response time failed: Missing required parameter. Parameter: --time", \
            "Please provide time value using --time parameter (0=short(default), 1=medium, 2=long)");
    }

    int32_t time = 0;
    if (!ConvertStringToInt64(timeStr, time) ||
    time < static_cast<int>(AccessibilityConfig::CLICK_RESPONSE_TIME::ResponseDelayShort) ||
    time > static_cast<int>(AccessibilityConfig::CLICK_RESPONSE_TIME::ResponseDelayLong)) {
        return OutputError("ERR_ARG_INVALID",
            "Set click response time failed: Invalid parameter value. Parameter: --time", \
            "Please provide valid time value using --time parameter (0=short(default), 1=medium, 2=long)");
    }

    AccessibilityConfig::CLICK_RESPONSE_TIME timeValue = static_cast<AccessibilityConfig::CLICK_RESPONSE_TIME>(time);
    RetError ret = config.SetClickResponseTime(timeValue);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_ACTION", 1, 0);
        std::string data = "{\"time\":" + timeStr + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_ACTION", 0, 1);
    std::cout << "Failed to set click response time. Error: " << ret << std::endl;
    return OutputError("ERR_SET_CLICK_RESPONSE", "Failed to set click response time", \
        "Please check if the time value is valid (0=short(default), 1=medium, 2=long)");
}

int CommandClickGetResponseTime(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    AccessibilityConfig::CLICK_RESPONSE_TIME time = AccessibilityConfig::CLICK_RESPONSE_TIME::ResponseDelayShort;
    RetError ret = config.GetClickResponseTime(time);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"time\":" + std::to_string(static_cast<int>(time)) + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get click response time. Error: " << ret << std::endl;
    return OutputError("ERR_GET_CLICK_RESPONSE", "Failed to get click response time", \
        "Please check if accessibility service is running");
}

int CommandRepeatClickSetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string stateStr = GetOption(args, "--state");
    if (stateStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set ignore repeat click state failed: Missing required parameter. Parameter: --state", \
            "Please provide state value using --state parameter (true/false)");
    }
    if (stateStr != "true" && stateStr != "false") {
        return OutputError("ERR_ARG_INVALID",
            "Set ignore repeat click state failed: Invalid parameter value. Parameter: --state", \
            "Please provide valid state value using --state parameter (true/false)");
    }

    bool state = (stateStr == "true");
    RetError ret = config.SetIgnoreRepeatClickState(state);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_ACTION", 1, 0);
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_ACTION", 0, 1);
    std::cout << "Failed to set ignore repeat click state. Error: " << ret << std::endl;
    return OutputError("ERR_SET_REPEAT_CLICK",
        "Failed to set ignore repeat click state", \
        "Please check if accessibility service is running");
}

int CommandRepeatClickGetState(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    bool state = false;
    RetError ret = config.GetIgnoreRepeatClickState(state);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"state\":" + std::string(state ? "true" : "false") + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get ignore repeat click state. Error: " << ret << std::endl;
    return OutputError("ERR_GET_REPEAT_CLICK", "Failed to get ignore repeat click state", \
        "Please check if accessibility service is running");
}

int CommandRepeatClickSetTime(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    std::string intervalStr = GetOption(args, "--interval");
    if (intervalStr.empty()) {
        return OutputError("ERR_ARG_MISSING",
            "Set ignore repeat click time failed: Missing required parameter. Parameter: --interval", \
            "Please provide interval value using --interval parameter " \
            "(0=0.1s, 1=0.4s, 2=0.7s, 3=1.0s, 4=1.3s)");
    }

    int32_t interval = 0;
    if (!ConvertStringToInt64(intervalStr, interval) ||
    interval < static_cast<int>(AccessibilityConfig::IGNORE_REPEAT_CLICK_TIME::RepeatClickTimeoutShortest) ||
    interval > static_cast<int>(AccessibilityConfig::IGNORE_REPEAT_CLICK_TIME::RepeatClickTimeoutLongest)) {
        return OutputError("ERR_ARG_INVALID",
            "Set ignore repeat click time failed: Invalid parameter value. Parameter: --interval", \
            "Please provide valid interval value using --interval parameter " \
            "(0=0.1s, 1=0.4s, 2=0.7s, 3=1.0s, 4=1.3s)");
    }

    AccessibilityConfig::IGNORE_REPEAT_CLICK_TIME time = \
        static_cast<AccessibilityConfig::IGNORE_REPEAT_CLICK_TIME>(interval);
    RetError ret = config.SetIgnoreRepeatClickTime(time);
    if (ret == RetError::RET_OK) {
        Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
            "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_ACTION", 1, 0);
        std::string data = "{\"interval\":" + intervalStr + "}";
        return OutputSuccess(data);
    }
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        "ohos.permission.cli.WRITE_ACCESSIBILITY_CONFIG_ACTION", 0, 1);
    std::cout << "Failed to set ignore repeat click time. Error: " << ret << std::endl;
    return OutputError("ERR_SET_REPEAT_CLICK_TIME", "Failed to set ignore repeat click time", \
        "Please check if the interval value is valid (0=0.1s, 1=0.4s, 2=0.7s, 3=1.0s, 4=1.3s)");
}

int CommandRepeatClickGetTime(const std::vector<std::string>& args)
{
    auto& config = AccessibilityConfig::AccessibilityConfig::GetInstance();
    AccessibilityConfig::IGNORE_REPEAT_CLICK_TIME time = \
        AccessibilityConfig::IGNORE_REPEAT_CLICK_TIME::RepeatClickTimeoutShortest;
    RetError ret = config.GetIgnoreRepeatClickTime(time);
    if (ret == RetError::RET_OK) {
        std::string data = "{\"interval\":" + std::to_string(static_cast<int>(time)) + "}";
        return OutputSuccess(data);
    }
    std::cout << "Failed to get ignore repeat click time. Error: " << ret << std::endl;
    return OutputError("ERR_GET_REPEAT_CLICK_TIME", "Failed to get ignore repeat click time", \
        "Please check if accessibility service is running");
}

typedef int (*CommandHandler)(const std::vector<std::string>&);

std::unordered_map<std::string, CommandHandler> g_commands = {
    {"state-is-screen-reader-enabled", CommandStateIsScreenReaderEnabled},
    {"ability-enable", CommandAbilityEnable},
    {"ability-disable", CommandAbilityDisable},
    {"ability-enable-screen-reader", CommandAbilityEnableScreenReader},
    {"ability-disable-screen-reader", CommandAbilityDisableScreenReader},
    {"magnification-set-state", CommandMagnificationSetState},
    {"magnification-get-state", CommandMagnificationGetState},
    {"shortkey-set-state", CommandShortkeySetState},
    {"shortkey-get-state", CommandShortkeyGetState},
    {"high-contrast-set-state", CommandContrastSetState},
    {"high-contrast-get-state", CommandContrastGetState},
    {"invert-color-set-state", CommandInvertSetState},
    {"invert-color-get-state", CommandInvertGetState},
    {"animation-off-set-state", CommandAnimationSetState},
    {"animation-off-get-state", CommandAnimationGetState},
    {"audio-set-mono", CommandAudioSetMono},
    {"audio-get-mono", CommandAudioGetMono},
    {"audio-set-balance", CommandAudioSetBalance},
    {"audio-get-balance", CommandAudioGetBalance},
    {"daltonization-set-state", CommandDaltonizationSetState},
    {"daltonization-get-state", CommandDaltonizationGetState},
    {"daltonization-set-filter", CommandDaltonizationSetFilter},
    {"daltonization-get-filter", CommandDaltonizationGetFilter},
    {"click-set-response-time", CommandClickSetResponseTime},
    {"click-get-response-time", CommandClickGetResponseTime},
    {"ignore-repeat-click-set-state", CommandRepeatClickSetState},
    {"ignore-repeat-click-get-state", CommandRepeatClickGetState},
    {"ignore-repeat-click-set-time", CommandRepeatClickSetTime},
    {"ignore-repeat-click-get-time", CommandRepeatClickGetTime},
};

int main(int argc, char* argv[])
{
    if (argc < ARGC_COUNT_TWO) {
        PrintHelp();
        return 1;
    }

    std::vector<std::string> args;
    for (int i = ARGC_COUNT_TWO; i < argc; i++) {
        args.push_back(argv[i]);
    }

    std::string command = argv[1];

    if (command == "--help" || command == "-h") {
        PrintHelp();
        return 0;
    }

    // Check for subcommand help
    if (HasOption(args, "--help") || HasOption(args, "-h")) {
        PrintHelp(command);
        return 0;
    }

    auto it = g_commands.find(command);
    if (it == g_commands.end()) {
        return OutputError("ERR_UNKNOWN_COMMAND", "Unknown command: " + command, \
            "Run 'ohos-a11yManager --help' for usage information");
    }

    return it->second(args);
}
// LCOV_EXCL_STOP