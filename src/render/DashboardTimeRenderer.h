#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "SegmentRenderer.h"
#include "render/RenderPrimitives.h"

namespace render {

enum class DashboardTimeRenderTarget : uint8_t {
    None,
    Full,
    Minute,
};

struct DashboardTimeRenderState {
    String last_time_digits;
    uint8_t partial_count = 0;
};

struct DashboardTimeRenderInput {
    String time_digits;
    int16_t x = 0;
    int16_t y = 0;
    bool full_refresh = false;
    uint8_t partial_clean_interval = 1;
};

struct DashboardTimeRenderResult {
    bool rendered = false;
    DashboardTimeRenderTarget target = DashboardTimeRenderTarget::None;
    int16_t push_x = 0;
    int16_t push_y = 0;
    bool partial_update = false;
    DirtyRect dirty;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

constexpr int16_t kDashboardTimeStatusW = 472;
constexpr int16_t kDashboardTimeStatusH = 236;
constexpr int16_t kDashboardMinuteStatusW = 256;
constexpr int16_t kDashboardMinuteStatusH = kDashboardTimeStatusH;

DashboardTimeRenderResult RenderDashboardTime(
    const SegmentRenderer& renderer,
    M5EPD_Canvas& time_canvas,
    M5EPD_Canvas& minute_canvas,
    const DashboardTimeRenderInput& input,
    DashboardTimeRenderState& state);

}  // namespace render
