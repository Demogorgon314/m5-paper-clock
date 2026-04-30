// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateBatteryCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::Battery)) {
        return;
    }

    const uint8_t battery = batteryPercentage();
    const bool wifi_connected = connectivity_.isConnected();
    const uint8_t wifi_signal_level = wifi_connected
                                          ? wifiBitmapLevel(
                                                wifiSignalLevelFromRssi(WiFi.RSSI()))
                                          : 1;
    const ConfigConnectionIcon config_connection_icon =
        activeConfigConnectionIcon();
    const bool status_forced_dirty = battery_status_dirty_;
    const bool battery_changed = battery != last_battery_percentage_;
    const bool wifi_changed = wifi_connected != last_wifi_connected_ ||
                              wifi_signal_level != last_wifi_signal_level_;
    const bool transport_changed =
        config_connection_icon != last_config_connection_icon_;
    if (!full_refresh && !battery_changed && !wifi_changed &&
        !transport_changed) {
        battery_status_dirty_ = false;
        return;
    }
    battery_status_dirty_ = false;
    char battery_label[8];
    snprintf(battery_label, sizeof(battery_label), "%3u%%", battery);

    const int16_t body_w = 38;
    const int16_t body_h = 20;
    const int16_t cap_w = 4;
    const int16_t cap_h = 10;
    const int16_t icon_right = kBatteryW - 2;
    const int16_t cap_x = icon_right - cap_w;
    const int16_t body_x = cap_x - body_w;
    const int16_t body_y = 12;
    const int16_t cap_y = body_y + 5;
    const int16_t inner_x = body_x + 3;
    const int16_t inner_y = body_y + 3;
    const int16_t inner_w = body_w - 6;
    const int16_t inner_h = body_h - 6;
    const int16_t fill_w = (battery * inner_w) / 100;
    const int16_t label_right = body_x - 10;

    battery_canvas_.fillCanvas(kWhite);
    battery_canvas_.setTextColor(kText);
    setCanvasTextSize(battery_canvas_, cjk_font_ready_, 2);
    battery_canvas_.setTextDatum(CR_DATUM);
    const int16_t label_width = battery_canvas_.textWidth(String(battery_label));
    const int16_t wifi_gap = 8;
    const int16_t wifi_size = 32;
    const int16_t wifi_x = label_right - label_width - wifi_gap - wifi_size;
    const int16_t wifi_y = 6;
    const int16_t transport_gap = 10;
    const int16_t transport_size = 18;
    const int16_t transport_x = wifi_x - transport_gap - transport_size;
    const int16_t transport_y = 10;
    battery_canvas_.drawString(String(battery_label), label_right,
                               kBatteryH / 2);
    if (config_connection_icon == ConfigConnectionIcon::Serial) {
        drawUsbStatusIcon(battery_canvas_, transport_x, transport_y);
    } else if (config_connection_icon == ConfigConnectionIcon::Bluetooth) {
        drawBluetoothStatusIcon(battery_canvas_, transport_x, transport_y);
    }
    drawWifiStatusIcon(battery_canvas_, wifi_x, wifi_y, wifi_connected,
                       wifi_signal_level);
    battery_canvas_.drawRoundRect(body_x, body_y, body_w, body_h, 3, kText);
    battery_canvas_.fillRect(cap_x, cap_y, cap_w, cap_h, kText);
    if (fill_w > 0) {
        battery_canvas_.fillRect(inner_x, inner_y, fill_w, inner_h, kText);
    }
    const int16_t battery_x =
        usesDashboardLayout()
            ? dashboardComponentX(logic::DashboardComponentId::Battery)
            : kBatteryX;
    const int16_t battery_y =
        usesDashboardLayout()
            ? dashboardComponentY(logic::DashboardComponentId::Battery)
            : kBatteryY;
    battery_canvas_.pushCanvas(battery_x, battery_y, UPDATE_MODE_NONE);

    if (full_refresh) {
        battery_partial_count_ = 0;
    } else {
        DirtyRect dirty;
        if (battery_changed) {
            DirtyRect rect;
            rect.x = max<int16_t>(0, label_right - 72);
            rect.y = 0;
            rect.w = kBatteryW - rect.x;
            rect.h = kBatteryH;
            rect.valid = true;
            includeDirtyRect(dirty, rect);
        }
        if (wifi_changed) {
            DirtyRect rect;
            rect.x = max<int16_t>(0, wifi_x - 4);
            rect.y = 0;
            rect.w = min<int16_t>(kBatteryW - rect.x, wifi_size + 8);
            rect.h = kBatteryH;
            rect.valid = true;
            includeDirtyRect(dirty, rect);
        }
        if (transport_changed) {
            DirtyRect rect;
            rect.x = max<int16_t>(0, transport_x - 4);
            rect.y = 0;
            rect.w = min<int16_t>(kBatteryW - rect.x, transport_size + 8);
            rect.h = kBatteryH;
            rect.valid = true;
            includeDirtyRect(dirty, rect);
        }
        if (status_forced_dirty && !dirty.valid) {
            dirty.x = 0;
            dirty.y = 0;
            dirty.w = kBatteryW;
            dirty.h = kBatteryH;
            dirty.valid = true;
        }
        const m5epd_update_mode_t mode =
            nextPartialMode(battery_partial_count_,
                            settings_.partial_clean_interval);
        if (dirty.valid) {
            updateAlignedArea(battery_x + dirty.x, battery_y + dirty.y,
                              dirty.w, dirty.h, mode);
            ++battery_partial_count_;
        }
    }
    last_battery_percentage_ = battery;
    last_wifi_connected_ = wifi_connected;
    last_wifi_signal_level_ = wifi_signal_level;
    last_config_connection_icon_ = config_connection_icon;
}

