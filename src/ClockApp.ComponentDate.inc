// Included by ClockApp.cpp so component rendering can share the
// current canvas/font helpers while ClockApp is being modularized.
void ClockApp::updateDateCanvas(bool full_refresh) {
    if (usesDashboardLayout() &&
        !dashboardComponentVisible(logic::DashboardComponentId::Date)) {
        return;
    }
    if (usesDashboardLayout() && full_refresh && fast_dashboard_entry_render_) {
        rtc_date_t current_date;
        M5.RTC.getDate(&current_date);
        const String date_text = formatDateOnly(current_date);
        const int16_t date_x =
            dashboardComponentX(logic::DashboardComponentId::Date);
        const int16_t date_y =
            dashboardComponentY(logic::DashboardComponentId::Date);
        date_canvas_.fillCanvas(kWhite);
        date_canvas_.setTextDatum(CL_DATUM);
        date_canvas_.setTextColor(kText);
        setCanvasTextSize(date_canvas_, false, 3);
        date_canvas_.drawString(date_text, 0, kDateH / 2);
        date_canvas_.pushCanvas(date_x, date_y, UPDATE_MODE_NONE);
        last_date_text_rendered_ = "";
        last_holiday_display_rendered_ = "";
        last_weekday_rendered_ = 255;
        return;
    }

    rtc_date_t current_date;
    M5.RTC.getDate(&current_date);

    const String date_text =
        formatConfiguredDate(current_date, settings_.date_format);
    const uint8_t week = calculateWeekday(current_date);
    const logic::HolidayDisplay holiday_display = logic::HolidayDisplayForDate(
        current_date.year, current_date.mon, current_date.day);
    const String holiday_signature =
        settings_.show_holiday ? formatHolidayDisplaySignature(holiday_display)
                               : String("");
    const String pairing_signature =
        blePairingCodeActive() ? "ble:" + ble_pairing_code_ : "";
    const String holiday_pairing_signature =
        holiday_signature + "|" + pairing_signature + "|" +
        normalizeDateFormat(settings_.date_format) + "|" +
        normalizeWeekdayFormat(settings_.weekday_format) + "|" +
        normalizeDateLayout(settings_.date_layout) + "|" +
        String(normalizeDateTextSize(settings_.date_text_size));
    if (date_text == last_date_text_rendered_ &&
        week == last_weekday_rendered_ &&
        holiday_pairing_signature == last_holiday_display_rendered_) {
        return;
    }

    const bool date_changed = date_text != last_date_text_rendered_;
    const bool weekday_changed = week != last_weekday_rendered_;
    const bool holiday_changed =
        holiday_pairing_signature != last_holiday_display_rendered_;
    const bool fast_ascii_only = false;
    bool use_date_cjk = date_cjk_font_ready_;
    if (use_date_cjk &&
        !ensureCjkCanvas(date_cjk_canvas_, kDateW, kDateH, {2, 3, 7})) {
        use_date_cjk = false;
        date_cjk_font_ready_ = false;
    }
    M5EPD_Canvas& active_date_canvas =
        use_date_cjk ? date_cjk_canvas_ : date_canvas_;

    active_date_canvas.fillCanvas(kWhite);
    const String weekday_text = weekdayLabel(week, settings_.weekday_format);
    const String date_layout = normalizeDateLayout(settings_.date_layout);
    const uint8_t date_text_size =
        normalizeDateTextSize(settings_.date_text_size);
    const bool show_date = date_layout != "holiday-only";
    const bool show_weekday =
        date_layout != "date-only" && date_layout != "holiday-only";
    const bool show_holiday =
        settings_.show_holiday && date_layout != "date-only" &&
        holiday_display.valid();

    active_date_canvas.setTextDatum(CL_DATUM);
    active_date_canvas.setTextColor(kText);
    if (use_date_cjk) {
        const String holiday_text =
            show_holiday ? formatHolidayDisplayText(
                               holiday_display,
                               holiday_display.state ==
                                   logic::HolidayDisplayState::InHoliday)
                         : String("");
        if (date_layout == "two-line") {
            const uint8_t two_line_text_size =
                std::min<uint8_t>(date_text_size, 2);
            const String first_line = joinDateTextParts(
                show_date ? date_text : String(""),
                show_weekday ? weekday_text : String(""), String(""));
            fitDateTextSize(active_date_canvas, first_line, two_line_text_size,
                            2, kDateW);
            if (!first_line.isEmpty()) {
                drawFauxBoldString(active_date_canvas, first_line, 0, 12);
            }
            if (!holiday_text.isEmpty()) {
                fitDateTextSize(active_date_canvas, holiday_text,
                                two_line_text_size, 2, kDateW);
                drawFauxBoldString(active_date_canvas, holiday_text, 0, 34);
            }
        } else {
            const String line = joinDateTextParts(
                show_date ? date_text : String(""),
                show_weekday ? weekday_text : String(""), holiday_text);
            fitDateTextSize(active_date_canvas, line, date_text_size, 2, kDateW);
            if (!line.isEmpty()) {
                drawFauxBoldString(active_date_canvas, line, 0, kDateH / 2);
            }
            if (blePairingCodeActive()) {
                int16_t pairing_x = 520;
                drawFauxBoldString(active_date_canvas,
                                   "蓝牙配对码 " + ble_pairing_code_,
                                   pairing_x, kDateH / 2);
            }
        }
    } else {
        setCanvasTextSize(active_date_canvas, false, 3);
        if (show_date && !date_text.isEmpty()) {
            active_date_canvas.drawString(date_text, 0, kDateH / 2);
        }
    }
    const int16_t date_x =
        usesDashboardLayout()
            ? dashboardComponentX(logic::DashboardComponentId::Date)
            : kDateX;
    const int16_t date_y =
        usesDashboardLayout()
            ? dashboardComponentY(logic::DashboardComponentId::Date)
            : kDateY;
    active_date_canvas.pushCanvas(date_x, date_y, UPDATE_MODE_NONE);

    if (!full_refresh) {
        const m5epd_update_mode_t mode =
            nextPartialMode(date_partial_count_, settings_.partial_clean_interval);
        if (use_date_cjk) {
            DirtyRect dirty;
            if (holiday_changed) {
                DirtyRect rect;
                rect.x = 0;
                rect.y = 0;
                rect.w = kDateW;
                rect.h = kDateH;
                rect.valid = true;
                includeDirtyRect(dirty, rect);
            } else if (date_changed) {
                DirtyRect rect;
                rect.x = kDateDateAreaX;
                rect.y = 0;
                rect.w = kDateDateAreaW;
                rect.h = kDateH;
                rect.valid = true;
                includeDirtyRect(dirty, rect);
            }
            if (weekday_changed) {
                DirtyRect rect;
                rect.x = kDateWeekdayAreaX;
                rect.y = 0;
                rect.w = kDateWeekdayAreaW;
                rect.h = kDateH;
                rect.valid = true;
                includeDirtyRect(dirty, rect);
            }
            if (dirty.valid) {
                updateAlignedArea(date_x + dirty.x, date_y + dirty.y,
                                  dirty.w, dirty.h, mode);
            }
        } else {
            updateAlignedArea(date_x, date_y, kDateW, kDateH, mode);
        }
        ++partial_refresh_count_;
    } else {
        date_partial_count_ = 0;
    }
    if (fast_ascii_only) {
        last_date_text_rendered_ = "";
        last_holiday_display_rendered_ = "";
        last_weekday_rendered_ = 255;
    } else {
        last_date_text_rendered_ = date_text;
        last_holiday_display_rendered_ = holiday_pairing_signature;
        last_weekday_rendered_ = week;
    }
}

