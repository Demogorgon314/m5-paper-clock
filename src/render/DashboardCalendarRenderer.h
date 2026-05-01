#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "render/RenderPrimitives.h"

namespace render {

struct DashboardCalendarRenderState {
    uint8_t partial_count = 0;
};

struct DashboardCalendarRenderInput {
    rtc_date_t current_date {};
    rtc_date_t previous_date {};
    bool full_refresh = false;
    bool fast_ascii_only = false;
    bool use_cjk_font = false;
    bool use_weekday_cjk_font = false;
    uint8_t partial_clean_interval = 1;
};

struct DashboardCalendarRenderResult {
    bool rendered = false;
    bool partial_update = false;
    DirtyRect dirty;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

constexpr int16_t kDashboardCalendarStatusW = 304;
constexpr int16_t kDashboardCalendarStatusH = 232;

DashboardCalendarRenderResult RenderDashboardCalendar(
    M5EPD_Canvas& canvas,
    const DashboardCalendarRenderInput& input,
    DashboardCalendarRenderState& state);

}  // namespace render
