#include "ClockApp.h"

#include <WiFi.h>

#include "logic/UiLogic.h"
#include "resources/weekday_bitmaps.h"

namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kBorder = 11;
constexpr uint8_t kText = 15;
constexpr uint8_t kMutedText = 8;
constexpr uint8_t kPrimaryFill = 2;
constexpr uint8_t kPressedFill = 4;
constexpr uint8_t kErrorText = 12;
constexpr uint8_t kAccentText = 13;
constexpr uint32_t kClockSensorIntervalMs = 15000;
constexpr uint32_t kClockRefreshIntervalMs = 30000;
constexpr uint32_t kCenterButtonLongPressMs = 1500;
constexpr uint16_t kHeaderHeight = 60;
constexpr int16_t kPasswordFieldX = 28;
constexpr int16_t kPasswordFieldY = 156;
constexpr int16_t kPasswordFieldW = 904;
constexpr int16_t kPasswordFieldH = 58;
constexpr int16_t kPasswordStatusX = 28;
constexpr int16_t kPasswordStatusY = 500;
constexpr int16_t kPasswordStatusW = 904;
constexpr int16_t kPasswordStatusH = 28;
constexpr int16_t kSettingsStatusX = 24;
constexpr int16_t kSettingsStatusY = 86;
constexpr int16_t kSettingsStatusW = 912;
constexpr int16_t kSettingsStatusH = 132;
constexpr int16_t kTimeX = 100;
constexpr int16_t kTimeY = 72;
constexpr int16_t kDateX = 24;
constexpr int16_t kDateY = 20;
constexpr int16_t kDateW = 520;
constexpr int16_t kDateH = 44;
constexpr int16_t kBatteryX = 896;
constexpr int16_t kBatteryY = 24;
constexpr int16_t kBatteryW = 48;
constexpr int16_t kBatteryH = 32;
constexpr int16_t kTimeCanvasW = 760;
constexpr int16_t kTimeCanvasH = 300;
constexpr int16_t kTimeDigitWidth = 150;
constexpr int16_t kTimeDigitHeight = 280;
constexpr int16_t kTimeGap = 24;
constexpr int16_t kTimeDigitY = 10;
constexpr int16_t kTimeColonX = 355;
constexpr int16_t kTimeColonW = 50;
constexpr int16_t kTimeColonY = 10;
constexpr int16_t kTimeColonH = 280;
constexpr int16_t kInfoX = 100;
constexpr int16_t kInfoY = 378;
constexpr int16_t kInfoCanvasW = 760;
constexpr int16_t kInfoCanvasH = 120;
constexpr int16_t kSmallDigitWidth = 54;
constexpr int16_t kSmallDigitHeight = 96;
constexpr int16_t kSmallGap = 8;
constexpr int16_t kInfoDigitY = 6;
constexpr int16_t kHumidityUnitX = 226;
constexpr int16_t kTemperatureDotX = 404;
constexpr int16_t kTemperatureDegreeX = 480;
constexpr int16_t kTemperatureUnitX = 495;

constexpr int16_t kTimeDigitXs[] = {7, 181, 429, 603};
constexpr int16_t kHumidityDigitXs[] = {40, 102, 164};
constexpr int16_t kTemperatureDigitXs[] = {280, 342, 422};

String formatTwoDigits(int value) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d", value);
    return String(buf);
}

String padLeft(const String& value, size_t width) {
    if (value.length() >= width) {
        return value.substring(value.length() - width);
    }

    String padded;
    while (padded.length() + value.length() < width) {
        padded += ' ';
    }
    return padded + value;
}

String formatTimeDigits(const rtc_time_t& time) {
    return formatTwoDigits(time.hour) + formatTwoDigits(time.min);
}

String formatDateTime(const rtc_date_t& date, const rtc_time_t& time) {
    if (date.year < 2020) {
        return String("--/--/-- --:--");
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d", date.year, date.mon,
             date.day, time.hour, time.min);
    return String(buf);
}

String formatTemperature(float value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", value);
    return String(buf);
}

String formatHumidity(float value) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f", value);
    return String(buf);
}

String formatHumidityField(const EnvironmentReading& reading) {
    if (!reading.valid) {
        return String(" --");
    }
    return padLeft(formatHumidity(reading.humidity), 3);
}

String formatTemperatureField(const EnvironmentReading& reading) {
    if (!reading.valid) {
        return String("---");
    }

    const String value = formatTemperature(reading.temperature);
    const int dot_index = value.indexOf('.');
    String whole = dot_index >= 0 ? value.substring(0, dot_index) : value;
    const char tenths =
        dot_index >= 0 && dot_index + 1 < static_cast<int>(value.length())
            ? value.charAt(dot_index + 1)
            : ' ';
    whole = padLeft(whole, 2);
    return whole + String(tenths);
}

String formatDateOnly(const rtc_date_t& date) {
    if (date.year < 2020) {
        return String("");
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", date.year, date.mon, date.day);
    return String(buf);
}

uint8_t calculateWeekday(const rtc_date_t& date) {
    if (date.mon < 1 || date.mon > 12 || date.day < 1 || date.day > 31) {
        return 0;
    }

    static constexpr int kMonthOffsets[] = {0, 3, 2, 5, 0, 3,
                                             5, 1, 4, 6, 2, 4};
    int year = date.year;
    if (date.mon < 3) {
        --year;
    }
    return static_cast<uint8_t>((year + year / 4 - year / 100 + year / 400 +
                                 kMonthOffsets[date.mon - 1] + date.day) %
                                7);
}

const WeekdayBitmap& weekdayBitmap(uint8_t week) {
    return kWeekdayBitmaps[week % 7];
}

void drawSegmentChar(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                     char ch, int16_t width, int16_t height, uint8_t body_color,
                     uint8_t edge_color) {
    canvas.fillCanvas(kWhite);
    if (ch == ' ') {
        return;
    }
    renderer.drawText(canvas, 0, 0, String(ch), width, height, 0, body_color,
                      edge_color);
}

}  // namespace

