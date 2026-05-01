// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateTimeCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::ClassicTime)) {
        return;
    }

    rtc_time_t current_time;
    M5.RTC.getTime(&current_time);

    const String time_digits = TimeDigits(current_time);
    const int16_t time_x = usesDashboardLayout()
        ? dashboardComponentX(logic::DashboardComponentId::ClassicTime)
        : render::kClassicTimeDefaultX;
    const int16_t time_y = usesDashboardLayout()
        ? dashboardComponentY(logic::DashboardComponentId::ClassicTime)
        : render::kClassicTimeDefaultY;

    render::ClassicTimeRenderInput input;
    input.time_digits = time_digits;
    input.x = time_x;
    input.y = time_y;
    input.full_refresh = full_refresh;
    input.partial_clean_interval = settings_.partial_clean_interval;
    const render::ClassicTimeRenderResult result =
        render::RenderClassicTime(renderer_, render_.time_canvas,
                                  render_.minute_canvas, input,
                                  render_cache_.classic_time);
    if (!result.rendered) {
        return;
    }

    M5EPD_Canvas& active_canvas =
        result.target == render::ClassicTimeRenderTarget::Minute
            ? render_.minute_canvas
            : render_.time_canvas;
    active_canvas.pushCanvas(result.push_x, result.push_y, UPDATE_MODE_NONE);
    if (full_refresh) {
        partial_refresh_count_ = 0;
    } else if (result.partial_update) {
        updateAlignedArea(result.push_x + result.dirty.x,
                          result.push_y + result.dirty.y, result.dirty.w,
                          result.dirty.h, result.update_mode);
        ++partial_refresh_count_;
    }
}

void ClockApp::updateInfoCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::ClassicInfo)) {
        return;
    }

    const int16_t info_x = usesDashboardLayout()
        ? dashboardComponentX(logic::DashboardComponentId::ClassicInfo)
        : render::kClassicInfoDefaultX;
    const int16_t info_y = usesDashboardLayout()
        ? dashboardComponentY(logic::DashboardComponentId::ClassicInfo)
        : render::kClassicInfoDefaultY;

    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = ClassicHumidityField(last_environment_);
    const String temperature_text = ClassicTemperatureField(last_environment_);
    const String comfort_face =
        ComfortFace(last_environment_, settings_.comfort_settings);

    render::ClassicInfoRenderInput input;
    input.humidity_text = humidity_text;
    input.temperature_text = temperature_text;
    input.comfort_face = comfort_face;
    input.x = info_x;
    input.y = info_y;
    input.full_refresh = full_refresh;
    input.use_cjk_font = cjk_font_ready_;
    input.partial_clean_interval = settings_.partial_clean_interval;
    const render::ClassicInfoRenderResult result =
        render::RenderClassicInfo(renderer_, render_.info_canvas,
                                  render_.humidity_canvas,
                                  render_.temperature_canvas,
                                  render_.comfort_canvas, input,
                                  render_cache_.classic_info);
    if (!result.rendered) {
        return;
    }

    if (result.full_render) {
        render_.info_canvas.pushCanvas(info_x, info_y, UPDATE_MODE_NONE);
        partial_refresh_count_ = 0;
        return;
    }

    for (uint8_t i = 0; i < result.partial_update_count; ++i) {
        const render::ClassicInfoPartialUpdate& update =
            result.partial_updates[i];
        M5EPD_Canvas* canvas = nullptr;
        if (update.target == render::ClassicInfoRenderTarget::Humidity) {
            canvas = &render_.humidity_canvas;
        } else if (update.target ==
                   render::ClassicInfoRenderTarget::Temperature) {
            canvas = &render_.temperature_canvas;
        } else {
            canvas = &render_.comfort_canvas;
        }
        canvas->pushCanvas(update.push_x, update.push_y, UPDATE_MODE_NONE);
        updateAlignedArea(update.push_x, update.push_y, update.width,
                          update.height, update.update_mode);
        ++partial_refresh_count_;
    }
}
