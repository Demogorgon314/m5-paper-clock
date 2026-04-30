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
    const String summary_signature =
        marketSummarySignature(state.quote, wifi_connected);
    if (!full_refresh && summary_signature == state.last_signature) {
        return;
    }

    const bool fast_ascii_only = full_refresh && fast_dashboard_entry_render_;
    bool use_summary_cjk = dashboard_summary_cjk_font_ready_ && !fast_ascii_only;
    if (use_summary_cjk &&
        !ensureCjkCanvas(dashboard_summary_cjk_canvas_, kDashboardSummaryW,
                         kDashboardSummaryH, {2, 3, 7})) {
        use_summary_cjk = false;
        dashboard_summary_cjk_font_ready_ = false;
    }
    M5EPD_Canvas& active_summary_canvas =
        use_summary_cjk ? dashboard_summary_cjk_canvas_
                        : dashboard_summary_canvas_;

    active_summary_canvas.fillCanvas(kWhite);
    active_summary_canvas.drawRoundRect(0, 0, kDashboardSummaryW,
                                        kDashboardSummaryH, 6, kBorder);
    active_summary_canvas.setTextDatum(TL_DATUM);
    active_summary_canvas.setTextColor(kText);
    setCanvasTextSize(active_summary_canvas, use_summary_cjk, 2);
    const String market_title =
        marketDisplayLabel(state.quote, use_summary_cjk);
    const bool summary_valid = state.quote.valid;
    String summary_price;
    String summary_bottom;
    String change_value;
    String percent_value;
    String status_label;
    String status_detail;
    if (summary_valid) {
        summary_price = state.quote.price;
        const String change_prefix = state.quote.positive ? "+" : "";
        change_value = change_prefix + state.quote.change;
        percent_value = change_prefix + state.quote.change_percent + "%";
        status_label =
            marketStatusLabel(state.quote, market_open, use_summary_cjk);
        summary_bottom = change_value + "|" + percent_value + "|" +
                         status_label + "|" +
                         (state.quote.positive ? "1" : "0");
    } else {
        status_detail =
            !state.quote.error_message.isEmpty()
                ? state.quote.error_message
                : (wifi_connected ? "Data unavailable" : "Wi-Fi offline");
        summary_bottom = status_detail;
    }
    if (use_summary_cjk) {
        drawFauxBoldString(active_summary_canvas, market_title,
                           kDashboardSummaryTitleX, kDashboardSummaryTitleY);
    } else {
        active_summary_canvas.drawString(market_title,
                                         kDashboardSummaryTitleX,
                                         kDashboardSummaryTitleY);
    }

    if (summary_valid) {
        active_summary_canvas.setTextDatum(TR_DATUM);
        setCanvasTextSize(active_summary_canvas, false, 4);
        active_summary_canvas.setTextColor(kText);
        active_summary_canvas.drawString(summary_price,
                                         kDashboardSummaryPriceRight,
                                         kDashboardSummaryPriceY);

        setCanvasTextSize(active_summary_canvas, false, 2);
        active_summary_canvas.setTextColor(kText);
        active_summary_canvas.setTextDatum(TR_DATUM);
        const int16_t percent_right = kDashboardSummaryPriceRight;
        const int16_t percent_width =
            active_summary_canvas.textWidth(percent_value);
        const int16_t change_right =
            percent_right - percent_width - kDashboardSummaryValueGap;
        const int16_t change_width =
            active_summary_canvas.textWidth(change_value);
        const int16_t change_left = change_right - change_width;
        const int16_t arrow_center_x = change_left - kDashboardSummaryArrowGap;
        const int16_t arrow_center_y = kDashboardSummaryBottomY + 5;
        if (state.quote.positive) {
            active_summary_canvas.fillTriangle(arrow_center_x - 8,
                                               arrow_center_y + 6,
                                               arrow_center_x,
                                               arrow_center_y - 6,
                                               arrow_center_x + 8,
                                               arrow_center_y + 6, kText);
        } else {
            active_summary_canvas.fillTriangle(arrow_center_x - 8,
                                               arrow_center_y - 6,
                                               arrow_center_x,
                                               arrow_center_y + 6,
                                               arrow_center_x + 8,
                                               arrow_center_y - 6, kText);
        }
        active_summary_canvas.setTextDatum(TL_DATUM);
        active_summary_canvas.setTextColor(kMutedText);
        setCanvasTextSize(active_summary_canvas, use_summary_cjk, 2);
        active_summary_canvas.drawString(status_label, 16, 58);
        active_summary_canvas.setTextColor(kText);
        setCanvasTextSize(active_summary_canvas, false, 2);
        active_summary_canvas.setTextDatum(TR_DATUM);
        active_summary_canvas.drawString(change_value, change_right,
                                         kDashboardSummaryBottomY);
        active_summary_canvas.drawString(percent_value, percent_right,
                                         kDashboardSummaryBottomY);
    } else {
        active_summary_canvas.setTextDatum(TL_DATUM);
        setCanvasTextSize(active_summary_canvas, false, 3);
        active_summary_canvas.setTextColor(kText);
        active_summary_canvas.drawString("No quote", 16, 34);
        setCanvasTextSize(active_summary_canvas, false, 2);
        active_summary_canvas.setTextColor(kMutedText);
        active_summary_canvas.drawString(status_detail, 16, 62);
    }

    const int16_t summary_x = item.x;
    const int16_t summary_y = item.y;
    active_summary_canvas.pushCanvas(summary_x, summary_y, UPDATE_MODE_NONE);
    if (full_refresh) {
        state.partial_count = 0;
    } else {
        DirtyRect dirty;
        if (summary_valid != state.last_valid) {
            dirty.x = 0;
            dirty.y = 0;
            dirty.w = kDashboardSummaryW;
            dirty.h = kDashboardSummaryH;
            dirty.valid = true;
        } else {
            if (market_title != state.last_title) {
                DirtyRect rect;
                rect.x = kDashboardSummaryTitleAreaX;
                rect.y = kDashboardSummaryTitleAreaY;
                rect.w = kDashboardSummaryTitleAreaW;
                rect.h = kDashboardSummaryTitleAreaH;
                rect.valid = true;
                includeDirtyRect(dirty, rect);
            }
            if (summary_price != state.last_price) {
                DirtyRect rect;
                rect.x = kDashboardSummaryPriceAreaX;
                rect.y = kDashboardSummaryPriceAreaY;
                rect.w = kDashboardSummaryPriceAreaW;
                rect.h = kDashboardSummaryPriceAreaH;
                rect.valid = true;
                includeDirtyRect(dirty, rect);
            }
            if (summary_bottom != state.last_bottom) {
                DirtyRect rect;
                rect.x = kDashboardSummaryBottomAreaX;
                rect.y = kDashboardSummaryBottomAreaY;
                rect.w = kDashboardSummaryBottomAreaW;
                rect.h = kDashboardSummaryBottomAreaH;
                rect.valid = true;
                includeDirtyRect(dirty, rect);
            }
        }
        const m5epd_update_mode_t mode =
            nextPartialMode(state.partial_count,
                            settings_.partial_clean_interval);
        if (dirty.valid) {
            updateAlignedArea(summary_x + dirty.x, summary_y + dirty.y,
                              dirty.w, dirty.h, mode);
        }
        ++partial_refresh_count_;
    }
    state.last_signature = fast_ascii_only ? String("") : summary_signature;
    state.last_title = fast_ascii_only ? String("") : market_title;
    state.last_price = fast_ascii_only ? String("") : summary_price;
    state.last_bottom = fast_ascii_only ? String("") : summary_bottom;
    state.last_valid = !fast_ascii_only && summary_valid;

    if (item.instance_id == "market-1") {
        market_quote_ = state.quote;
        last_market_fetch_ms_ = state.last_fetch_ms;
        last_market_summary_rendered_ = state.last_signature;
        last_dashboard_summary_title_rendered_ = state.last_title;
        last_dashboard_summary_price_rendered_ = state.last_price;
        last_dashboard_summary_bottom_rendered_ = state.last_bottom;
        dashboard_summary_partial_count_ = state.partial_count;
        last_dashboard_summary_valid_ = state.last_valid;
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