void ClockApp::begin() {
    initializeHardware();
    createCanvases();
    store_.begin();
    settings_ = store_.load();
    settings_.timezone = logic::ClampTimeZone(settings_.timezone);
    M5.RTC.getTime(&last_time_);
    M5.RTC.getDate(&last_date_);
    renderPage(UPDATE_MODE_GC16, true);
}

void ClockApp::loop() {
    M5.update();
    handleHardwareButtons();
    handleTouch();

    if (current_page_ == PageId::Settings) {
        autoConnectIfNeeded();
    }

    if (current_page_ == PageId::Clock) {
        updateClockPage();
    }
}

void ClockApp::initializeHardware() {
    Serial.begin(115200);
    delay(50);

    pinMode(M5EPD_MAIN_PWR_PIN, OUTPUT);
    pinMode(M5EPD_EXT_PWR_EN_PIN, OUTPUT);
    pinMode(M5EPD_EPD_PWR_EN_PIN, OUTPUT);
    pinMode(M5EPD_KEY_RIGHT_PIN, INPUT);
    pinMode(M5EPD_KEY_PUSH_PIN, INPUT);
    pinMode(M5EPD_KEY_LEFT_PIN, INPUT);

    M5.enableMainPower();
    M5.enableEXTPower();
    M5.enableEPDPower();
    delay(1000);

    M5.EPD.begin(M5EPD_SCK_PIN, M5EPD_MOSI_PIN, M5EPD_MISO_PIN, M5EPD_CS_PIN,
                 M5EPD_BUSY_PIN);
    M5.EPD.Clear(true);
    M5.EPD.SetRotation(M5EPD_Driver::ROTATE_0);
    M5.TP.SetRotation(GT911::ROTATE_0);
    M5.TP.begin(21, 22, 36);
    M5.BatteryADCBegin();
    sensor_.begin();
    M5.RTC.begin();
}

void ClockApp::createCanvases() {
    page_canvas_.createCanvas(kScreenWidth, kScreenHeight);
    page_canvas_.useFreetypeFont(false);
    time_canvas_.createCanvas(kTimeCanvasW, kTimeCanvasH);
    time_canvas_.useFreetypeFont(false);
    info_canvas_.createCanvas(kInfoCanvasW, kInfoCanvasH);
    info_canvas_.useFreetypeFont(false);
    date_canvas_.createCanvas(kDateW, kDateH);
    date_canvas_.useFreetypeFont(false);
    battery_canvas_.createCanvas(kBatteryW, kBatteryH);
    battery_canvas_.useFreetypeFont(false);
    password_field_canvas_.createCanvas(kPasswordFieldW, kPasswordFieldH);
    password_field_canvas_.useFreetypeFont(false);
    password_status_canvas_.createCanvas(kPasswordStatusW, kPasswordStatusH);
    password_status_canvas_.useFreetypeFont(false);

    for (M5EPD_Canvas& canvas : time_digit_canvases_) {
        canvas.setDriver(&M5.EPD);
        canvas.createCanvas(kTimeDigitWidth, kTimeDigitHeight);
        canvas.useFreetypeFont(false);
    }
    for (M5EPD_Canvas& canvas : humidity_digit_canvases_) {
        canvas.setDriver(&M5.EPD);
        canvas.createCanvas(kSmallDigitWidth, kSmallDigitHeight);
        canvas.useFreetypeFont(false);
    }
    for (M5EPD_Canvas& canvas : temperature_digit_canvases_) {
        canvas.setDriver(&M5.EPD);
        canvas.createCanvas(kSmallDigitWidth, kSmallDigitHeight);
        canvas.useFreetypeFont(false);
    }
}

void ClockApp::renderPage(m5epd_update_mode_t mode, bool force) {
    rebuildButtons();
    switch (current_page_) {
        case PageId::Settings:
            renderSettingsPage(mode);
            break;
        case PageId::WifiScan:
            renderWifiScanPage(mode);
            break;
        case PageId::Password:
            renderPasswordPage(mode);
            break;
        case PageId::Clock:
            renderClockPage(force || mode == UPDATE_MODE_GC16);
            break;
    }
}

void ClockApp::renderSettingsPage(m5epd_update_mode_t mode) {
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.setTextColor(kText);
    drawHeader("Clock Setup", false);

    drawSettingsStatusCard(page_canvas_, kSettingsStatusX, kSettingsStatusY);

    const Rect wifi_card {24, 238, 430, 122};
    const Rect tz_card {482, 238, 454, 122};
    drawCard(wifi_card);
    drawCard(tz_card);

    page_canvas_.setTextColor(kMutedText);
    page_canvas_.drawString("Saved Wi-Fi", wifi_card.x + 18, wifi_card.y + 16);
    page_canvas_.setTextColor(kText);
    page_canvas_.drawString(settings_.ssid.isEmpty() ? "Not configured"
                                                     : settings_.ssid,
                            wifi_card.x + 18, wifi_card.y + 52);

    page_canvas_.setTextColor(kMutedText);
    page_canvas_.drawString("Timezone", tz_card.x + 18, tz_card.y + 16);
    page_canvas_.setTextColor(kText);
    page_canvas_.drawString(timezoneLabel(), tz_card.x + 18, tz_card.y + 52);

    for (const Button& button : buttons_) {
        drawButton(button);
    }

    page_canvas_.setTextDatum(TR_DATUM);
    page_canvas_.setTextColor(kMutedText);
    page_canvas_.setTextSize(2);
    page_canvas_.drawString("M5Paper horizontal ink clock", kScreenWidth - 28,
                            kScreenHeight - 22);

    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(mode);
}

