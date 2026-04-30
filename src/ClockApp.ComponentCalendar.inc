// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateDashboardCalendarCanvas(bool full_refresh) {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Calendar)) {
        return;
    }

    rtc_date_t current_date;
    M5.RTC.getDate(&current_date);
    const bool fast_ascii_only = full_refresh && fast_dashboard_entry_render_;

    dashboard_calendar_canvas_.fillCanvas(kWhite);
    dashboard_calendar_canvas_.setTextColor(kText);
    dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
    setCanvasTextSize(dashboard_calendar_canvas_, cjk_font_ready_, 5);
    dashboard_calendar_canvas_.drawString(formatDashboardMonth(current_date),
                                          kDashboardCalendarW / 2, 24);

    for (int col = 0; col < 7; ++col) {
        const int16_t center_x =
            kDashboardCalendarStartX + col * kDashboardCalendarCellW +
            (kDashboardCalendarCellW - 2) / 2;
        if (dashboard_calendar_cjk_font_ready_ && !fast_ascii_only) {
            dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
            dashboard_calendar_canvas_.setTextColor(kText);
            setCanvasTextSize(dashboard_calendar_canvas_, true, 2);
            dashboard_calendar_canvas_.drawString(
                calendarWeekdayLabel(static_cast<uint8_t>(col)),
                center_x, kDashboardCalendarHeaderY + 8);
        } else {
            static constexpr const char* kFallbackWeekdayLabels[] = {"S", "M", "T",
                                                                     "W", "T", "F",
                                                                     "S"};
            dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
            dashboard_calendar_canvas_.setTextColor(kText);
            setCanvasTextSize(dashboard_calendar_canvas_, false, 2);
            dashboard_calendar_canvas_.drawString(
                kFallbackWeekdayLabels[col], center_x,
                kDashboardCalendarHeaderY + 8);
        }
    }

    rtc_date_t first_day = current_date;
    first_day.day = 1;
    const uint8_t first_weekday = calculateWeekday(first_day);
    const uint8_t month_days = daysInMonth(current_date.year, current_date.mon);

    setCanvasTextSize(dashboard_calendar_canvas_, cjk_font_ready_, 2);
    dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            const int day = row * 7 + col - first_weekday + 1;
            if (day < 1 || day > month_days) {
                continue;
            }

            const int16_t x =
                kDashboardCalendarStartX + col * kDashboardCalendarCellW;
            const int16_t y =
                kDashboardCalendarGridY + row * kDashboardCalendarCellH;
            const bool is_today = day == current_date.day;
            const bool is_past = day < current_date.day;

            if (is_today) {
                dashboard_calendar_canvas_.fillRect(
                    x, y, kDashboardCalendarCellW - 2,
                    kDashboardCalendarCellH - 2, kCalendarTodayFill);
            } else if (is_past) {
                dashboard_calendar_canvas_.fillRect(
                    x, y, kDashboardCalendarCellW - 2,
                    kDashboardCalendarCellH - 2, kCalendarPastFill);
            }
            dashboard_calendar_canvas_.drawRect(x, y,
                                                kDashboardCalendarCellW - 2,
                                                kDashboardCalendarCellH - 2,
                                                kBorder);
            dashboard_calendar_canvas_.setTextColor(is_today ? kWhite : kText);
            dashboard_calendar_canvas_.drawString(
                String(day), x + (kDashboardCalendarCellW - 2) / 2,
                y + (kDashboardCalendarCellH - 2) / 2);
        }
    }

    const int16_t calendar_x =
        dashboardComponentX(logic::DashboardComponentId::Calendar);
    const int16_t calendar_y =
        dashboardComponentY(logic::DashboardComponentId::Calendar);
    dashboard_calendar_canvas_.pushCanvas(calendar_x, calendar_y,
                                          UPDATE_MODE_NONE);
    if (full_refresh) {
        dashboard_calendar_partial_count_ = 0;
    } else {
        const bool can_refresh_changed_cells =
            last_date_.year == current_date.year &&
            last_date_.mon == current_date.mon &&
            last_date_.day >= 1 &&
            last_date_.day != current_date.day;
        const m5epd_update_mode_t mode =
            nextPartialMode(dashboard_calendar_partial_count_,
                            settings_.partial_clean_interval);
        if (can_refresh_changed_cells) {
            DirtyRect dirty;
            includeDirtyRect(dirty, dashboardCalendarCellRect(last_date_));
            includeDirtyRect(dirty, dashboardCalendarCellRect(current_date));
            if (dirty.valid) {
                updateAlignedArea(calendar_x + dirty.x,
                                  calendar_y + dirty.y, dirty.w,
                                  dirty.h, mode);
            }
        } else {
            updateAlignedArea(calendar_x, calendar_y, kDashboardCalendarW,
                              kDashboardCalendarH, mode);
        }
        ++partial_refresh_count_;
    }
}

