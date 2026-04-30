// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateDashboardClimateCanvas(bool full_refresh) {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Climate)) {
        return;
    }

    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = formatDashboardHumidity(last_environment_);
    const String temperature_text = formatDashboardTemperature(last_environment_);
    const String comfort_face =
        comfortFace(last_environment_, settings_.comfort_settings);
    const bool humidity_changed =
        humidity_text != last_dashboard_humidity_text_rendered_;
    const bool temperature_changed =
        temperature_text != last_dashboard_temperature_text_rendered_;
    const bool comfort_changed =
        comfort_face != last_dashboard_comfort_face_rendered_;

    if (!full_refresh && !humidity_changed && !temperature_changed &&
        !comfort_changed) {
        return;
    }

    dashboard_climate_canvas_.fillCanvas(kWhite);
    dashboard_climate_canvas_.drawRoundRect(0, 0, kDashboardClimateW,
                                            kDashboardClimateH, 6, kBorder);
    dashboard_climate_canvas_.drawFastVLine(kDashboardClimateHumidityDividerX,
                                            18, 50, kBorder);
    dashboard_climate_canvas_.drawFastVLine(kDashboardClimateTemperatureDividerX,
                                            18, 50, kBorder);

    const int16_t climate_value_y = kDashboardClimateH / 2 + 2;
    const int16_t humidity_x = 18;
    const int16_t temperature_x = 134;

    dashboard_climate_canvas_.setTextDatum(CL_DATUM);
    dashboard_climate_canvas_.setTextColor(kText);
    setCanvasTextSize(dashboard_climate_canvas_, cjk_font_ready_, 5);
    dashboard_climate_canvas_.drawString(humidity_text, humidity_x,
                                         climate_value_y);
    const int16_t humidity_width = dashboard_climate_canvas_.textWidth(humidity_text);
    setCanvasTextSize(dashboard_climate_canvas_, cjk_font_ready_, 2);
    dashboard_climate_canvas_.drawString("%", humidity_x + humidity_width + 6,
                                         climate_value_y + 14);

    setCanvasTextSize(dashboard_climate_canvas_, cjk_font_ready_, 5);
    dashboard_climate_canvas_.drawString(temperature_text, temperature_x,
                                         climate_value_y);
    const int16_t temperature_width =
        dashboard_climate_canvas_.textWidth(temperature_text);
    const int16_t unit_x = temperature_x + temperature_width + 4;
    setCanvasTextSize(dashboard_climate_canvas_, cjk_font_ready_, 2);
    dashboard_climate_canvas_.drawString("o", unit_x, climate_value_y - 8);
    dashboard_climate_canvas_.drawString("C", unit_x + 14,
                                         climate_value_y + 14);

    const int16_t face_cell_center_x =
        kDashboardClimateTemperatureDividerX +
        ((kDashboardClimateW - kDashboardClimateTemperatureDividerX) / 2);
    drawCompactComfortInfoAt(dashboard_climate_canvas_, face_cell_center_x,
                             kDashboardClimateH / 2 + 2, comfort_face, kText,
                             cjk_font_ready_);

    const int16_t climate_x =
        dashboardComponentX(logic::DashboardComponentId::Climate);
    const int16_t climate_y =
        dashboardComponentY(logic::DashboardComponentId::Climate);
    dashboard_climate_canvas_.pushCanvas(climate_x, climate_y,
                                         UPDATE_MODE_NONE);
    if (full_refresh) {
        dashboard_climate_partial_count_ = 0;
    } else {
        int16_t dirty_left = kDashboardClimateW;
        int16_t dirty_right = 0;
        const auto includeDirtyArea = [&](int16_t x, int16_t w) {
            dirty_left = min<int16_t>(dirty_left, x);
            dirty_right = max<int16_t>(dirty_right, static_cast<int16_t>(x + w));
        };
        if (humidity_changed) {
            includeDirtyArea(kDashboardClimateHumidityAreaX,
                             kDashboardClimateHumidityAreaW);
        }
        if (temperature_changed) {
            includeDirtyArea(kDashboardClimateTemperatureAreaX,
                             kDashboardClimateTemperatureAreaW);
        }
        if (comfort_changed) {
            includeDirtyArea(kDashboardClimateComfortAreaX,
                             kDashboardClimateComfortAreaW);
        }

        const m5epd_update_mode_t mode =
            nextPartialMode(dashboard_climate_partial_count_,
                            settings_.partial_clean_interval);
        updateAlignedArea(climate_x + dirty_left,
                          climate_y + kDashboardClimateContentY,
                          dirty_right - dirty_left,
                          kDashboardClimateContentH, mode);
        ++partial_refresh_count_;
    }
    last_dashboard_humidity_text_rendered_ = humidity_text;
    last_dashboard_temperature_text_rendered_ = temperature_text;
    last_dashboard_comfort_face_rendered_ = comfort_face;
}

