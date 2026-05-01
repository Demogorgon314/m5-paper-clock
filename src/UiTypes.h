#pragma once

#include <Arduino.h>

#include "logic/AppInputLogic.h"

static constexpr int kButtonWifi = logic::kButtonWifi;
static constexpr int kButtonSyncTime = logic::kButtonSyncTime;
static constexpr int kButtonEnterClock = logic::kButtonEnterClock;
static constexpr int kButtonTimezoneMinus = logic::kButtonTimezoneMinus;
static constexpr int kButtonTimezonePlus = logic::kButtonTimezonePlus;
static constexpr int kButtonBack = logic::kButtonBack;
static constexpr int kButtonRescan = logic::kButtonRescan;
static constexpr int kButtonPrevPage = logic::kButtonPrevPage;
static constexpr int kButtonNextPage = logic::kButtonNextPage;
static constexpr int kButtonBackspace = logic::kButtonBackspace;
static constexpr int kButtonSpace = logic::kButtonSpace;
static constexpr int kButtonClear = logic::kButtonClear;
static constexpr int kButtonPasswordVisibility = logic::kButtonPasswordVisibility;
static constexpr int kButtonConnect = logic::kButtonConnect;
static constexpr int kButtonKeyboardMode = logic::kButtonKeyboardMode;
static constexpr int kButtonShift = logic::kButtonShift;
static constexpr int kButtonNetworkBase = logic::kButtonNetworkBase;
static constexpr int kButtonKeyboardBase = logic::kButtonKeyboardBase;

struct Rect {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;

    Rect() = default;
    Rect(int16_t x_value, int16_t y_value, int16_t width_value,
         int16_t height_value)
        : x(x_value), y(y_value), w(width_value), h(height_value) {
    }

    bool contains(int16_t px, int16_t py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

struct UiButton {
    int id = 0;
    Rect bounds;
    String label;
    bool visible = true;
    bool enabled = true;
    bool primary = false;

    UiButton() = default;
    UiButton(int button_id, Rect button_bounds, const String& button_label,
           bool is_visible, bool is_enabled, bool is_primary)
        : id(button_id),
          bounds(button_bounds),
          label(button_label),
          visible(is_visible),
          enabled(is_enabled),
          primary(is_primary) {
    }
};