void ClockApp::renderWifiScanPage(m5epd_update_mode_t mode) {
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.setTextColor(kText);
    drawHeader("Choose Wi-Fi", true);

    page_canvas_.setTextDatum(TL_DATUM);
    page_canvas_.setTextSize(2);
    page_canvas_.setTextColor(kMutedText);
    page_canvas_.drawString("Tap a network and then enter its password.", 28,
                            82);

    const int visible_count =
        logic::VisibleItemCount(static_cast<int>(wifi_networks_.size()),
                                wifi_page_index_, kWifiPageSize);
    for (int index = 0; index < visible_count; ++index) {
        const int network_index =
            logic::PageStart(wifi_page_index_, kWifiPageSize) + index;
        drawWifiRow(buttons_[index], wifi_networks_[network_index]);
    }

    for (size_t i = static_cast<size_t>(visible_count); i < buttons_.size();
         ++i) {
        drawButton(buttons_[i]);
    }

    page_canvas_.setTextDatum(TR_DATUM);
    page_canvas_.setTextColor(kMutedText);
    const int page_count =
        logic::PageCount(static_cast<int>(wifi_networks_.size()), kWifiPageSize);
    page_canvas_.drawString("Page " + String(wifi_page_index_ + 1) + "/" +
                                String(page_count),
                            kScreenWidth - 28, 82);

    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(mode);
}

void ClockApp::renderPasswordPage(m5epd_update_mode_t mode) {
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.setTextColor(kText);
    drawHeader("Enter Password", true);

    page_canvas_.setTextDatum(TL_DATUM);
    page_canvas_.setTextSize(2);
    page_canvas_.setTextColor(kMutedText);
    page_canvas_.drawString("SSID", 28, 82);
    page_canvas_.setTextColor(kText);
    page_canvas_.drawString(selected_ssid_, 28, 112);

    drawPasswordField(Rect(kPasswordFieldX, kPasswordFieldY, kPasswordFieldW,
                           kPasswordFieldH));

    for (const Button& button : buttons_) {
        drawButton(button);
    }

    if (!status_message_.isEmpty()) {
        page_canvas_.setTextColor(status_error_ ? kErrorText : kAccentText);
        page_canvas_.drawString(status_message_, kPasswordStatusX,
                                kPasswordStatusY + (kPasswordStatusH / 2));
    }

    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(mode);
}

void ClockApp::renderClockPage(bool full_refresh) {
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(UPDATE_MODE_GC16);

    last_time_text_rendered_ = "";
    last_humidity_text_rendered_ = "";
    last_temperature_text_rendered_ = "";
    last_date_text_rendered_ = "";
    last_weekday_rendered_ = 255;
    last_battery_percentage_ = 255;

    updateTimeCanvas(true);
    updateBatteryCanvas(true);
    updateInfoCanvas(true);
    updateDateCanvas(true);
    enter_clock_at_ms_ = millis();
    partial_refresh_count_ = 0;
}

void ClockApp::updateClockPage() {
    rtc_time_t current_time;
    rtc_date_t current_date;
    M5.RTC.getTime(&current_time);
    M5.RTC.getDate(&current_date);

    const bool minute_changed = current_time.min != last_time_.min ||
                                current_time.hour != last_time_.hour;
    const bool time_for_sensor =
        millis() - last_sensor_read_ms_ > kClockSensorIntervalMs;
    const bool time_for_gc16 =
        millis() - enter_clock_at_ms_ > kClockRefreshIntervalMs &&
        partial_refresh_count_ >= 30;

    if (minute_changed || time_for_gc16) {
        updateTimeCanvas(time_for_gc16);
        updateDateCanvas(time_for_gc16);
        last_time_ = current_time;
        last_date_ = current_date;
    }

    if (minute_changed || time_for_sensor || time_for_gc16) {
        updateBatteryCanvas(time_for_gc16);
    }

    if (time_for_sensor || time_for_gc16) {
        updateInfoCanvas(time_for_gc16);
    }
}

