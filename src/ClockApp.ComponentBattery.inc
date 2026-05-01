// Included by ClockApp.cpp. Adapts app state to render/BatteryStatusRenderer.
void ClockApp::updateBatteryCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::Battery)) {
        return;
    }

    const uint8_t battery = batteryPercentage();
    const bool wifi_connected = connectivity_.isConnected();
    const uint8_t wifi_signal_level = wifi_connected
                                          ? WifiBitmapLevel(
                                                WifiSignalLevelFromRssi(
                                                    connectivity_.currentRssi()))
                                          : 1;
    const ConfigConnectionIcon config_connection_icon =
        activeConfigConnectionIcon();
    const bool status_forced_dirty = battery_status_dirty_;
    const int16_t battery_x =
        usesDashboardLayout()
            ? dashboardComponentX(logic::DashboardComponentId::Battery)
            : kBatteryX;
    const int16_t battery_y =
        usesDashboardLayout()
            ? dashboardComponentY(logic::DashboardComponentId::Battery)
            : kBatteryY;

    render::BatteryStatusRenderInput input;
    input.battery_percentage = battery;
    input.wifi_connected = wifi_connected;
    input.wifi_signal_level = wifi_signal_level;
    input.config_connection_icon = config_connection_icon;
    input.full_refresh = full_refresh;
    input.forced_dirty = status_forced_dirty;
    input.use_cjk_font = cjk_font_ready_;
    input.partial_clean_interval = settings_.partial_clean_interval;

    const render::BatteryStatusRenderResult result =
        render::RenderBatteryStatus(render_.battery_canvas, input,
                                    render_cache_.battery_status);
    battery_status_dirty_ = false;
    if (!result.rendered) {
        return;
    }

    render_.battery_canvas.pushCanvas(battery_x, battery_y, UPDATE_MODE_NONE);

    if (result.partial_update) {
        updateAlignedArea(battery_x + result.dirty.x,
                          battery_y + result.dirty.y, result.dirty.w,
                          result.dirty.h, result.update_mode);
    }
}
