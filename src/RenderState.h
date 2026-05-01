#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "ConfigTypes.h"
#include "render/BatteryStatusRenderer.h"
#include "render/ClassicInfoRenderer.h"
#include "render/ClassicTimeRenderer.h"
#include "render/DashboardCalendarRenderer.h"
#include "render/DashboardClimateRenderer.h"
#include "render/DashboardMarketRenderer.h"
#include "render/DashboardTimeRenderer.h"
#include "render/DateStatusRenderer.h"

struct RenderResources {
    M5EPD_Canvas page_canvas {&M5.EPD};
    M5EPD_Canvas time_canvas {&M5.EPD};
    M5EPD_Canvas minute_canvas {&M5.EPD};
    M5EPD_Canvas info_canvas {&M5.EPD};
    M5EPD_Canvas humidity_canvas {&M5.EPD};
    M5EPD_Canvas temperature_canvas {&M5.EPD};
    M5EPD_Canvas comfort_canvas {&M5.EPD};
    M5EPD_Canvas date_canvas {&M5.EPD};
    M5EPD_Canvas date_cjk_canvas {&M5.EPD};
    M5EPD_Canvas battery_canvas {&M5.EPD};
    M5EPD_Canvas dashboard_calendar_canvas {&M5.EPD};
    M5EPD_Canvas dashboard_time_canvas {&M5.EPD};
    M5EPD_Canvas dashboard_minute_canvas {&M5.EPD};
    M5EPD_Canvas dashboard_summary_canvas {&M5.EPD};
    M5EPD_Canvas dashboard_summary_cjk_canvas {&M5.EPD};
    M5EPD_Canvas dashboard_climate_canvas {&M5.EPD};
    M5EPD_Canvas password_field_canvas {&M5.EPD};
    M5EPD_Canvas password_status_canvas {&M5.EPD};
};

struct RenderCache {
    render::ClassicTimeRenderState classic_time;
    render::ClassicInfoRenderState classic_info;
    render::DashboardTimeRenderState dashboard_time;
    render::DashboardClimateRenderState dashboard_climate;
    render::DateStatusRenderState date_status;
    uint8_t stable_battery_percentage = 255;
    uint8_t battery_candidate_percentage = 255;
    uint8_t battery_candidate_count = 0;
    render::BatteryStatusRenderState battery_status;
    render::DashboardCalendarRenderState dashboard_calendar;
};
