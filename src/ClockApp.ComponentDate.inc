// Included by ClockApp.cpp. Adapts app state to render/DateStatusRenderer.
void ClockApp::updateDateCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::Date)) {
        return;
    }
    if (usesDashboardLayout() && full_refresh && fast_dashboard_entry_render_) {
        rtc_date_t current_date;
        M5.RTC.getDate(&current_date);
        const String date_text = DateOnly(current_date);
        const int16_t date_x =
            dashboardComponentX(logic::DashboardComponentId::Date);
        const int16_t date_y =
            dashboardComponentY(logic::DashboardComponentId::Date);
        render::RenderFastDateStatus(render_.date_canvas, date_text);
        render_.date_canvas.pushCanvas(date_x, date_y, UPDATE_MODE_NONE);
        render_cache_.date_status = render::DateStatusRenderState {};
        return;
    }

    rtc_date_t current_date;
    M5.RTC.getDate(&current_date);

    const String date_text =
        ConfiguredDate(current_date, settings_.date_format);
    const uint8_t week = CalculateWeekday(current_date);
    const logic::HolidayDisplay holiday_display = logic::HolidayDisplayForDate(
        current_date.year, current_date.mon, current_date.day);
    const String holiday_signature =
        settings_.show_holiday ? HolidayDisplaySignature(holiday_display)
                               : String("");
    const String pairing_signature =
        blePairingCodeActive() ? "ble:" + ble_pairing_code_ : "";
    const String holiday_pairing_signature =
        holiday_signature + "|" + pairing_signature + "|" +
        NormalizeDateFormat(settings_.date_format) + "|" +
        NormalizeWeekdayFormat(settings_.weekday_format) + "|" +
        NormalizeDateLayout(settings_.date_layout) + "|" +
        String(NormalizeDateTextSize(settings_.date_text_size));
    bool use_date_cjk = date_cjk_font_ready_;
    if (use_date_cjk &&
        !ensureCjkCanvas(render_.date_cjk_canvas, render::kDateStatusW,
                         render::kDateStatusH, {2, 3, 7})) {
        use_date_cjk = false;
        date_cjk_font_ready_ = false;
    }
    M5EPD_Canvas& active_date_canvas =
        use_date_cjk ? render_.date_cjk_canvas : render_.date_canvas;

    const String weekday_text = WeekdayLabel(week, settings_.weekday_format);
    const String date_layout = NormalizeDateLayout(settings_.date_layout);
    const uint8_t date_text_size =
        NormalizeDateTextSize(settings_.date_text_size);
    const bool show_holiday =
        settings_.show_holiday && date_layout != "date-only" &&
        holiday_display.valid();
    const String holiday_text =
        show_holiday ? HolidayDisplayText(
                           holiday_display,
                           holiday_display.state ==
                               logic::HolidayDisplayState::InHoliday)
                     : String("");
    const int16_t date_x =
        usesDashboardLayout()
            ? dashboardComponentX(logic::DashboardComponentId::Date)
            : kDateX;
    const int16_t date_y =
        usesDashboardLayout()
            ? dashboardComponentY(logic::DashboardComponentId::Date)
            : kDateY;

    render::DateStatusRenderInput input;
    input.date_text = date_text;
    input.weekday_text = weekday_text;
    input.holiday_text = holiday_text;
    input.render_signature = holiday_pairing_signature;
    input.pairing_code = blePairingCodeActive() ? ble_pairing_code_ : String("");
    input.date_layout = date_layout;
    input.weekday = week;
    input.date_text_size = date_text_size;
    input.show_holiday = show_holiday;
    input.use_cjk_font = use_date_cjk;
    input.full_refresh = full_refresh;
    input.partial_clean_interval = settings_.partial_clean_interval;

    const render::DateStatusRenderResult result =
        render::RenderDateStatus(active_date_canvas, input,
                                 render_cache_.date_status);
    if (!result.rendered) {
        return;
    }

    active_date_canvas.pushCanvas(date_x, date_y, UPDATE_MODE_NONE);

    if (result.partial_update) {
        updateAlignedArea(date_x + result.dirty.x, date_y + result.dirty.y,
                          result.dirty.w, result.dirty.h,
                          result.update_mode);
    }
    if (!full_refresh) {
        ++partial_refresh_count_;
    }
}
