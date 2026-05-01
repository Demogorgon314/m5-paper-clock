// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateDashboardSummaryCanvas(bool full_refresh,
                                            bool allow_fetch) {
    syncMarketRenderStates();
    for (const MarketLayoutItem& item : settings_.market_layout) {
        if (!item.visible) {
            continue;
        }
        MarketRenderState& state = marketRenderStateFor(item);
        updateDashboardMarketInstance(item, state, full_refresh, allow_fetch);
    }
}

void ClockApp::updateDashboardMarketInstance(const MarketLayoutItem& item,
                                             MarketRenderState& state,
                                             bool full_refresh,
                                             bool allow_fetch) {
    if (allow_fetch) {
        refreshMarketQuote(state, full_refresh);
        if (state.last_fetch_ms != 0 &&
            (last_market_fetch_ms_ == 0 ||
             state.last_fetch_ms > last_market_fetch_ms_)) {
            last_market_fetch_ms_ = state.last_fetch_ms;
        }
    }
    const bool wifi_connected = connectivity_.isConnected();
    rtc_date_t current_date;
    rtc_time_t current_time;
    M5.RTC.getDate(&current_date);
    M5.RTC.getTime(&current_time);
    const bool market_open = logic::IsCnAShareMarketOpen(
        current_date.year, current_date.mon, current_date.day, current_time.hour,
        current_time.min);
    const bool market_refresh_due =
        state.last_fetch_ms == 0 ||
        millis() - state.last_fetch_ms > kDashboardMarketIntervalMs;
    if (!allow_fetch && wifi_connected && market_refresh_due) {
        if (!market_open && state.quote.valid) {
            state.last_fetch_ms = millis();
            if (last_market_fetch_ms_ == 0 ||
                state.last_fetch_ms > last_market_fetch_ms_) {
                last_market_fetch_ms_ = state.last_fetch_ms;
            }
        } else {
            pending_market_refresh_ = true;
        }
    }
    const bool fast_ascii_only = full_refresh && fast_dashboard_entry_render_;
    bool use_summary_cjk = dashboard_summary_cjk_font_ready_ && !fast_ascii_only;
    if (use_summary_cjk &&
        !ensureCjkCanvas(render_.dashboard_summary_cjk_canvas,
                         render::kDashboardMarketStatusW,
                         render::kDashboardMarketStatusH, {2, 3, 7})) {
        use_summary_cjk = false;
        dashboard_summary_cjk_font_ready_ = false;
    }
    M5EPD_Canvas& active_summary_canvas =
        use_summary_cjk ? render_.dashboard_summary_cjk_canvas
                        : render_.dashboard_summary_canvas;

    render::DashboardMarketRenderInput input;
    input.quote = state.quote;
    input.wifi_connected = wifi_connected;
    input.market_open = market_open;
    input.full_refresh = full_refresh;
    input.fast_ascii_only = fast_ascii_only;
    input.use_cjk_font = use_summary_cjk;
    input.partial_clean_interval = settings_.partial_clean_interval;
    const render::DashboardMarketRenderResult result =
        render::RenderDashboardMarket(active_summary_canvas, input,
                                      state.render_state);
    if (!result.rendered) {
        return;
    }

    const int16_t summary_x = item.x;
    const int16_t summary_y = item.y;
    active_summary_canvas.pushCanvas(summary_x, summary_y, UPDATE_MODE_NONE);
    if (result.partial_update) {
        updateAlignedArea(summary_x + result.dirty.x,
                          summary_y + result.dirty.y, result.dirty.w,
                          result.dirty.h, result.update_mode);
        ++partial_refresh_count_;
    }

    if (item.instance_id == "market-1") {
        market_quote_ = state.quote;
        last_market_fetch_ms_ = state.last_fetch_ms;
    }
}


bool ClockApp::refreshMarketQuote(bool force) {
    syncMarketRenderStates();
    bool any_valid = false;
    uint32_t latest_fetch_ms = 0;
    for (const MarketLayoutItem& item : settings_.market_layout) {
        if (!item.visible) {
            continue;
        }
        MarketRenderState& state = marketRenderStateFor(item);
        any_valid = refreshMarketQuote(state, force) || any_valid;
        if (state.last_fetch_ms > latest_fetch_ms) {
            latest_fetch_ms = state.last_fetch_ms;
        }
    }
    if (!market_render_states_.empty()) {
        for (const MarketLayoutItem& item : settings_.market_layout) {
            if (!item.visible) {
                continue;
            }
            MarketRenderState& state = marketRenderStateFor(item);
            market_quote_ = state.quote;
            break;
        }
    }
    if (latest_fetch_ms != 0) {
        last_market_fetch_ms_ = latest_fetch_ms;
    }
    return any_valid;
}

bool ClockApp::refreshMarketQuote(MarketRenderState& state, bool force) {
    if (!force && millis() - state.last_fetch_ms < kDashboardMarketIntervalMs) {
        return state.quote.valid;
    }

    auto mark_checked = [&]() {
        state.last_fetch_ms = millis();
        if (last_market_fetch_ms_ == 0 ||
            state.last_fetch_ms > last_market_fetch_ms_) {
            last_market_fetch_ms_ = state.last_fetch_ms;
        }
    };

    if (!connectivity_.isConnected()) {
        mark_checked();
        return state.quote.valid;
    }

    rtc_date_t current_date;
    rtc_time_t current_time;
    M5.RTC.getDate(&current_date);
    M5.RTC.getTime(&current_time);

    const bool market_open = logic::IsCnAShareMarketOpen(
        current_date.year, current_date.mon, current_date.day, current_time.hour,
        current_time.min);
    if (!force && !market_open && state.quote.valid) {
        mark_checked();
        return true;
    }

    mark_checked();

    const MarketQuote fetched = market_.fetchQuote(state.symbol);
    if (!fetched.valid) {
        if (!state.quote.valid) {
            seedMarketQuoteFromSettings(state, state.symbol);
            state.quote.error_message = fetched.error_message;
        }
        return state.quote.valid;
    }

    state.quote = fetched;
    return true;
}
