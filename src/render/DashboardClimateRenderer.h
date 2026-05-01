#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "render/RenderPrimitives.h"

namespace render {

struct DashboardClimateRenderState {
    String last_humidity_text;
    String last_temperature_text;
    String last_comfort_face;
    uint8_t partial_count = 0;
};

struct DashboardClimateRenderInput {
    String humidity_text;
    String temperature_text;
    String comfort_face;
    bool full_refresh = false;
    bool use_cjk_font = false;
    uint8_t partial_clean_interval = 1;
};

struct DashboardClimateRenderResult {
    bool rendered = false;
    bool partial_update = false;
    DirtyRect dirty;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

constexpr int16_t kDashboardClimateStatusW = 456;
constexpr int16_t kDashboardClimateStatusH = 86;

DashboardClimateRenderResult RenderDashboardClimate(
    M5EPD_Canvas& canvas,
    const DashboardClimateRenderInput& input,
    DashboardClimateRenderState& state);

}  // namespace render
