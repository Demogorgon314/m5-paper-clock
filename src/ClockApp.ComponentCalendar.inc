// Included by ClockApp.cpp. Adapts app state to render/DashboardCalendarRenderer.
void ClockApp::updateDashboardCalendarCanvas(bool full_refresh) {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Calendar)) {
        return;
    }

    rtc_date_t current_date;
    M5.RTC.getDate(&current_date);
    const bool fast_ascii_only = full_refresh && fast_dashboard_entry_render_;

    render::DashboardCalendarRenderInput input;
    input.current_date = current_date;
    input.previous_date = last_date_;
    input.full_refresh = full_refresh;
    input.fast_ascii_only = fast_ascii_only;
    input.use_cjk_font = cjk_font_ready_;
    input.use_weekday_cjk_font = dashboard_calendar_cjk_font_ready_;
    input.partial_clean_interval = settings_.partial_clean_interval;

    const render::DashboardCalendarRenderResult result =
        render::RenderDashboardCalendar(render_.dashboard_calendar_canvas, input,
                                        render_cache_.dashboard_calendar);

    const int16_t calendar_x =
        dashboardComponentX(logic::DashboardComponentId::Calendar);
    const int16_t calendar_y =
        dashboardComponentY(logic::DashboardComponentId::Calendar);
    render_.dashboard_calendar_canvas.pushCanvas(calendar_x, calendar_y,
                                          UPDATE_MODE_NONE);
    if (result.partial_update) {
        updateAlignedArea(calendar_x + result.dirty.x,
                          calendar_y + result.dirty.y, result.dirty.w,
                          result.dirty.h, result.update_mode);
        ++partial_refresh_count_;
    }
}
