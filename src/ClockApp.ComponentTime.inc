// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateDashboardTimeCanvas(bool full_refresh) {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Time)) {
        return;
    }

    rtc_time_t current_time;
    M5.RTC.getTime(&current_time);

    const String time_digits = formatTimeDigits(current_time);
    if (!full_refresh && time_digits == last_time_text_rendered_) {
        return;
    }

    const bool can_refresh_minutes_only =
        !full_refresh && last_time_text_rendered_.length() == 4 &&
        time_digits.substring(0, 2) == last_time_text_rendered_.substring(0, 2);

    const int16_t time_x = dashboardComponentX(logic::DashboardComponentId::Time);
    const int16_t time_y = dashboardComponentY(logic::DashboardComponentId::Time);
    if (can_refresh_minutes_only) {
        const int16_t minute_x = alignDownTo4(time_x + kDashboardMinuteLocalX);
        const int16_t minute_y = time_y;
        const int16_t minute_draw_x =
            (time_x + kDashboardMinuteLocalX) - minute_x;
        drawDashboardMinuteTime(renderer_, dashboard_minute_canvas_, time_digits,
                                minute_draw_x, kText, kTimeEdgeText);
        const m5epd_update_mode_t mode =
            nextPartialMode(dashboard_time_partial_count_,
                            settings_.partial_clean_interval);
        pushAndRefreshCanvas(dashboard_minute_canvas_, minute_x, minute_y, mode);
        ++partial_refresh_count_;
        last_time_text_rendered_ = time_digits;
        return;
    }

    const String time_text =
        time_digits.substring(0, 2) + ":" + time_digits.substring(2);

    dashboard_time_canvas_.fillCanvas(kWhite);
    renderer_.drawText(dashboard_time_canvas_, kDashboardTimeDrawX,
                       kDashboardTimeDigitY, time_text,
                       kDashboardTimeDigitW, kDashboardTimeDigitH,
                       kDashboardTimeGap, kText, kTimeEdgeText);
    dashboard_time_canvas_.pushCanvas(time_x, time_y, UPDATE_MODE_NONE);
    if (full_refresh) {
        dashboard_time_partial_count_ = 0;
    } else {
        const m5epd_update_mode_t mode =
            nextPartialMode(dashboard_time_partial_count_,
                            settings_.partial_clean_interval);
        updateAlignedArea(time_x, time_y, kDashboardTimeW, kDashboardTimeH, mode);
        ++partial_refresh_count_;
    }
    last_time_text_rendered_ = time_digits;
}

