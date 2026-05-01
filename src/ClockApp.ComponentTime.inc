// Included by ClockApp.cpp. Adapts app state to render/DashboardTimeRenderer.
void ClockApp::updateDashboardTimeCanvas(bool full_refresh) {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Time)) {
        return;
    }

    rtc_time_t current_time;
    M5.RTC.getTime(&current_time);

    const String time_digits = TimeDigits(current_time);
    const int16_t time_x = dashboardComponentX(logic::DashboardComponentId::Time);
    const int16_t time_y = dashboardComponentY(logic::DashboardComponentId::Time);

    render::DashboardTimeRenderInput input;
    input.time_digits = time_digits;
    input.x = time_x;
    input.y = time_y;
    input.full_refresh = full_refresh;
    input.partial_clean_interval = settings_.partial_clean_interval;

    const render::DashboardTimeRenderResult result =
        render::RenderDashboardTime(renderer_, render_.dashboard_time_canvas,
                                    render_.dashboard_minute_canvas, input,
                                    render_cache_.dashboard_time);
    if (!result.rendered) {
        return;
    }

    if (result.target == render::DashboardTimeRenderTarget::Minute) {
        pushAndRefreshCanvas(render_.dashboard_minute_canvas, result.push_x,
                             result.push_y, result.update_mode);
        ++partial_refresh_count_;
        return;
    }

    render_.dashboard_time_canvas.pushCanvas(result.push_x, result.push_y,
                                             UPDATE_MODE_NONE);
    if (result.partial_update) {
        updateAlignedArea(result.push_x + result.dirty.x,
                          result.push_y + result.dirty.y, result.dirty.w,
                          result.dirty.h, result.update_mode);
        ++partial_refresh_count_;
    }
}
