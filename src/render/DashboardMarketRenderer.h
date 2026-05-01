#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "MarketService.h"
#include "render/RenderPrimitives.h"

namespace render {

struct DashboardMarketRenderState {
    String last_signature;
    String last_title;
    String last_price;
    String last_bottom;
    uint8_t partial_count = 0;
    bool last_valid = false;
};

struct DashboardMarketRenderInput {
    MarketQuote quote;
    bool wifi_connected = false;
    bool market_open = false;
    bool full_refresh = false;
    bool fast_ascii_only = false;
    bool use_cjk_font = false;
    uint8_t partial_clean_interval = 1;
};

struct DashboardMarketRenderResult {
    bool rendered = false;
    bool partial_update = false;
    DirtyRect dirty;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

constexpr int16_t kDashboardMarketStatusW = 332;
constexpr int16_t kDashboardMarketStatusH = 86;

DashboardMarketRenderResult RenderDashboardMarket(
    M5EPD_Canvas& canvas,
    const DashboardMarketRenderInput& input,
    DashboardMarketRenderState& state);

String DashboardMarketSignature(const MarketQuote& quote,
                                bool wifi_connected,
                                bool market_open);

}  // namespace render
