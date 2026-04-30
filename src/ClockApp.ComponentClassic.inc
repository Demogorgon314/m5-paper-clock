// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateTimeCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::ClassicTime)) {
        return;
    }

    rtc_time_t current_time;
    M5.RTC.getTime(&current_time);

    const String time_digits = formatTimeDigits(current_time);
    if (!full_refresh && time_digits == last_dashboard_time_text_rendered_) {
        return;
    }

    const bool can_refresh_minutes_only =
        !full_refresh && last_dashboard_time_text_rendered_.length() == 4 &&
        time_digits.substring(0, 2) ==
            last_dashboard_time_text_rendered_.substring(0, 2);

    const int16_t time_x = usesDashboardLayout()
        ? dashboardComponentX(logic::DashboardComponentId::ClassicTime)
        : kTimeX;
    const int16_t time_y = usesDashboardLayout()
        ? dashboardComponentY(logic::DashboardComponentId::ClassicTime)
        : kTimeY;

    if (can_refresh_minutes_only) {
        const int16_t minute_canvas_x = alignDownTo4(time_x + kTimeColonX);
        const int16_t minute_canvas_y = time_y;
        const int16_t minute_draw_x =
            (time_x + kTimeColonX) - minute_canvas_x;
        drawMinuteTime(renderer_, minute_canvas_, time_digits, minute_draw_x,
                       kText, kTimeEdgeText);
        const m5epd_update_mode_t mode =
            nextPartialMode(classic_time_partial_count_,
                            settings_.partial_clean_interval);
        pushAndRefreshCanvas(minute_canvas_, minute_canvas_x,
                             minute_canvas_y, mode);
        ++partial_refresh_count_;
        last_dashboard_time_text_rendered_ = time_digits;
        return;
    }

    const String time_text =
        time_digits.substring(0, 2) + ":" + time_digits.substring(2);
    time_canvas_.fillCanvas(kWhite);
    renderer_.drawText(time_canvas_, kTimeDigitXs[0], kTimeDigitY, time_text,
                       kTimeDigitWidth, kTimeDigitHeight, kTimeGap, kText,
                       kTimeEdgeText);
    time_canvas_.pushCanvas(time_x, time_y, UPDATE_MODE_NONE);

    if (full_refresh) {
        partial_refresh_count_ = 0;
        time_digit_partial_counts_.fill(0);
        classic_time_partial_count_ = 0;
    } else {
        const m5epd_update_mode_t mode =
            nextPartialMode(classic_time_partial_count_,
                            settings_.partial_clean_interval);
        updateAlignedArea(time_x, time_y, time_canvas_.width(),
                          time_canvas_.height(), mode);
        ++partial_refresh_count_;
    }
    last_dashboard_time_text_rendered_ = time_digits;
}

