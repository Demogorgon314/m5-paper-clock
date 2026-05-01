#include "PageButtonFactory.h"

#include "logic/UiLogic.h"

namespace pagebuttons {

void BuildSettingsButtons(std::vector<UiButton>& buttons) {
    buttons.push_back(UiButton(kButtonWifi, Rect(24, 378, 276, 72),
                               "Choose Wi-Fi", true, true, false));
    buttons.push_back(UiButton(kButtonTimezoneMinus, Rect(482, 378, 72, 72),
                               "-", true, true, false));
    buttons.push_back(UiButton(kButtonTimezonePlus, Rect(610, 378, 72, 72),
                               "+", true, true, false));
    buttons.push_back(UiButton(kButtonSyncTime, Rect(706, 378, 230, 72),
                               "Sync Time", true, true, false));
    buttons.push_back(UiButton(kButtonEnterClock, Rect(24, 460, 912, 56),
                               "Enter Clock", true, true, true));
}

void BuildWifiButtons(std::vector<UiButton>& buttons, int network_count,
                      int page_index, int page_size) {
    const int visible_count =
        logic::VisibleItemCount(network_count, page_index, page_size);
    for (int index = 0; index < visible_count; ++index) {
        buttons.push_back(UiButton(
            kButtonNetworkBase + index,
            Rect(24, static_cast<int16_t>(118 + index * 56), 912, 48), "",
            true, true, false));
    }

    buttons.push_back(UiButton(kButtonBack, Rect(24, 470, 140, 44), "Back",
                               true, true, false));
    buttons.push_back(UiButton(kButtonPrevPage, Rect(590, 470, 100, 44),
                               "Prev", true, page_index > 0, false));
    buttons.push_back(UiButton(kButtonRescan, Rect(704, 470, 110, 44),
                               "Rescan", true, true, false));
    buttons.push_back(UiButton(
        kButtonNextPage, Rect(828, 470, 108, 44), "Next", true,
        page_index + 1 < logic::PageCount(network_count, page_size), false));
}

void BuildPasswordButtons(std::vector<UiButton>& buttons,
                          const std::vector<String>& keys,
                          bool password_visible, bool keyboard_symbols) {
    buttons.push_back(UiButton(kButtonBack, Rect(24, 18, 100, 32), "Back",
                               true, true, false));

    const int16_t key_w = 70;
    const int16_t key_h = 46;
    const int16_t gap = 8;

    for (size_t i = 0; i < 10; ++i) {
        buttons.push_back(UiButton(
            kButtonKeyboardBase + static_cast<int>(i),
            Rect(94 + static_cast<int16_t>(i) * (key_w + gap), 246, key_w,
                 key_h),
            keys[i], true, true, false));
    }
    for (size_t i = 0; i < 9; ++i) {
        buttons.push_back(UiButton(
            kButtonKeyboardBase + 10 + static_cast<int>(i),
            Rect(133 + static_cast<int16_t>(i) * (key_w + gap), 300, key_w,
                 key_h),
            keys[10 + i], true, true, false));
    }

    buttons.push_back(UiButton(kButtonShift, Rect(108, 354, 96, key_h),
                               "Shift", true, true, false));
    for (size_t i = 0; i < 7; ++i) {
        buttons.push_back(UiButton(
            kButtonKeyboardBase + 19 + static_cast<int>(i),
            Rect(220 + static_cast<int16_t>(i) * (key_w + gap), 354, key_w,
                 key_h),
            keys[19 + i], true, true, false));
    }
    buttons.push_back(UiButton(kButtonBackspace, Rect(794, 354, 96, key_h),
                               "Back", true, true, false));

    buttons.push_back(UiButton(kButtonKeyboardMode, Rect(108, 408, 96, key_h),
                               keyboard_symbols ? "Abc" : "123", true, true,
                               false));
    buttons.push_back(UiButton(kButtonSpace, Rect(220, 408, 252, key_h),
                               "Space", true, true, false));
    buttons.push_back(UiButton(kButtonClear, Rect(488, 408, 112, key_h),
                               "Clear", true, true, false));
    buttons.push_back(UiButton(kButtonPasswordVisibility,
                               Rect(616, 408, 112, key_h),
                               password_visible ? "Hide" : "Show", true, true,
                               false));
    buttons.push_back(UiButton(kButtonConnect, Rect(744, 408, 162, key_h),
                               "Connect", true, true, true));
}

}  // namespace pagebuttons
