#pragma once

#include <cstdint>

#include "logic/UiLogic.h"

namespace logic {

enum class AppPage : uint8_t { Settings, WifiScan, Password, Clock };

static constexpr int kButtonWifi = 1;
static constexpr int kButtonSyncTime = 2;
static constexpr int kButtonEnterClock = 3;
static constexpr int kButtonTimezoneMinus = 4;
static constexpr int kButtonTimezonePlus = 5;
static constexpr int kButtonBack = 6;
static constexpr int kButtonRescan = 7;
static constexpr int kButtonPrevPage = 8;
static constexpr int kButtonNextPage = 9;
static constexpr int kButtonBackspace = 10;
static constexpr int kButtonSpace = 11;
static constexpr int kButtonClear = 12;
static constexpr int kButtonPasswordVisibility = 13;
static constexpr int kButtonConnect = 14;
static constexpr int kButtonKeyboardMode = 15;
static constexpr int kButtonShift = 16;
static constexpr int kButtonNetworkBase = 100;
static constexpr int kButtonKeyboardBase = 200;

enum class ButtonAction : uint8_t {
    None,
    OpenWifiScan,
    DecreaseTimezone,
    IncreaseTimezone,
    SyncTime,
    EnterClock,
    BackToSettings,
    RescanWifi,
    PreviousWifiPage,
    NextWifiPage,
    SelectWifiNetwork,
    BackToWifiScan,
    ToggleKeyboardShift,
    ToggleKeyboardMode,
    BackspacePassword,
    AppendPasswordSpace,
    ClearPassword,
    TogglePasswordVisibility,
    ConnectSelectedNetwork,
    AppendKeyboardCharacter,
};

struct ButtonActionDecision {
    ButtonAction action = ButtonAction::None;
    int index = -1;

    constexpr ButtonActionDecision() noexcept = default;
    constexpr ButtonActionDecision(ButtonAction next_action,
                                   int next_index) noexcept
        : action(next_action), index(next_index) {
    }
};

inline ButtonActionDecision DecideButtonAction(AppPage page,
                                               int button_id,
                                               int wifi_page_index,
                                               int wifi_network_count,
                                               int wifi_page_size) {
    switch (page) {
        case AppPage::Settings:
            switch (button_id) {
                case kButtonWifi:
                    return {ButtonAction::OpenWifiScan, -1};
                case kButtonTimezoneMinus:
                    return {ButtonAction::DecreaseTimezone, -1};
                case kButtonTimezonePlus:
                    return {ButtonAction::IncreaseTimezone, -1};
                case kButtonSyncTime:
                    return {ButtonAction::SyncTime, -1};
                case kButtonEnterClock:
                    return {ButtonAction::EnterClock, -1};
                default:
                    return {};
            }

        case AppPage::WifiScan:
            if (button_id == kButtonBack) {
                return {ButtonAction::BackToSettings, -1};
            }
            if (button_id == kButtonRescan) {
                return {ButtonAction::RescanWifi, -1};
            }
            if (button_id == kButtonPrevPage && wifi_page_index > 0) {
                return {ButtonAction::PreviousWifiPage, -1};
            }
            if (button_id == kButtonNextPage &&
                wifi_page_index + 1 <
                    PageCount(wifi_network_count, wifi_page_size)) {
                return {ButtonAction::NextWifiPage, -1};
            }
            if (button_id >= kButtonNetworkBase &&
                button_id < kButtonNetworkBase + wifi_page_size) {
                const int index = PageStart(wifi_page_index, wifi_page_size) +
                                  (button_id - kButtonNetworkBase);
                if (index >= 0 && index < wifi_network_count) {
                    return {ButtonAction::SelectWifiNetwork, index};
                }
            }
            return {};

        case AppPage::Password:
            if (button_id == kButtonBack) {
                return {ButtonAction::BackToWifiScan, -1};
            }
            if (button_id == kButtonShift) {
                return {ButtonAction::ToggleKeyboardShift, -1};
            }
            if (button_id == kButtonKeyboardMode) {
                return {ButtonAction::ToggleKeyboardMode, -1};
            }
            if (button_id == kButtonBackspace) {
                return {ButtonAction::BackspacePassword, -1};
            }
            if (button_id == kButtonSpace) {
                return {ButtonAction::AppendPasswordSpace, -1};
            }
            if (button_id == kButtonClear) {
                return {ButtonAction::ClearPassword, -1};
            }
            if (button_id == kButtonPasswordVisibility) {
                return {ButtonAction::TogglePasswordVisibility, -1};
            }
            if (button_id == kButtonConnect) {
                return {ButtonAction::ConnectSelectedNetwork, -1};
            }
            if (button_id >= kButtonKeyboardBase &&
                button_id < kButtonKeyboardBase + 26) {
                return {ButtonAction::AppendKeyboardCharacter,
                        button_id - kButtonKeyboardBase};
            }
            return {};

        case AppPage::Clock:
        default:
            return {};
    }
}

}  // namespace logic
