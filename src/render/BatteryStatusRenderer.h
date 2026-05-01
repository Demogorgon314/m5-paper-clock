#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "ConfigTypes.h"
#include "render/RenderPrimitives.h"

namespace render {

struct BatteryStatusRenderState {
    uint8_t last_battery_percentage = 255;
    bool last_wifi_connected = false;
    uint8_t last_wifi_signal_level = 255;
    ConfigConnectionIcon last_config_connection_icon =
        ConfigConnectionIcon::None;
    uint8_t partial_count = 0;
};

struct BatteryStatusRenderInput {
    uint8_t battery_percentage = 0;
    bool wifi_connected = false;
    uint8_t wifi_signal_level = 1;
    ConfigConnectionIcon config_connection_icon = ConfigConnectionIcon::None;
    bool full_refresh = false;
    bool forced_dirty = false;
    bool use_cjk_font = false;
    uint8_t partial_clean_interval = 1;
};

struct BatteryStatusRenderResult {
    bool rendered = false;
    bool partial_update = false;
    DirtyRect dirty;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

constexpr int16_t kBatteryStatusW = 176;
constexpr int16_t kBatteryStatusH = 44;

BatteryStatusRenderResult RenderBatteryStatus(
    M5EPD_Canvas& canvas,
    const BatteryStatusRenderInput& input,
    BatteryStatusRenderState& state);

}  // namespace render