void ClockApp::updateInfoCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::ClassicInfo)) {
        return;
    }

    const int16_t info_x = usesDashboardLayout()
        ? dashboardComponentX(logic::DashboardComponentId::ClassicInfo)
        : kInfoX;
    const int16_t info_y = usesDashboardLayout()
        ? dashboardComponentY(logic::DashboardComponentId::ClassicInfo)
        : kInfoY;

    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = formatHumidityField(last_environment_);
    const String temperature_text = formatTemperatureField(last_environment_);
    const String comfort_face =
        comfortFace(last_environment_, settings_.comfort_settings);

    if (full_refresh) {
        info_canvas_.fillCanvas(kWhite);
        info_canvas_.setTextDatum(TL_DATUM);
        renderer_.drawText(info_canvas_, kHumidityDigitXs[0], kInfoDigitY,
                           humidity_text, kSmallDigitWidth, kSmallDigitHeight,
                           kSmallGap, kText, kMutedText);
        info_canvas_.setTextColor(kText);
        setCanvasTextSize(info_canvas_, cjk_font_ready_, 3);
        info_canvas_.drawString("%", kHumidityUnitX, kHumidityUnitY);

        const String temperature_display = temperature_text.substring(0, 2) +
                                           "." + temperature_text.substring(2);
        renderer_.drawText(info_canvas_, kTemperatureDigitXs[0], kInfoDigitY,
                           temperature_display, kSmallDigitWidth,
                           kSmallDigitHeight, kSmallGap, kText, kMutedText);
        info_canvas_.setTextColor(kText);
        info_canvas_.drawString(" C", kTemperatureUnitX, kTemperatureUnitY);
        setCanvasTextSize(info_canvas_, cjk_font_ready_, 2);
        info_canvas_.drawString("o", kTemperatureDegreeX, kTemperatureDegreeY);

        info_canvas_.setTextDatum(CC_DATUM);
        info_canvas_.setTextColor(kText);
        drawComfortInfoAt(info_canvas_, 640, 58, comfort_face, kText,
                          cjk_font_ready_);

        info_canvas_.pushCanvas(info_x, info_y, UPDATE_MODE_NONE);
        partial_refresh_count_ = 0;
        last_humidity_text_rendered_ = humidity_text;
        last_temperature_text_rendered_ = temperature_text;
        last_comfort_face_rendered_ = comfort_face;
        humidity_digit_partial_counts_.fill(0);
        temperature_digit_partial_counts_.fill(0);
        classic_humidity_partial_count_ = 0;
        classic_temperature_partial_count_ = 0;
        classic_comfort_partial_count_ = 0;
        return;
    }

    bool humidity_changed = humidity_text != last_humidity_text_rendered_;
    bool temperature_changed = temperature_text != last_temperature_text_rendered_;
    bool comfort_changed = comfort_face != last_comfort_face_rendered_;

    if (humidity_changed) {
        const int16_t humidity_canvas_x =
            alignDownTo4(info_x + kHumidityDigitXs[0]);
        const int16_t humidity_canvas_y = info_y;
        drawHumidityInfo(renderer_, humidity_canvas_, humidity_text, kText,
                         kMutedText, cjk_font_ready_);
        humidity_canvas_.pushCanvas(humidity_canvas_x, humidity_canvas_y,
                                    UPDATE_MODE_NONE);
        const m5epd_update_mode_t mode =
            nextPartialMode(classic_humidity_partial_count_,
                            settings_.partial_clean_interval);
        updateAlignedArea(humidity_canvas_x, humidity_canvas_y,
                          humidity_canvas_.width(), humidity_canvas_.height(),
                          mode);
        ++partial_refresh_count_;
        humidity_digit_partial_counts_.fill(0);
    }

    if (temperature_changed) {
        const int16_t temperature_canvas_x =
            alignDownTo4(info_x + kTemperatureDigitXs[0]);
        const int16_t temperature_canvas_y = info_y;
        drawTemperatureInfo(renderer_, temperature_canvas_, temperature_text, kText,
                            kMutedText, cjk_font_ready_);
        temperature_canvas_.pushCanvas(temperature_canvas_x, temperature_canvas_y,
                                       UPDATE_MODE_NONE);
        const m5epd_update_mode_t mode =
            nextPartialMode(classic_temperature_partial_count_,
                            settings_.partial_clean_interval);
        updateAlignedArea(temperature_canvas_x, temperature_canvas_y,
                          temperature_canvas_.width(),
                          temperature_canvas_.height(), mode);
        ++partial_refresh_count_;
        temperature_digit_partial_counts_.fill(0);
    }
    if (comfort_changed) {
        const int16_t comfort_canvas_x = alignDownTo4(info_x + 520);
        const int16_t comfort_canvas_y = info_y;
        drawComfortInfo(comfort_canvas_, comfort_face, kText, cjk_font_ready_);
        comfort_canvas_.pushCanvas(comfort_canvas_x, comfort_canvas_y,
                                   UPDATE_MODE_NONE);
        const m5epd_update_mode_t mode =
            nextPartialMode(classic_comfort_partial_count_,
                            settings_.partial_clean_interval);
        updateAlignedArea(comfort_canvas_x, comfort_canvas_y,
                          comfort_canvas_.width(), comfort_canvas_.height(),
                          mode);
        ++partial_refresh_count_;
    }
    last_humidity_text_rendered_ = humidity_text;
    last_temperature_text_rendered_ = temperature_text;
    last_comfort_face_rendered_ = comfort_face;
}

