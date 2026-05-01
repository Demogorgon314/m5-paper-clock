// Included by ClockApp.cpp. Adapts app state to render/DashboardClimateRenderer.
void ClockApp::updateDashboardClimateCanvas(bool full_refresh) {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Climate)) {
        return;
    }

    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = DashboardHumidity(last_environment_);
    const String temperature_text = DashboardTemperature(last_environment_);
    const String comfort_face =
        ComfortFace(last_environment_, settings_.comfort_settings);

    render::DashboardClimateRenderInput input;
    input.humidity_text = humidity_text;
    input.temperature_text = temperature_text;
    input.comfort_face = comfort_face;
    input.full_refresh = full_refresh;
    input.use_cjk_font = cjk_font_ready_;
    input.partial_clean_interval = settings_.partial_clean_interval;

    const render::DashboardClimateRenderResult result =
        render::RenderDashboardClimate(render_.dashboard_climate_canvas, input,
                                       render_cache_.dashboard_climate);
    if (!result.rendered) {
        return;
    }

    const int16_t climate_x =
        dashboardComponentX(logic::DashboardComponentId::Climate);
    const int16_t climate_y =
        dashboardComponentY(logic::DashboardComponentId::Climate);
    render_.dashboard_climate_canvas.pushCanvas(climate_x, climate_y,
                                         UPDATE_MODE_NONE);
    if (result.partial_update) {
        updateAlignedArea(climate_x + result.dirty.x,
                          climate_y + result.dirty.y, result.dirty.w,
                          result.dirty.h, result.update_mode);
        ++partial_refresh_count_;
    }
}