void ClockApp::updateTimeCanvas(bool full_refresh) {
    rtc_time_t current_time;
    M5.RTC.getTime(&current_time);

    const String time_digits = formatTimeDigits(current_time);
    if (full_refresh || last_time_text_rendered_.length() != 4) {
        const String time_text =
            time_digits.substring(0, 2) + ":" + time_digits.substring(2);
        time_canvas_.fillCanvas(kWhite);
        renderer_.drawText(time_canvas_, kTimeDigitXs[0], kTimeDigitY, time_text,
                           kTimeDigitWidth, kTimeDigitHeight, kTimeGap, kText,
                           kMutedText);
        time_canvas_.pushCanvas(kTimeX, kTimeY, UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kTimeX, kTimeY, time_canvas_.width(),
                          time_canvas_.height(), UPDATE_MODE_GC16);
        partial_refresh_count_ = 0;
        last_time_text_rendered_ = time_digits;
        return;
    }

    uint32_t changed_regions = 0;
    for (size_t index = 0; index < 4; ++index) {
        if (time_digits.charAt(index) == last_time_text_rendered_.charAt(index)) {
            continue;
        }
        drawSegmentChar(renderer_, time_digit_canvases_[index],
                        time_digits.charAt(index), kTimeDigitWidth,
                        kTimeDigitHeight, kText, kMutedText);
        const int16_t absolute_x = kTimeX + kTimeDigitXs[index];
        const int16_t absolute_y = kTimeY + kTimeDigitY;
        time_digit_canvases_[index].pushCanvas(absolute_x, absolute_y,
                                               UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(absolute_x, absolute_y, kTimeDigitWidth,
                          kTimeDigitHeight, UPDATE_MODE_GL16);
        ++changed_regions;
    }

    if (changed_regions > 0) {
        partial_refresh_count_ += changed_regions;
    }
    last_time_text_rendered_ = time_digits;
}

void ClockApp::updateInfoCanvas(bool full_refresh) {
    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = formatHumidityField(last_environment_);
    const String temperature_text = formatTemperatureField(last_environment_);

    if (full_refresh) {
        info_canvas_.fillCanvas(kWhite);
        info_canvas_.setTextDatum(TL_DATUM);
        renderer_.drawText(info_canvas_, kHumidityDigitXs[0], kInfoDigitY,
                           humidity_text, kSmallDigitWidth, kSmallDigitHeight,
                           kSmallGap, kText, kMutedText);
        info_canvas_.setTextColor(kText);
        info_canvas_.setTextSize(3);
        info_canvas_.drawString("%", kHumidityUnitX, 54);

        const String temperature_display = temperature_text.substring(0, 2) +
                                           "." + temperature_text.substring(2);
        renderer_.drawText(info_canvas_, kTemperatureDigitXs[0], kInfoDigitY,
                           temperature_display, kSmallDigitWidth,
                           kSmallDigitHeight, kSmallGap, kText, kMutedText);
        info_canvas_.setTextColor(kText);
        info_canvas_.drawString(" C", kTemperatureUnitX, 54);
        info_canvas_.setTextSize(2);
        info_canvas_.drawString("o", kTemperatureDegreeX, 44);

        info_canvas_.setTextDatum(CC_DATUM);
        info_canvas_.setTextColor(kText);
        info_canvas_.setTextSize(5);
        info_canvas_.drawString("(^_^)", 640, 58);

        info_canvas_.pushCanvas(kInfoX, kInfoY, UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kInfoX, kInfoY, info_canvas_.width(),
                          info_canvas_.height(), UPDATE_MODE_GC16);
        partial_refresh_count_ = 0;
        last_humidity_text_rendered_ = humidity_text;
        last_temperature_text_rendered_ = temperature_text;
        return;
    }

    uint32_t changed_regions = 0;
    for (size_t index = 0; index < 3; ++index) {
        if (last_humidity_text_rendered_.length() != 3 ||
            humidity_text.charAt(index) != last_humidity_text_rendered_.charAt(index)) {
            drawSegmentChar(renderer_, humidity_digit_canvases_[index],
                            humidity_text.charAt(index), kSmallDigitWidth,
                            kSmallDigitHeight, kText, kMutedText);
            const int16_t absolute_x = kInfoX + kHumidityDigitXs[index];
            const int16_t absolute_y = kInfoY + kInfoDigitY;
            humidity_digit_canvases_[index].pushCanvas(absolute_x, absolute_y,
                                                       UPDATE_MODE_NONE);
            M5.EPD.UpdateArea(absolute_x, absolute_y, kSmallDigitWidth,
                              kSmallDigitHeight, UPDATE_MODE_GL16);
            ++changed_regions;
        }
        if (last_temperature_text_rendered_.length() != 3 ||
            temperature_text.charAt(index) !=
                last_temperature_text_rendered_.charAt(index)) {
            drawSegmentChar(renderer_, temperature_digit_canvases_[index],
                            temperature_text.charAt(index), kSmallDigitWidth,
                            kSmallDigitHeight, kText, kMutedText);
            const int16_t absolute_x = kInfoX + kTemperatureDigitXs[index];
            const int16_t absolute_y = kInfoY + kInfoDigitY;
            temperature_digit_canvases_[index].pushCanvas(absolute_x,
                                                          absolute_y,
                                                          UPDATE_MODE_NONE);
            M5.EPD.UpdateArea(absolute_x, absolute_y, kSmallDigitWidth,
                              kSmallDigitHeight, UPDATE_MODE_GL16);
            ++changed_regions;
        }
    }

    if (changed_regions > 0) {
        partial_refresh_count_ += changed_regions;
    }
    last_humidity_text_rendered_ = humidity_text;
    last_temperature_text_rendered_ = temperature_text;
}

void ClockApp::updateDateCanvas(bool full_refresh) {
    rtc_date_t current_date;
    M5.RTC.getDate(&current_date);

    const String date_text = formatDateOnly(current_date);
    const uint8_t week = calculateWeekday(current_date);
    if (date_text == last_date_text_rendered_ && week == last_weekday_rendered_) {
        return;
    }

    date_canvas_.fillCanvas(kWhite);
    const WeekdayBitmap& weekday = weekdayBitmap(week);

    date_canvas_.setTextDatum(CL_DATUM);
    date_canvas_.setTextColor(kText);
    date_canvas_.setTextSize(3);
    date_canvas_.drawString(date_text, 0, kDateH / 2);

    const int16_t date_width = date_canvas_.textWidth(date_text);
    const int16_t weekday_x = date_width + 24;
    const int16_t weekday_y = (kDateH - weekday.height) / 2;
    if (!date_text.isEmpty() && weekday_x + weekday.width <= kDateW) {
        date_canvas_.pushImage(weekday_x, weekday_y, weekday.width,
                               weekday.height, weekday.data);
    }
    date_canvas_.pushCanvas(kDateX, kDateY, UPDATE_MODE_NONE);

    if (full_refresh) {
        M5.EPD.UpdateArea(kDateX, kDateY, kDateW, kDateH, UPDATE_MODE_GC16);
    } else {
        M5.EPD.UpdateArea(kDateX, kDateY, kDateW, kDateH, UPDATE_MODE_GL16);
        ++partial_refresh_count_;
    }
    last_date_text_rendered_ = date_text;
    last_weekday_rendered_ = week;
}

void ClockApp::updateBatteryCanvas(bool full_refresh) {
    const uint8_t battery = batteryPercentage();
    if (!full_refresh && battery == last_battery_percentage_) {
        return;
    }
    const int16_t body_x = 2;
    const int16_t body_y = 6;
    const int16_t body_w = 38;
    const int16_t body_h = 20;
    const int16_t cap_x = body_x + body_w;
    const int16_t cap_y = 11;
    const int16_t cap_w = 4;
    const int16_t cap_h = 10;
    const int16_t inner_x = body_x + 3;
    const int16_t inner_y = body_y + 3;
    const int16_t inner_w = body_w - 6;
    const int16_t inner_h = body_h - 6;
    const int16_t fill_w = (battery * inner_w) / 100;

    battery_canvas_.fillCanvas(kWhite);
    battery_canvas_.drawRoundRect(body_x, body_y, body_w, body_h, 3, kText);
    battery_canvas_.fillRect(cap_x, cap_y, cap_w, cap_h, kText);
    if (fill_w > 0) {
        battery_canvas_.fillRect(inner_x, inner_y, fill_w, inner_h, kText);
    }
    battery_canvas_.pushCanvas(kBatteryX, kBatteryY, UPDATE_MODE_NONE);

    if (full_refresh) {
        M5.EPD.UpdateArea(kBatteryX, kBatteryY, kBatteryW, kBatteryH,
                          UPDATE_MODE_GC16);
    } else {
        M5.EPD.UpdateArea(kBatteryX, kBatteryY, kBatteryW, kBatteryH,
                          UPDATE_MODE_GL16);
        ++partial_refresh_count_;
    }
    last_battery_percentage_ = battery;
}

void ClockApp::updatePasswordFieldCanvas(m5epd_update_mode_t mode) {
    password_field_canvas_.fillCanvas(kWhite);
    password_field_canvas_.drawRoundRect(0, 0, kPasswordFieldW, kPasswordFieldH,
                                         6, kBorder);
    password_field_canvas_.setTextDatum(CL_DATUM);
    password_field_canvas_.setTextColor(kText);
    password_field_canvas_.setTextSize(2);
    password_field_canvas_.drawString(maskedPassword(), 16, kPasswordFieldH / 2);
    password_field_canvas_.pushCanvas(kPasswordFieldX, kPasswordFieldY,
                                      UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(kPasswordFieldX, kPasswordFieldY, kPasswordFieldW,
                      kPasswordFieldH, mode);
}

void ClockApp::updatePasswordStatusCanvas(m5epd_update_mode_t mode) {
    password_status_canvas_.fillCanvas(kWhite);
    password_status_canvas_.setTextDatum(CL_DATUM);
    password_status_canvas_.setTextSize(2);
    password_status_canvas_.setTextColor(status_error_ ? kErrorText : kAccentText);
    password_status_canvas_.drawString(status_message_, 0, kPasswordStatusH / 2);
    password_status_canvas_.pushCanvas(kPasswordStatusX, kPasswordStatusY,
                                       UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(kPasswordStatusX, kPasswordStatusY, kPasswordStatusW,
                      kPasswordStatusH, mode);
}

void ClockApp::updateSettingsStatusCanvas(m5epd_update_mode_t mode) {
    M5EPD_Canvas settings_status_canvas(&M5.EPD);
    settings_status_canvas.createCanvas(kSettingsStatusW, kSettingsStatusH);
    settings_status_canvas.useFreetypeFont(false);
    settings_status_canvas.fillCanvas(kWhite);
    drawSettingsStatusCard(settings_status_canvas, 0, 0);
    settings_status_canvas.pushCanvas(kSettingsStatusX, kSettingsStatusY,
                                      UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(kSettingsStatusX, kSettingsStatusY, kSettingsStatusW,
                      kSettingsStatusH, mode);
}

void ClockApp::drawHeader(const String& title, bool show_back) {
    (void)show_back;
    page_canvas_.drawFastHLine(0, kHeaderHeight - 1, kScreenWidth, kBorder);
    page_canvas_.setTextDatum(CC_DATUM);
    page_canvas_.setTextSize(4);
    page_canvas_.setTextColor(kText);
    page_canvas_.drawString(title, kScreenWidth / 2, 26);
}

void ClockApp::drawButton(const Button& button, bool pressed) {
    drawButton(page_canvas_, button, pressed);
}

void ClockApp::drawButton(M5EPD_Canvas& canvas, const Button& button, bool pressed) {
    if (!button.visible) {
        return;
    }

    canvas.useFreetypeFont(false);
    const uint8_t fill = pressed ? kPressedFill :
                         (button.primary ? kPrimaryFill : kWhite);
    const uint8_t border = button.enabled ? kBorder : 12;
    canvas.fillRoundRect(button.bounds.x, button.bounds.y, button.bounds.w,
                         button.bounds.h, 6, fill);
    canvas.drawRoundRect(button.bounds.x, button.bounds.y, button.bounds.w,
                         button.bounds.h, 6, border);
    canvas.setTextDatum(CC_DATUM);
    canvas.setTextColor(button.enabled ? kText : kMutedText);
    canvas.setTextSize(2);
    canvas.drawString(button.label, button.bounds.x + button.bounds.w / 2,
                      button.bounds.y + button.bounds.h / 2);
}

void ClockApp::drawCard(const Rect& rect, uint8_t fill, uint8_t border) {
    page_canvas_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, fill);
    page_canvas_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, border);
}

void ClockApp::drawSettingsStatusCard(M5EPD_Canvas& canvas, int16_t origin_x,
                                      int16_t origin_y) {
    canvas.useFreetypeFont(false);
    canvas.fillRoundRect(origin_x, origin_y, kSettingsStatusW, kSettingsStatusH, 6,
                         kWhite);
    canvas.drawRoundRect(origin_x, origin_y, kSettingsStatusW, kSettingsStatusH, 6,
                         kBorder);
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextSize(2);
    canvas.setTextColor(kText);
    canvas.drawString("Network", origin_x + 18, origin_y + 18);
    canvas.drawString("RTC", origin_x + 18, origin_y + 52);
    canvas.drawString("Sync", origin_x + 18, origin_y + 86);
    canvas.setTextColor(kAccentText);
    canvas.drawString(wifiStatusLabel(), origin_x + 160, origin_y + 18);
    canvas.drawString(formatRtcTimestamp(), origin_x + 160, origin_y + 52);
    canvas.setTextColor(status_error_ ? kErrorText : kAccentText);
    canvas.drawString(syncStatusLabel(), origin_x + 160, origin_y + 86);
}

void ClockApp::drawStatusLine(const String& label, const String& value,
                              int16_t x, int16_t y) {
    page_canvas_.setTextDatum(TL_DATUM);
    page_canvas_.setTextSize(2);
    page_canvas_.setTextColor(kMutedText);
    page_canvas_.drawString(label, x, y);
    page_canvas_.setTextColor(kText);
    page_canvas_.drawString(value, x + 140, y);
}

void ClockApp::drawWifiRow(const Button& button, const WiFiNetwork& network) {
    drawButton(button);

    String label = network.ssid;
    if (label.length() > 28) {
        label = label.substring(0, 28) + "...";
    }

    page_canvas_.setTextDatum(CL_DATUM);
    page_canvas_.setTextColor(kText);
    page_canvas_.setTextSize(2);
    page_canvas_.drawString(label, button.bounds.x + 18,
                            button.bounds.y + button.bounds.h / 2);

    page_canvas_.setTextDatum(CR_DATUM);
    page_canvas_.setTextColor(kMutedText);
    page_canvas_.drawString(String(network.rssi) + " dBm",
                            button.bounds.x + button.bounds.w - 18,
                            button.bounds.y + button.bounds.h / 2);
}

void ClockApp::drawPasswordField(const Rect& rect) {
    page_canvas_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, kWhite);
    page_canvas_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, kBorder);
    page_canvas_.setTextDatum(CL_DATUM);
    page_canvas_.setTextColor(kText);
    page_canvas_.setTextSize(2);
    page_canvas_.drawString(maskedPassword(), rect.x + 16, rect.y + rect.h / 2);
}

void ClockApp::updateButtonCanvas(const Button& button, m5epd_update_mode_t mode,
                                  bool pressed) {
    M5EPD_Canvas button_canvas(&M5.EPD);
    button_canvas.createCanvas(button.bounds.w, button.bounds.h);
    button_canvas.useFreetypeFont(false);
    button_canvas.fillCanvas(kWhite);
    Button local_button = button;
    local_button.bounds.x = 0;
    local_button.bounds.y = 0;
    drawButton(button_canvas, local_button, pressed);
    button_canvas.pushCanvas(button.bounds.x, button.bounds.y, UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(button.bounds.x, button.bounds.y, button.bounds.w,
                      button.bounds.h, mode);
}

void ClockApp::rebuildButtons() {
    buttons_.clear();
    switch (current_page_) {
        case PageId::Settings:
            rebuildSettingsButtons();
            break;
        case PageId::WifiScan:
            rebuildWifiButtons();
            break;
        case PageId::Password:
            rebuildPasswordButtons();
            break;
        case PageId::Clock:
            rebuildClockButtons();
            break;
    }
}

void ClockApp::rebuildSettingsButtons() {
    buttons_.push_back(
        Button(kButtonWifi, Rect(24, 378, 276, 72), "Choose Wi-Fi", true, true,
               false));
    buttons_.push_back(Button(kButtonTimezoneMinus, Rect(482, 378, 72, 72), "-",
                              true, true, false));
    buttons_.push_back(Button(kButtonTimezonePlus, Rect(610, 378, 72, 72), "+",
                              true, true, false));
    buttons_.push_back(Button(kButtonSyncTime, Rect(706, 378, 230, 72),
                              "Sync Time", true, true, false));
    buttons_.push_back(Button(kButtonEnterClock, Rect(24, 460, 912, 56),
                              "Enter Clock", true, true, true));
}

void ClockApp::rebuildWifiButtons() {
    const int visible_count =
        logic::VisibleItemCount(static_cast<int>(wifi_networks_.size()),
                                wifi_page_index_, kWifiPageSize);
    for (int index = 0; index < visible_count; ++index) {
        buttons_.push_back(Button(
            kButtonNetworkBase + index,
            Rect(24, static_cast<int16_t>(118 + index * 56), 912, 48), "", true,
            true, false));
    }

    buttons_.push_back(
        Button(kButtonBack, Rect(24, 470, 140, 44), "Back", true, true, false));
    buttons_.push_back(Button(kButtonPrevPage, Rect(590, 470, 100, 44), "Prev",
                              true, wifi_page_index_ > 0, false));
    buttons_.push_back(Button(kButtonRescan, Rect(704, 470, 110, 44), "Rescan",
                              true, true, false));
    buttons_.push_back(Button(
        kButtonNextPage, Rect(828, 470, 108, 44), "Next", true,
        wifi_page_index_ + 1 <
            logic::PageCount(static_cast<int>(wifi_networks_.size()),
                             kWifiPageSize),
        false));
}

void ClockApp::rebuildPasswordButtons() {
    buttons_.push_back(
        Button(kButtonBack, Rect(24, 18, 100, 32), "Back", true, true, false));

    const std::vector<String> keys = activeKeyboardLayout();
    const int16_t key_w = 70;
    const int16_t key_h = 46;
    const int16_t gap = 8;

    for (size_t i = 0; i < 10; ++i) {
        buttons_.push_back(Button(
            kButtonKeyboardBase + static_cast<int>(i),
            Rect(94 + static_cast<int16_t>(i) * (key_w + gap), 246, key_w, key_h),
            keys[i], true, true, false));
    }
    for (size_t i = 0; i < 9; ++i) {
        buttons_.push_back(Button(
            kButtonKeyboardBase + 10 + static_cast<int>(i),
            Rect(133 + static_cast<int16_t>(i) * (key_w + gap), 300, key_w, key_h),
            keys[10 + i], true, true, false));
    }

    buttons_.push_back(
        Button(kButtonShift, Rect(108, 354, 96, key_h), "Shift", true, true, false));
    for (size_t i = 0; i < 7; ++i) {
        buttons_.push_back(Button(
            kButtonKeyboardBase + 19 + static_cast<int>(i),
            Rect(220 + static_cast<int16_t>(i) * (key_w + gap), 354, key_w, key_h),
            keys[19 + i], true, true, false));
    }
    buttons_.push_back(Button(kButtonBackspace, Rect(794, 354, 96, key_h), "Back",
                              true, true, false));

    buttons_.push_back(Button(kButtonKeyboardMode, Rect(108, 408, 96, key_h),
                              keyboard_symbols_ ? "Abc" : "123", true, true,
                              false));
    buttons_.push_back(
        Button(kButtonSpace, Rect(220, 408, 252, key_h), "Space", true, true, false));
    buttons_.push_back(
        Button(kButtonClear, Rect(488, 408, 112, key_h), "Clear", true, true, false));
    buttons_.push_back(Button(kButtonPasswordVisibility,
                              Rect(616, 408, 112, key_h),
                              password_visible_ ? "Hide" : "Show", true, true,
                              false));
    buttons_.push_back(Button(kButtonConnect, Rect(744, 408, 162, key_h),
                              "Connect", true, true, true));
}

void ClockApp::rebuildClockButtons() {
}

void ClockApp::handleTouch() {
    if (!M5.TP.available()) {
        return;
    }

    M5.TP.update();
    const bool finger_up = M5.TP.isFingerUp();
    const int16_t x = M5.TP.readFingerX(0);
    const int16_t y = M5.TP.readFingerY(0);

    if (!finger_up) {
        touch_down_ = true;
        pressed_button_id_ = buttonIdAt(x, y);
    } else if (touch_down_) {
        const int released_button_id = buttonIdAt(x, y);
        if (pressed_button_id_ >= 0 && released_button_id == pressed_button_id_) {
            handleButtonPress(released_button_id);
        }
        pressed_button_id_ = -1;
        touch_down_ = false;
    }

    M5.TP.flush();
}

void ClockApp::handleHardwareButtons() {
    if (M5.BtnP.isPressed() && !center_button_long_press_handled_ &&
        M5.BtnP.pressedFor(kCenterButtonLongPressMs)) {
        center_button_long_press_handled_ = true;
        if (current_page_ != PageId::Settings) {
            switchPage(PageId::Settings);
        }
        return;
    }

    if (M5.BtnP.wasReleased()) {
        if (!center_button_long_press_handled_) {
            refreshCurrentPage();
        }
        center_button_long_press_handled_ = false;
    }
}

void ClockApp::handleButtonPress(int button_id) {
    if (current_page_ == PageId::Settings) {
        switch (button_id) {
            case kButtonWifi:
                switchPage(PageId::WifiScan);
                scanWiFi();
                break;
            case kButtonTimezoneMinus:
                settings_.timezone = logic::ClampTimeZone(settings_.timezone - 1);
                store_.saveTimezone(settings_.timezone);
                renderPage(UPDATE_MODE_GL16);
                break;
            case kButtonTimezonePlus:
                settings_.timezone = logic::ClampTimeZone(settings_.timezone + 1);
                store_.saveTimezone(settings_.timezone);
                renderPage(UPDATE_MODE_GL16);
                break;
            case kButtonSyncTime:
                trySyncTime(true);
                break;
            case kButtonEnterClock:
                switchPage(PageId::Clock);
                break;
        }
        return;
    }

    if (current_page_ == PageId::WifiScan) {
        if (button_id == kButtonBack) {
            switchPage(PageId::Settings);
            return;
        }
        if (button_id == kButtonRescan) {
            scanWiFi();
            return;
        }
        if (button_id == kButtonPrevPage && wifi_page_index_ > 0) {
            --wifi_page_index_;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonNextPage &&
            wifi_page_index_ + 1 <
                logic::PageCount(static_cast<int>(wifi_networks_.size()),
                                 kWifiPageSize)) {
            ++wifi_page_index_;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id >= kButtonNetworkBase &&
            button_id < kButtonNetworkBase + kWifiPageSize) {
            const int index = logic::PageStart(wifi_page_index_, kWifiPageSize) +
                              (button_id - kButtonNetworkBase);
            if (index >= 0 && index < static_cast<int>(wifi_networks_.size())) {
                selected_ssid_ = wifi_networks_[index].ssid;
                password_input_.clear();
                password_visible_ = false;
                status_message_ = "Ready to connect";
                status_error_ = false;
                switchPage(PageId::Password);
            }
        }
        return;
    }

    if (current_page_ == PageId::Password) {
        if (button_id == kButtonBack) {
            switchPage(PageId::WifiScan);
            return;
        }
        if (button_id == kButtonShift) {
            keyboard_upper_ = !keyboard_upper_;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonKeyboardMode) {
            keyboard_symbols_ = !keyboard_symbols_;
            keyboard_upper_ = false;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonBackspace) {
            if (!password_input_.isEmpty()) {
                password_input_.remove(password_input_.length() - 1);
                updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            }
            return;
        }
        if (button_id == kButtonSpace) {
            appendPasswordChar(" ");
            return;
        }
        if (button_id == kButtonClear) {
            password_input_.clear();
            updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonPasswordVisibility) {
            password_visible_ = !password_visible_;
            updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            rebuildPasswordButtons();
            for (const Button& button : buttons_) {
                if (button.id == kButtonPasswordVisibility) {
                    updateButtonCanvas(button, UPDATE_MODE_GL16);
                    break;
                }
            }
            return;
        }
        if (button_id == kButtonConnect) {
            connectSelectedNetwork();
            return;
        }
        if (button_id >= kButtonKeyboardBase &&
            button_id < kButtonKeyboardBase + 26) {
            appendPasswordChar(keyboardCharacterLabel(
                static_cast<size_t>(button_id - kButtonKeyboardBase)));
        }
        return;
    }
}

int ClockApp::buttonIdAt(int16_t x, int16_t y) const {
    for (const Button& button : buttons_) {
        if (button.visible && button.enabled && button.bounds.contains(x, y)) {
            return button.id;
        }
    }
    return -1;
}

void ClockApp::switchPage(PageId page, bool force_full_refresh) {
    current_page_ = page;
    buttons_.clear();
    renderPage(force_full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16, true);
}

void ClockApp::refreshCurrentPage() {
    if (current_page_ == PageId::Clock) {
        renderClockPage(true);
        return;
    }
    renderPage(UPDATE_MODE_GC16, true);
}

void ClockApp::autoConnectIfNeeded() {
    if (auto_connect_attempted_ || settings_.ssid.isEmpty()) {
        return;
    }

    auto_connect_attempted_ = true;
    status_message_ = "Connecting to " + settings_.ssid + "...";
    status_error_ = false;
    updateSettingsStatusCanvas(UPDATE_MODE_GL16);

    if (!connectivity_.ensureConnected(settings_.ssid, settings_.password)) {
        status_message_ = "Wi-Fi connect failed";
        status_error_ = true;
        updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        return;
    }

    status_message_ = "Wi-Fi connected, syncing time...";
    updateSettingsStatusCanvas(UPDATE_MODE_GL16);
    if (trySyncTime(false)) {
        switchPage(PageId::Clock);
    }
}

void ClockApp::scanWiFi() {
    status_message_ = "Scanning Wi-Fi...";
    status_error_ = false;
    renderPage(UPDATE_MODE_GL16);

    wifi_networks_.clear();
    wifi_page_index_ = 0;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    const int count = WiFi.scanNetworks();
    for (int index = 0; index < count; ++index) {
        WiFiNetwork network;
        network.ssid = WiFi.SSID(index);
        network.rssi = WiFi.RSSI(index);
        if (!network.ssid.isEmpty()) {
            wifi_networks_.push_back(network);
        }
    }
    WiFi.scanDelete();

    status_message_ = wifi_networks_.empty() ? "No network found" : "Scan complete";
    status_error_ = wifi_networks_.empty();
    renderPage(UPDATE_MODE_GC16);
}

bool ClockApp::trySyncTime(bool allow_connect) {
    if (allow_connect && !connectivity_.isConnected()) {
        if (settings_.ssid.isEmpty()) {
            status_message_ = "Configure Wi-Fi first";
            status_error_ = true;
            if (current_page_ == PageId::Settings) {
                updateSettingsStatusCanvas(UPDATE_MODE_GL16);
            }
            return false;
        }
        if (!connectivity_.ensureConnected(settings_.ssid, settings_.password)) {
            status_message_ = "Wi-Fi connect failed";
            status_error_ = true;
            if (current_page_ == PageId::Settings) {
                updateSettingsStatusCanvas(UPDATE_MODE_GL16);
            }
            return false;
        }
    }

    if (!connectivity_.isConnected()) {
        status_message_ = "Wi-Fi is offline";
        status_error_ = true;
        if (current_page_ == PageId::Settings) {
            updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        }
        return false;
    }

    if (connectivity_.syncTimeToRtc(settings_.timezone)) {
        settings_.time_synced = true;
        store_.saveTimeSynced(true);
        status_message_ = "Time sync successful";
        status_error_ = false;
        M5.RTC.getTime(&last_time_);
        M5.RTC.getDate(&last_date_);
    } else {
        status_message_ = "Time sync failed";
        status_error_ = true;
    }

    if (current_page_ == PageId::Settings) {
        updateSettingsStatusCanvas(UPDATE_MODE_GL16);
    }
    return !status_error_;
}

void ClockApp::connectSelectedNetwork() {
    if (selected_ssid_.isEmpty()) {
        status_message_ = "Choose a Wi-Fi network";
        status_error_ = true;
        updatePasswordStatusCanvas(UPDATE_MODE_GL16);
        return;
    }

    status_message_ = "Connecting to " + selected_ssid_ + "...";
    status_error_ = false;
    updatePasswordStatusCanvas(UPDATE_MODE_GL16);

    if (!connectivity_.connect(selected_ssid_, password_input_)) {
        status_message_ = "Connection failed";
        status_error_ = true;
        updatePasswordStatusCanvas(UPDATE_MODE_GL16);
        return;
    }

    settings_.ssid = selected_ssid_;
    settings_.password = password_input_;
    store_.saveWifi(settings_.ssid, settings_.password);
    auto_connect_attempted_ = true;
    status_message_ = "Connected";
    status_error_ = false;
    updatePasswordStatusCanvas(UPDATE_MODE_GL16);

    if (trySyncTime(false)) {
        switchPage(PageId::Clock);
        return;
    }
    switchPage(PageId::Settings);
}

String ClockApp::timezoneLabel() const {
    if (settings_.timezone > 0) {
        return "UTC+" + String(settings_.timezone);
    }
    if (settings_.timezone < 0) {
        return "UTC" + String(settings_.timezone);
    }
    return "UTC";
}

String ClockApp::wifiStatusLabel() const {
    if (connectivity_.isConnected()) {
        return "Connected to " + connectivity_.currentSsid();
    }
    if (settings_.ssid.isEmpty()) {
        return "No Wi-Fi configured";
    }
    return "Saved: " + settings_.ssid;
}

String ClockApp::syncStatusLabel() const {
    return status_message_;
}

String ClockApp::formatRtcTimestamp() const {
    rtc_time_t time;
    rtc_date_t date;
    M5.RTC.getTime(&time);
    M5.RTC.getDate(&date);
    return formatDateTime(date, time);
}

uint8_t ClockApp::batteryPercentage() const {
    uint32_t voltage = M5.getBatteryVoltage();
    if (voltage < 3300) {
        voltage = 3300;
    } else if (voltage > 4350) {
        voltage = 4350;
    }

    float battery = static_cast<float>(voltage - 3300) /
                    static_cast<float>(4350 - 3300);
    if (battery <= 0.01f) {
        battery = 0.01f;
    }
    if (battery > 1.0f) {
        battery = 1.0f;
    }
    return static_cast<uint8_t>(battery * 100.0f);
}

String ClockApp::maskedPassword() const {
    if (password_input_.isEmpty()) {
        return "<empty>";
    }

    if (password_visible_) {
        return password_input_;
    }

    String masked;
    masked.reserve(password_input_.length());
    for (size_t index = 0; index < password_input_.length(); ++index) {
        masked += '*';
    }
    return masked;
}

String ClockApp::keyboardCharacterLabel(size_t index) const {
    const std::vector<String> keys = activeKeyboardLayout();
    if (index >= keys.size()) {
        return "";
    }
    return keys[index];
}

std::vector<String> ClockApp::activeKeyboardLayout() const {
    if (keyboard_symbols_) {
        return {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                "-", "/", "@", ".", ":", ";", "(", ")", "!",
                "_", "\"", ",", "?", "#", "$", "&"};
    }

    if (keyboard_upper_) {
        return {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
                "A", "S", "D", "F", "G", "H", "J", "K", "L",
                "Z", "X", "C", "V", "B", "N", "M"};
    }

    return {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
            "a", "s", "d", "f", "g", "h", "j", "k", "l",
            "z", "x", "c", "v", "b", "n", "m"};
}

void ClockApp::appendPasswordChar(const String& text) {
    password_input_ += text;
    updatePasswordFieldCanvas(UPDATE_MODE_GL16);
}
