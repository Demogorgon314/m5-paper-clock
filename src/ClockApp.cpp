#include "ClockApp.h"

#include <algorithm>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "logic/ComfortLogic.h"
#include "logic/HolidayLogic.h"
#include "logic/MarketLogic.h"
#include "logic/UiLogic.h"
#include "resources/wifi_bitmaps.h"

namespace {

constexpr int16_t alignDownTo4(int16_t value) {
    return value & ~0x03;
}

constexpr int16_t alignUpTo4(int16_t value) {
    return (value + 3) & ~0x03;
}

constexpr uint8_t kWhite = 0;
constexpr uint8_t kBorder = 11;
constexpr uint8_t kText = 15;
constexpr uint8_t kMutedText = 8;
constexpr uint8_t kTimeEdgeText = 8;
constexpr uint8_t kPrimaryFill = 2;
constexpr uint8_t kPressedFill = 4;
constexpr uint8_t kErrorText = 12;
constexpr uint8_t kAccentText = 13;
constexpr uint32_t kClockSensorIntervalMs = 15000;
constexpr uint32_t kDashboardMarketIntervalMs = 60000;
constexpr uint32_t kCenterButtonLongPressMs = 1500;
constexpr uint32_t kBackgroundReconnectStartDelayMs = 1800;
constexpr uint32_t kBackgroundReconnectTimeoutMs = 4000;
constexpr uint32_t kBackgroundTimeSyncTimeoutMs = 3000;
constexpr size_t kBleNotifyChunkSize = 20;
constexpr const char* kBleDeviceName = "M5Paper Clock";
constexpr const char* kBleServiceUuid =
    "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
constexpr const char* kBleRxCharacteristicUuid =
    "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
constexpr const char* kBleTxCharacteristicUuid =
    "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
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
constexpr int16_t kDateW = 744;
constexpr int16_t kDateH = 44;
constexpr uint8_t kDateCjkTextSize = 7;
constexpr int16_t kDateCjkDateX = 0;
constexpr int16_t kDateCjkWeekdayX = 142;
constexpr int16_t kDateCjkHolidayX = 222;
constexpr int16_t kBatteryX = 792;
constexpr int16_t kBatteryY = 20;
constexpr int16_t kBatteryW = 152;
constexpr int16_t kBatteryH = 44;
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
constexpr int16_t kHumidityUnitY = 72;
constexpr int16_t kTemperatureDotX = 404;
constexpr int16_t kTemperatureDegreeX = 480;
constexpr int16_t kTemperatureDegreeY = 62;
constexpr int16_t kTemperatureUnitX = 495;
constexpr int16_t kTemperatureUnitY = 72;
constexpr int16_t kTimeDigitXs[] = {7, 181, 429, 603};
constexpr int16_t kHumidityDigitXs[] = {40, 102, 164};
constexpr int16_t kTemperatureDigitXs[] = {280, 342, 422};
constexpr uint8_t kPartialCleanInterval = 4;
constexpr int16_t kTimePartialCanvasW = alignUpTo4(kTimeDigitWidth + 3);
constexpr int16_t kTimePartialCanvasH = alignUpTo4(kTimeDigitHeight + 3);
constexpr int16_t kTimeMinuteCanvasX = alignDownTo4(kTimeX + kTimeColonX);
constexpr int16_t kTimeMinuteCanvasY = kTimeY;
constexpr int16_t kTimeMinuteCanvasW =
    alignUpTo4((kTimeDigitXs[3] + kTimeDigitWidth) - kTimeColonX);
constexpr int16_t kTimeMinuteCanvasH = 292;
constexpr int16_t kTimeMinuteDrawX = (kTimeX + kTimeColonX) - kTimeMinuteCanvasX;
constexpr int16_t kSmallPartialCanvasW = alignUpTo4(kSmallDigitWidth + 3);
constexpr int16_t kSmallPartialCanvasH = alignUpTo4(kSmallDigitHeight + 3);
constexpr int16_t kHumidityCanvasX = alignDownTo4(kInfoX + kHumidityDigitXs[0]);
constexpr int16_t kHumidityCanvasY = kInfoY;
constexpr int16_t kHumidityCanvasW =
    alignUpTo4((kInfoX + kHumidityUnitX + 36) - kHumidityCanvasX);
constexpr int16_t kHumidityCanvasH = kInfoCanvasH;
constexpr int16_t kHumidityDrawX =
    (kInfoX + kHumidityDigitXs[0]) - kHumidityCanvasX;
constexpr int16_t kHumidityUnitDrawX =
    (kInfoX + kHumidityUnitX) - kHumidityCanvasX;
constexpr int16_t kTemperatureCanvasX =
    alignDownTo4(kInfoX + kTemperatureDigitXs[0]);
constexpr int16_t kTemperatureCanvasY = kInfoY;
constexpr int16_t kTemperatureCanvasW =
    alignUpTo4((kInfoX + kTemperatureUnitX + 48) - kTemperatureCanvasX);
constexpr int16_t kTemperatureCanvasH = kInfoCanvasH;
constexpr int16_t kTemperatureDrawX =
    (kInfoX + kTemperatureDigitXs[0]) - kTemperatureCanvasX;
constexpr int16_t kTemperatureDegreeDrawX =
    (kInfoX + kTemperatureDegreeX) - kTemperatureCanvasX;
constexpr int16_t kTemperatureUnitDrawX =
    (kInfoX + kTemperatureUnitX) - kTemperatureCanvasX;
constexpr int16_t kComfortCanvasX = alignDownTo4(kInfoX + 520);
constexpr int16_t kComfortCanvasY = kInfoY;
constexpr int16_t kComfortCanvasW =
    alignUpTo4((kInfoX + kInfoCanvasW) - kComfortCanvasX);
constexpr int16_t kComfortCanvasH = kInfoCanvasH;
constexpr int16_t kComfortFaceDrawX = (kInfoX + 640) - kComfortCanvasX;
constexpr int16_t kComfortFaceDrawY = 58;
constexpr int16_t kDashboardCalendarX = 72;
constexpr int16_t kDashboardCalendarY = 106;
constexpr int16_t kDashboardCalendarW = 304;
constexpr int16_t kDashboardCalendarH = 232;
constexpr int16_t kDashboardTimeX = 414;
constexpr int16_t kDashboardTimeY = 104;
constexpr int16_t kDashboardTimeW = 468;
constexpr int16_t kDashboardTimeH = 236;
constexpr int16_t kDashboardTimeDigitW = 92;
constexpr int16_t kDashboardTimeDigitH = 210;
constexpr int16_t kDashboardTimeGap = 18;
constexpr int16_t kDashboardSummaryX = 72;
constexpr int16_t kDashboardSummaryY = 392;
constexpr int16_t kDashboardSummaryW = 332;
constexpr int16_t kDashboardSummaryH = 86;
constexpr int16_t kDashboardSummaryTitleX = 16;
constexpr int16_t kDashboardSummaryTitleY = 9;
constexpr int16_t kDashboardSummaryPriceRight = kDashboardSummaryW - 14;
constexpr int16_t kDashboardSummaryPriceY = 16;
constexpr int16_t kDashboardSummaryBottomY = 58;
constexpr int16_t kDashboardSummaryArrowGap = 12;
constexpr int16_t kDashboardSummaryValueGap = 14;
constexpr int16_t kDashboardClimateX = 430;
constexpr int16_t kDashboardClimateY = 392;
constexpr int16_t kDashboardClimateW = 452;
constexpr int16_t kDashboardClimateH = 86;
constexpr int16_t kDashboardClimateHumidityDividerX = 126;
constexpr int16_t kDashboardClimateTemperatureDividerX = 286;
constexpr uint8_t kCalendarPastFill = 4;
constexpr uint8_t kCalendarTodayFill = 15;

struct PartialRegion {
    int16_t update_x = 0;
    int16_t update_y = 0;
    int16_t draw_x = 0;
    int16_t draw_y = 0;
};

inline uint16_t uiTextPixelSize(uint8_t legacy_size) {
    switch (legacy_size) {
        case 2:
            return 20;
        case 3:
            return 30;
        case 4:
            return 38;
        case 5:
            return 48;
        case 6:
            return 60;
        case 7:
            return 24;
        default:
            break;
    }
    return static_cast<uint16_t>(legacy_size) * 10;
}

void setCanvasTextSize(M5EPD_Canvas& canvas, bool use_cjk_font,
                       uint8_t legacy_size) {
    canvas.useFreetypeFont(use_cjk_font);
    canvas.setTextSize(use_cjk_font ? uiTextPixelSize(legacy_size) : legacy_size);
}

bool ensureCanvasSize(M5EPD_Canvas& canvas, int16_t width, int16_t height) {
    if (canvas.width() == width && canvas.height() == height) {
        return true;
    }
    canvas.deleteCanvas();
    return canvas.createCanvas(width, height) != nullptr;
}

void drawFauxBoldString(M5EPD_Canvas& canvas, const String& text, int16_t x,
                        int16_t y, int16_t offset_x = 1) {
    canvas.drawString(text, x, y);
    if (offset_x != 0 && !text.isEmpty()) {
        canvas.drawString(text, x + offset_x, y);
    }
}

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

String formatHolidayDisplaySignature(const logic::HolidayDisplay& display) {
    if (!display.valid()) {
        return String("");
    }

    char buf[40];
    snprintf(buf, sizeof(buf), "%u:%u:%d:%u:%u",
             static_cast<unsigned>(display.state),
             static_cast<unsigned>(display.id), display.days_remaining,
             static_cast<unsigned>(display.holiday_day_index),
             static_cast<unsigned>(display.holiday_total_days));
    return String(buf);
}

String weekdayLabel(uint8_t week) {
    switch (week % 7) {
        case 0:
            return String(u8"周日");
        case 1:
            return String(u8"周一");
        case 2:
            return String(u8"周二");
        case 3:
            return String(u8"周三");
        case 4:
            return String(u8"周四");
        case 5:
            return String(u8"周五");
        case 6:
        default:
            return String(u8"周六");
    }
}

String calendarWeekdayLabel(uint8_t week) {
    switch (week % 7) {
        case 0:
            return String(u8"日");
        case 1:
            return String(u8"一");
        case 2:
            return String(u8"二");
        case 3:
            return String(u8"三");
        case 4:
            return String(u8"四");
        case 5:
            return String(u8"五");
        case 6:
        default:
            return String(u8"六");
    }
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

bool isLeapYear(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

uint8_t daysInMonth(int year, int month) {
    static constexpr uint8_t kMonthDays[] = {31, 28, 31, 30, 31, 30,
                                             31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) {
        return 30;
    }
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return kMonthDays[month - 1];
}

String formatDashboardMonth(const rtc_date_t& date) {
    return formatTwoDigits(date.mon);
}

String formatDashboardSummaryTitle(const rtc_date_t& date) {
    return String(date.year) + "." + formatTwoDigits(date.mon);
}

String formatDashboardSummaryDetail(const rtc_date_t& date) {
    return "Day " + String(date.day) + " / " +
           String(daysInMonth(date.year, date.mon));
}

String marketSummarySignature(const MarketQuote& quote, bool wifi_connected) {
    if (quote.valid) {
        const String quote_time =
            quote.updated_at.length() >= 12
                ? quote.updated_at.substring(8, 10) + ":" +
                      quote.updated_at.substring(10, 12)
                : String("");
        rtc_date_t current_date;
        rtc_time_t current_time;
        M5.RTC.getDate(&current_date);
        M5.RTC.getTime(&current_time);
        const bool market_open = logic::IsCnAShareMarketOpen(
            current_date.year, current_date.mon, current_date.day,
            current_time.hour, current_time.min);
        return String("valid|") + quote.symbol + "|" + quote.display_name + "|" +
               quote.price + "|" + quote.change + "|" + quote.change_percent +
               "|" + (quote.positive ? "1" : "0") + "|" +
               (market_open ? "1" : "0") + "|" + quote_time;
    }

    const String status_detail =
        !quote.error_message.isEmpty()
            ? quote.error_message
            : (wifi_connected ? "Data unavailable" : "Wi-Fi offline");
    return String("invalid|") + quote.request_symbol + "|" + quote.symbol + "|" +
           quote.display_name + "|" + status_detail;
}

String marketQuoteTimeLabel(const MarketQuote& quote) {
    if (quote.updated_at.length() < 12) {
        return String("");
    }
    return quote.updated_at.substring(8, 10) + ":" +
           quote.updated_at.substring(10, 12);
}

String marketStatusLabel(const MarketQuote& quote, bool market_open,
                         bool use_cjk_font) {
    const String quote_time = marketQuoteTimeLabel(quote);
    if (use_cjk_font) {
        if (market_open) {
            return quote_time.isEmpty() ? String(u8"交易中")
                                        : String(u8"更新 ") + quote_time;
        }
        return quote_time.isEmpty() ? String(u8"休市")
                                    : String(u8"休市 ") + quote_time;
    }

    if (market_open) {
        return quote_time.isEmpty() ? String("Open")
                                    : String("Upd ") + quote_time;
    }
    return quote_time.isEmpty() ? String("Closed")
                                : String("Closed ") + quote_time;
}

String marketDisplayLabel(const MarketQuote& quote, bool prefer_display_name) {
    if (prefer_display_name && !quote.display_name.isEmpty()) {
        return quote.display_name;
    }
    if (!quote.symbol.isEmpty()) {
        return quote.symbol;
    }
    return String(u8"上证指数");
}

String marketCodeFromRequestSymbol(const String& request_symbol) {
    if (request_symbol.length() >= 6) {
        return request_symbol.substring(request_symbol.length() - 6);
    }
    return request_symbol;
}

bool marketSearchMatchesQuery(const MarketSearchResult& result,
                              const String& query) {
    return query.isEmpty() || result.code.startsWith(query) ||
           result.request_symbol.startsWith(query);
}

String formatDashboardHumidity(const EnvironmentReading& reading) {
    if (!reading.valid) {
        return String("--");
    }
    return formatHumidity(reading.humidity);
}

String formatDashboardTemperature(const EnvironmentReading& reading) {
    if (!reading.valid) {
        return String("--.-");
    }
    return formatTemperature(reading.temperature);
}

void drawHolidayDisplay(M5EPD_Canvas& canvas, int16_t start_x,
                        const logic::HolidayDisplay& display,
                        bool show_progress) {
    if (!display.valid() || display.id == logic::HolidayId::None) {
        return;
    }

    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, true, kDateCjkTextSize);
    switch (display.state) {
        case logic::HolidayDisplayState::Countdown:
            drawFauxBoldString(
                canvas,
                String(u8"距 ") + String(logic::HolidayNameZh(display.id)) +
                    String(u8" 还有 ") + String(display.days_remaining) +
                    String(u8" 天"),
                start_x, kDateH / 2);
            break;
        case logic::HolidayDisplayState::InHoliday: {
            String text = String(logic::HolidayNameZh(display.id)) +
                          String(u8"假期中");
            if (show_progress && display.holiday_day_index > 0 &&
                display.holiday_total_days > 0) {
                text += " " + String(display.holiday_day_index) + "/" +
                        String(display.holiday_total_days);
            }
            drawFauxBoldString(canvas, text, start_x, kDateH / 2);
            break;
        }
        case logic::HolidayDisplayState::LastDay:
            drawFauxBoldString(canvas,
                               String(logic::HolidayNameZh(display.id)) +
                                   String(u8"最后一天"),
                               start_x, kDateH / 2);
            break;
        case logic::HolidayDisplayState::None:
        default:
            break;
    }
}

PartialRegion makePartialRegion(int16_t x, int16_t y) {
    const int16_t update_x = alignDownTo4(x);
    const int16_t update_y = alignDownTo4(y);
    PartialRegion region;
    region.update_x = update_x;
    region.update_y = update_y;
    region.draw_x = x - update_x;
    region.draw_y = y - update_y;
    return region;
}

void drawSegmentCharAt(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                       int16_t x, int16_t y, char ch, int16_t width,
                       int16_t height, uint8_t body_color,
                       uint8_t edge_color) {
    if (ch == ' ') {
        return;
    }
    renderer.drawText(canvas, x, y, String(ch), width, height, 0, body_color,
                      edge_color);
}

void drawSegmentChar(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                     char ch, int16_t width, int16_t height, uint8_t body_color,
                     uint8_t edge_color) {
    canvas.fillCanvas(kWhite);
    drawSegmentCharAt(renderer, canvas, 0, 0, ch, width, height, body_color,
                      edge_color);
}

void drawMinuteTime(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                    const String& time_digits, uint8_t body_color,
                    uint8_t edge_color) {
    if (time_digits.length() != 4) {
        return;
    }

    const String minute_text = ":" + time_digits.substring(2);
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextColor(body_color);
    renderer.drawText(canvas, kTimeMinuteDrawX, kTimeDigitY, minute_text,
                      kTimeDigitWidth, kTimeDigitHeight, kTimeGap, body_color,
                      edge_color);
}

void drawHumidityInfo(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                      const String& humidity_text, uint8_t body_color,
                      uint8_t edge_color, bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    renderer.drawText(canvas, kHumidityDrawX, kInfoDigitY, humidity_text,
                      kSmallDigitWidth, kSmallDigitHeight, kSmallGap,
                      body_color, edge_color);
    canvas.setTextColor(body_color);
    setCanvasTextSize(canvas, use_cjk_font, 3);
    canvas.drawString("%", kHumidityUnitDrawX, kHumidityUnitY);
}

void drawTemperatureInfo(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                         const String& temperature_text, uint8_t body_color,
                         uint8_t edge_color, bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    const String temperature_display =
        temperature_text.substring(0, 2) + "." + temperature_text.substring(2);
    renderer.drawText(canvas, kTemperatureDrawX, kInfoDigitY, temperature_display,
                      kSmallDigitWidth, kSmallDigitHeight, kSmallGap,
                      body_color, edge_color);
    canvas.setTextColor(body_color);
    setCanvasTextSize(canvas, use_cjk_font, 3);
    canvas.drawString(" C", kTemperatureUnitDrawX, kTemperatureUnitY);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString("o", kTemperatureDegreeDrawX, kTemperatureDegreeY);
}

String comfortFace(const EnvironmentReading& reading,
                   const logic::ComfortSettings& comfort_settings) {
    return String(logic::ComfortFaceText(logic::ComfortStateForReading(
        reading.temperature, reading.humidity, reading.valid,
        comfort_settings)));
}

void drawComfortInfoAt(M5EPD_Canvas& canvas, int16_t center_x, int16_t center_y,
                       const String& face, uint8_t color, bool use_cjk_font) {
    const auto drawWideLine = [&](int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                  int16_t thickness) {
        const int16_t radius = thickness / 2;
        for (int16_t offset = -radius; offset <= radius; ++offset) {
            canvas.drawLine(x0, y0 + offset, x1, y1 + offset, color);
            if (offset != 0) {
                canvas.drawLine(x0 + offset, y0, x1 + offset, y1, color);
            }
        }
    };
    const auto drawCaret = [&](int16_t x, int16_t bottom_y, int16_t half_w,
                               int16_t rise, int16_t thickness) {
        drawWideLine(x - half_w, bottom_y, x, bottom_y - rise, thickness);
        drawWideLine(x + half_w, bottom_y, x, bottom_y - rise, thickness);
    };
    const auto drawDash = [&](int16_t x, int16_t y, int16_t half_w,
                              int16_t thickness) {
        const int16_t radius = thickness / 2;
        for (int16_t offset = -radius; offset <= radius; ++offset) {
            canvas.drawFastHLine(x - half_w, y + offset, half_w * 2 + 1, color);
        }
    };

    canvas.setTextDatum(CC_DATUM);
    canvas.setTextColor(color);
    setCanvasTextSize(canvas, use_cjk_font, 6);
    canvas.drawString("(", center_x - 78, center_y);
    canvas.drawString(")", center_x + 78, center_y);

    if (face == "(^_^)") {
        drawCaret(center_x - 30, center_y - 2, 10, 12, 4);
        drawCaret(center_x + 30, center_y - 2, 10, 12, 4);
        drawDash(center_x, center_y + 18, 12, 4);
        return;
    }

    if (face == "(-^-)") {
        drawDash(center_x - 34, center_y - 8, 10, 2);
        drawDash(center_x + 34, center_y - 8, 10, 2);
        drawCaret(center_x, center_y + 14, 9, 10, 2);
        return;
    }

    drawDash(center_x - 34, center_y - 8, 10, 2);
    drawDash(center_x + 34, center_y - 8, 10, 2);
    drawDash(center_x, center_y + 18, 12, 2);
}

void drawComfortInfo(M5EPD_Canvas& canvas, const String& face, uint8_t color,
                     bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    drawComfortInfoAt(canvas, kComfortFaceDrawX, kComfortFaceDrawY, face, color,
                      use_cjk_font);
}

void drawCompactComfortInfoAt(M5EPD_Canvas& canvas, int16_t center_x,
                              int16_t center_y, const String& face,
                              uint8_t color, bool use_cjk_font) {
    const auto drawWideLine = [&](int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                  int16_t thickness) {
        const int16_t radius = thickness / 2;
        for (int16_t offset = -radius; offset <= radius; ++offset) {
            canvas.drawLine(x0, y0 + offset, x1, y1 + offset, color);
            if (offset != 0) {
                canvas.drawLine(x0 + offset, y0, x1 + offset, y1, color);
            }
        }
    };
    const auto drawCaret = [&](int16_t x, int16_t bottom_y, int16_t half_w,
                               int16_t rise, int16_t thickness) {
        drawWideLine(x - half_w, bottom_y, x, bottom_y - rise, thickness);
        drawWideLine(x + half_w, bottom_y, x, bottom_y - rise, thickness);
    };
    const auto drawDash = [&](int16_t x, int16_t y, int16_t half_w,
                              int16_t thickness) {
        const int16_t radius = thickness / 2;
        for (int16_t offset = -radius; offset <= radius; ++offset) {
            canvas.drawFastHLine(x - half_w, y + offset, half_w * 2 + 1, color);
        }
    };

    canvas.setTextDatum(CC_DATUM);
    canvas.setTextColor(color);
    setCanvasTextSize(canvas, use_cjk_font, 4);
    canvas.drawString("(", center_x - 40, center_y);
    canvas.drawString(")", center_x + 40, center_y);

    if (face == "(^_^)") {
        drawCaret(center_x - 18, center_y - 2, 6, 7, 4);
        drawCaret(center_x + 18, center_y - 2, 6, 7, 4);
        drawDash(center_x, center_y + 10, 7, 4);
        return;
    }

    if (face == "(-^-)") {
        drawDash(center_x - 20, center_y - 5, 6, 2);
        drawDash(center_x + 20, center_y - 5, 6, 2);
        drawCaret(center_x, center_y + 9, 6, 7, 2);
        return;
    }

    drawDash(center_x - 20, center_y - 5, 6, 2);
    drawDash(center_x + 20, center_y - 5, 6, 2);
    drawDash(center_x, center_y + 10, 7, 2);
}

uint8_t wifiSignalLevelFromRssi(int32_t rssi) {
    if (rssi >= -55) {
        return 4;
    }
    if (rssi >= -67) {
        return 3;
    }
    if (rssi >= -75) {
        return 2;
    }
    return 1;
}

uint8_t wifiBitmapLevel(uint8_t signal_level) {
    if (signal_level >= 3) {
        return 3;
    }
    if (signal_level >= 2) {
        return 2;
    }
    return 1;
}

void drawWifiStatusIcon(M5EPD_Canvas& canvas, int16_t origin_x, int16_t origin_y,
                        bool connected, uint8_t signal_level) {
    const WifiBitmap& bitmap = wifiBitmapForLevel(wifiBitmapLevel(signal_level));
    canvas.pushImage(origin_x, origin_y, bitmap.width, bitmap.height, bitmap.data);

    if (connected) {
        return;
    }

    canvas.drawLine(origin_x + 5, origin_y + 26, origin_x + 26, origin_y + 5,
                    kText);
    canvas.drawLine(origin_x + 6, origin_y + 26, origin_x + 27, origin_y + 5,
                    kText);
}

m5epd_update_mode_t nextPartialMode(uint8_t& count) {
    if (++count >= kPartialCleanInterval) {
        count = 0;
        return UPDATE_MODE_GC16;
    }
    return UPDATE_MODE_GL16;
}

}  // namespace

class BleConfigServerCallbacks : public BLEServerCallbacks {
public:
    explicit BleConfigServerCallbacks(ClockApp* app) : app_(app) {
    }

    void onConnect(BLEServer*) override {
        if (!app_) {
            return;
        }
        app_->ble_config_client_connected_ = true;
        Serial.println("[ble] config client connected");
    }

    void onDisconnect(BLEServer*) override {
        if (!app_) {
            return;
        }
        app_->ble_config_client_connected_ = false;
        Serial.println("[ble] config client disconnected");
        BLEDevice::startAdvertising();
    }

private:
    ClockApp* app_;
};

class BleConfigRxCallbacks : public BLECharacteristicCallbacks {
public:
    explicit BleConfigRxCallbacks(ClockApp* app) : app_(app) {
    }

    void onWrite(BLECharacteristic* characteristic) override {
        if (!app_ || !characteristic) {
            return;
        }
        app_->enqueueBleConfigChunk(characteristic->getValue());
    }

private:
    ClockApp* app_;
};

void ClockApp::begin() {
    Serial.println("[boot] begin: initializeHardware");
    initializeHardware();
    Serial.println("[boot] begin: beginBleConfig");
    beginBleConfig();
    Serial.println("[boot] begin: createCanvases");
    createCanvases();
    Serial.println("[boot] begin: initializeTypography");
    initializeTypography();
    Serial.println("[boot] begin: store.begin");
    store_.begin();
    Serial.println("[boot] begin: store.load");
    settings_ = store_.load();
    settings_.timezone = logic::ClampTimeZone(settings_.timezone);
    if (settings_.market_symbol.isEmpty()) {
        settings_.market_symbol = "sh000001";
    }
    if (settings_.market_name.isEmpty()) {
        settings_.market_name = "上证指数";
    }
    settings_.comfort_settings =
        logic::NormalizeComfortSettings(settings_.comfort_settings);
    clock_style_ = settings_.clock_style == 1 ? ClockStyle::Dashboard
                                              : ClockStyle::Classic;
    seedMarketQuoteFromSettings();
    M5.RTC.getTime(&last_time_);
    M5.RTC.getDate(&last_date_);
    Serial.println("[boot] begin: renderPage");
    renderPage(UPDATE_MODE_GC16, true);
    Serial.println("[boot] begin: ready");
}

void ClockApp::initializeTypography() {
    Serial.println("[font] mounting SPIFFS");
    littlefs_ready_ = SPIFFS.begin(false);
    if (!littlefs_ready_) {
        Serial.println("[font] SPIFFS mount failed; using built-in font");
        cjk_font_ready_ = false;
        date_cjk_font_ready_ = false;
        dashboard_calendar_cjk_font_ready_ = false;
        dashboard_summary_cjk_font_ready_ = false;
        SPIFFS.end();
        return;
    }

    Serial.printf("[font] SPIFFS total=%u used=%u exists=%d\n",
                  SPIFFS.totalBytes(), SPIFFS.usedBytes(),
                  SPIFFS.exists("/cjk.ttf") ? 1 : 0);
    date_cjk_font_ready_ = loadCjkFont(date_cjk_canvas_, {2, 3, 7});
    dashboard_calendar_cjk_font_ready_ =
        loadCjkFont(dashboard_calendar_canvas_, {2, 3, 7});
    dashboard_summary_cjk_font_ready_ =
        loadCjkFont(dashboard_summary_cjk_canvas_, {2, 3, 7});
    cjk_font_ready_ = false;

    Serial.printf("[font] date=%s calendar=%s market=%s\n",
                  date_cjk_font_ready_ ? "ready" : "fallback",
                  dashboard_calendar_cjk_font_ready_ ? "ready" : "fallback",
                  dashboard_summary_cjk_font_ready_ ? "ready" : "fallback");
}

bool ClockApp::loadCjkFont(M5EPD_Canvas& canvas,
                           std::initializer_list<uint8_t> legacy_sizes) {
    if (!littlefs_ready_) {
        Serial.println("[font] load skipped: SPIFFS not ready");
        return false;
    }

    const esp_err_t rc = canvas.loadFont("/cjk.ttf", SPIFFS);
    if (rc != ESP_OK) {
        Serial.printf("[font] loadFont failed rc=%d\n", static_cast<int>(rc));
        return false;
    }

    std::array<bool, 8> created {};
    for (uint8_t legacy_size : legacy_sizes) {
        if (legacy_size >= created.size() || created[legacy_size]) {
            continue;
        }
        created[legacy_size] = true;
        const uint16_t cache_size = legacy_size <= 2 ? 48 : 24;
        const esp_err_t render_rc =
            canvas.createRender(uiTextPixelSize(legacy_size), cache_size);
        if (render_rc != ESP_OK) {
            Serial.printf("[font] createRender failed size=%u rc=%d\n",
                          static_cast<unsigned>(uiTextPixelSize(legacy_size)),
                          static_cast<int>(render_rc));
            return false;
        }
    }
    canvas.useFreetypeFont(false);
    Serial.println("[font] loadFont + createRender OK");
    return true;
}

bool ClockApp::ensureCjkCanvas(M5EPD_Canvas& canvas, int16_t width,
                               int16_t height,
                               std::initializer_list<uint8_t> legacy_sizes) {
    if (canvas.width() == width && canvas.height() == height) {
        return true;
    }

    canvas.deleteCanvas();
    if (canvas.createCanvas(width, height) == nullptr) {
        Serial.printf("[font] createCanvas failed %dx%d\n", width, height);
        return false;
    }
    canvas.useFreetypeFont(false);
    return loadCjkFont(canvas, legacy_sizes);
}

void ClockApp::loop() {
    M5.update();
    handleSerialConfig();
    handleBleConfig();
    handleHardwareButtons();
    handleTouch();
    handleBackgroundConnectivity();

    if (pending_serial_reboot_ &&
        millis() >= pending_serial_reboot_at_ms_) {
        Serial.flush();
        delay(50);
        ESP.restart();
    }

    if (current_page_ == PageId::Settings) {
        autoConnectIfNeeded();
    }

    if (current_page_ == PageId::Clock) {
        if (pending_dashboard_date_cjk_render_) {
            pending_dashboard_date_cjk_render_ = false;
            updateDateCanvas(false);
            return;
        }
        if (pending_dashboard_calendar_cjk_render_) {
            pending_dashboard_calendar_cjk_render_ = false;
            updateDashboardCalendarCanvas(false);
            return;
        }
        if (pending_market_refresh_) {
            pending_market_refresh_ = false;
            if (connectivity_.isConnected()) {
                refreshMarketQuote(true);
                updateDashboardSummaryCanvas(false, false);
            }
        }
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
    minute_canvas_.createCanvas(kTimeMinuteCanvasW, kTimeMinuteCanvasH);
    minute_canvas_.useFreetypeFont(false);
    info_canvas_.createCanvas(kInfoCanvasW, kInfoCanvasH);
    info_canvas_.useFreetypeFont(false);
    humidity_canvas_.createCanvas(kHumidityCanvasW, kHumidityCanvasH);
    humidity_canvas_.useFreetypeFont(false);
    temperature_canvas_.createCanvas(kTemperatureCanvasW, kTemperatureCanvasH);
    temperature_canvas_.useFreetypeFont(false);
    comfort_canvas_.createCanvas(kComfortCanvasW, kComfortCanvasH);
    comfort_canvas_.useFreetypeFont(false);
    date_canvas_.createCanvas(kDateW, kDateH);
    date_canvas_.useFreetypeFont(false);
    date_cjk_canvas_.createCanvas(kDateW, kDateH);
    date_cjk_canvas_.useFreetypeFont(false);
    battery_canvas_.createCanvas(kBatteryW, kBatteryH);
    battery_canvas_.useFreetypeFont(false);
    dashboard_calendar_canvas_.createCanvas(kDashboardCalendarW,
                                            kDashboardCalendarH);
    dashboard_calendar_canvas_.useFreetypeFont(false);
    dashboard_time_canvas_.createCanvas(kDashboardTimeW, kDashboardTimeH);
    dashboard_time_canvas_.useFreetypeFont(false);
    dashboard_summary_canvas_.createCanvas(kDashboardSummaryW,
                                           kDashboardSummaryH);
    dashboard_summary_canvas_.useFreetypeFont(false);
    dashboard_summary_cjk_canvas_.createCanvas(kDashboardSummaryW,
                                               kDashboardSummaryH);
    dashboard_summary_cjk_canvas_.useFreetypeFont(false);
    dashboard_climate_canvas_.createCanvas(kDashboardClimateW,
                                           kDashboardClimateH);
    dashboard_climate_canvas_.useFreetypeFont(false);
    password_field_canvas_.createCanvas(kPasswordFieldW, kPasswordFieldH);
    password_field_canvas_.useFreetypeFont(false);
    password_status_canvas_.createCanvas(kPasswordStatusW, kPasswordStatusH);
    password_status_canvas_.useFreetypeFont(false);

    for (M5EPD_Canvas& canvas : time_digit_canvases_) {
        canvas.setDriver(&M5.EPD);
        canvas.createCanvas(kTimePartialCanvasW, kTimePartialCanvasH);
        canvas.useFreetypeFont(false);
    }
    for (M5EPD_Canvas& canvas : humidity_digit_canvases_) {
        canvas.setDriver(&M5.EPD);
        canvas.createCanvas(kSmallPartialCanvasW, kSmallPartialCanvasH);
        canvas.useFreetypeFont(false);
    }
    for (M5EPD_Canvas& canvas : temperature_digit_canvases_) {
        canvas.setDriver(&M5.EPD);
        canvas.createCanvas(kSmallPartialCanvasW, kSmallPartialCanvasH);
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

    drawSettingsStatusCard(page_canvas_, kSettingsStatusX, kSettingsStatusY,
                           cjk_font_ready_);

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
    setCanvasTextSize(page_canvas_, cjk_font_ready_, 2);
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
    setCanvasTextSize(page_canvas_, cjk_font_ready_, 2);
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
    setCanvasTextSize(page_canvas_, cjk_font_ready_, 2);
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
    if (usesDashboardClockStyle()) {
        renderDashboardClockPage(full_refresh);
        return;
    }
    renderClassicClockPage(full_refresh);
}

void ClockApp::renderClassicClockPage(bool full_refresh) {
    const uint32_t render_started_ms = millis();
    Serial.printf("[perf] renderClassicClockPage start full=%d at=%lu\n",
                  full_refresh ? 1 : 0, render_started_ms);
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);

    last_time_text_rendered_ = "";
    last_humidity_text_rendered_ = "";
    last_temperature_text_rendered_ = "";
    last_comfort_face_rendered_ = "";
    last_market_summary_rendered_ = "";
    last_date_text_rendered_ = "";
    last_holiday_display_rendered_ = "";
    last_weekday_rendered_ = 255;
    last_battery_percentage_ = 255;
    last_wifi_connected_ = false;
    last_wifi_signal_level_ = 255;
    time_digit_partial_counts_.fill(0);
    humidity_digit_partial_counts_.fill(0);
    temperature_digit_partial_counts_.fill(0);
    battery_partial_count_ = 0;

    uint32_t step_started_ms = millis();
    updateTimeCanvas(true);
    Serial.printf("[perf] classic updateTimeCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateBatteryCanvas(true);
    Serial.printf("[perf] classic updateBatteryCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateInfoCanvas(true);
    Serial.printf("[perf] classic updateInfoCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateDateCanvas(true);
    Serial.printf("[perf] classic updateDateCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    const m5epd_update_mode_t mode =
        full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16;
    M5.EPD.UpdateFull(mode);
    Serial.printf("[perf] classic UpdateFull mode=%d took=%lu\n",
                  static_cast<int>(mode), millis() - step_started_ms);
    partial_refresh_count_ = 0;
    Serial.printf("[perf] renderClassicClockPage total=%lu\n",
                  millis() - render_started_ms);
}

void ClockApp::renderDashboardClockPage(bool full_refresh) {
    const uint32_t render_started_ms = millis();
    Serial.printf("[perf] renderDashboardClockPage start full=%d at=%lu\n",
                  full_refresh ? 1 : 0, render_started_ms);
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);

    last_time_text_rendered_ = "";
    last_humidity_text_rendered_ = "";
    last_temperature_text_rendered_ = "";
    last_comfort_face_rendered_ = "";
    last_market_summary_rendered_ = "";
    last_date_text_rendered_ = "";
    last_holiday_display_rendered_ = "";
    last_weekday_rendered_ = 255;
    last_battery_percentage_ = 255;
    last_wifi_connected_ = false;
    last_wifi_signal_level_ = 255;
    time_digit_partial_counts_.fill(0);
    humidity_digit_partial_counts_.fill(0);
    temperature_digit_partial_counts_.fill(0);
    battery_partial_count_ = 0;
    const bool fast_entry = full_refresh && fast_dashboard_entry_render_;

    uint32_t step_started_ms = millis();
    updateDateCanvas(true);
    Serial.printf("[perf] dashboard updateDateCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateBatteryCanvas(true);
    Serial.printf("[perf] dashboard updateBatteryCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateDashboardCalendarCanvas(true);
    Serial.printf("[perf] dashboard updateDashboardCalendarCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateDashboardTimeCanvas(true);
    Serial.printf("[perf] dashboard updateDashboardTimeCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateDashboardSummaryCanvas(true, false);
    Serial.printf("[perf] dashboard updateDashboardSummaryCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateDashboardClimateCanvas(true);
    Serial.printf("[perf] dashboard updateDashboardClimateCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    const m5epd_update_mode_t mode =
        full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16;
    M5.EPD.UpdateFull(mode);
    Serial.printf("[perf] dashboard UpdateFull mode=%d took=%lu\n",
                  static_cast<int>(mode), millis() - step_started_ms);
    partial_refresh_count_ = 0;
    if (fast_entry) {
        pending_dashboard_date_cjk_render_ = true;
        pending_dashboard_calendar_cjk_render_ = true;
    }
    fast_dashboard_entry_render_ = false;
    Serial.printf("[perf] renderDashboardClockPage total=%lu\n",
                  millis() - render_started_ms);
}

void ClockApp::updateClockPage() {
    rtc_time_t current_time;
    rtc_date_t current_date;
    M5.RTC.getTime(&current_time);
    M5.RTC.getDate(&current_date);

    const bool minute_changed = current_time.min != last_time_.min ||
                                current_time.hour != last_time_.hour;
    const bool date_changed = current_date.day != last_date_.day ||
                              current_date.mon != last_date_.mon ||
                              current_date.year != last_date_.year;
    const bool time_for_sensor =
        millis() - last_sensor_read_ms_ > kClockSensorIntervalMs;
    const bool time_for_market =
        millis() - last_market_fetch_ms_ > kDashboardMarketIntervalMs;
    const bool time_for_gc16 =
        minute_changed && current_time.min == 0 &&
        (current_time.hour != last_time_.hour || date_changed);

    if (time_for_gc16) {
        renderClockPage(true);
        last_time_ = current_time;
        last_date_ = current_date;
        return;
    }

    if (usesDashboardClockStyle()) {
        if (minute_changed) {
            updateDashboardTimeCanvas(false);
        }
        if (date_changed) {
            updateDateCanvas(false);
            updateDashboardCalendarCanvas(false);
            updateDashboardSummaryCanvas(false);
        }
        if (time_for_market) {
            updateDashboardSummaryCanvas(false);
        }
        if (minute_changed || time_for_sensor) {
            updateBatteryCanvas(false);
        }
        if (time_for_sensor) {
            updateDashboardClimateCanvas(false);
        }
    } else {
        if (minute_changed) {
            updateTimeCanvas(false);
            updateDateCanvas(false);
        }
        if (minute_changed || time_for_sensor) {
            updateBatteryCanvas(false);
        }
        if (time_for_sensor) {
            updateInfoCanvas(false);
        }
    }
    last_time_ = current_time;
    last_date_ = current_date;
}

void ClockApp::updateTimeCanvas(bool full_refresh) {
    rtc_time_t current_time;
    M5.RTC.getTime(&current_time);

    const String time_digits = formatTimeDigits(current_time);
    if (!full_refresh && time_digits == last_time_text_rendered_) {
        return;
    }

    const bool can_refresh_minutes_only =
        !full_refresh && last_time_text_rendered_.length() == 4 &&
        time_digits.substring(0, 2) == last_time_text_rendered_.substring(0, 2);

    if (can_refresh_minutes_only) {
        drawMinuteTime(renderer_, minute_canvas_, time_digits, kText, kTimeEdgeText);
        minute_canvas_.pushCanvas(kTimeMinuteCanvasX, kTimeMinuteCanvasY,
                                  UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kTimeMinuteCanvasX, kTimeMinuteCanvasY,
                          minute_canvas_.width(), minute_canvas_.height(),
                          UPDATE_MODE_GC16);
        ++partial_refresh_count_;
        last_time_text_rendered_ = time_digits;
        return;
    }

    const String time_text =
        time_digits.substring(0, 2) + ":" + time_digits.substring(2);
    time_canvas_.fillCanvas(kWhite);
    renderer_.drawText(time_canvas_, kTimeDigitXs[0], kTimeDigitY, time_text,
                       kTimeDigitWidth, kTimeDigitHeight, kTimeGap, kText,
                       kTimeEdgeText);
    time_canvas_.pushCanvas(kTimeX, kTimeY, UPDATE_MODE_NONE);

    if (full_refresh) {
        partial_refresh_count_ = 0;
        time_digit_partial_counts_.fill(0);
    } else {
        M5.EPD.UpdateArea(kTimeX, kTimeY, time_canvas_.width(),
                          time_canvas_.height(), UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
    last_time_text_rendered_ = time_digits;
}

void ClockApp::updateInfoCanvas(bool full_refresh) {
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

        info_canvas_.pushCanvas(kInfoX, kInfoY, UPDATE_MODE_NONE);
        partial_refresh_count_ = 0;
        last_humidity_text_rendered_ = humidity_text;
        last_temperature_text_rendered_ = temperature_text;
        last_comfort_face_rendered_ = comfort_face;
        humidity_digit_partial_counts_.fill(0);
        temperature_digit_partial_counts_.fill(0);
        return;
    }

    bool humidity_changed = humidity_text != last_humidity_text_rendered_;
    bool temperature_changed = temperature_text != last_temperature_text_rendered_;
    bool comfort_changed = comfort_face != last_comfort_face_rendered_;

    if (humidity_changed) {
        drawHumidityInfo(renderer_, humidity_canvas_, humidity_text, kText,
                         kMutedText, cjk_font_ready_);
        humidity_canvas_.pushCanvas(kHumidityCanvasX, kHumidityCanvasY,
                                    UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kHumidityCanvasX, kHumidityCanvasY,
                          humidity_canvas_.width(), humidity_canvas_.height(),
                          UPDATE_MODE_GC16);
        ++partial_refresh_count_;
        humidity_digit_partial_counts_.fill(0);
    }

    if (temperature_changed) {
        drawTemperatureInfo(renderer_, temperature_canvas_, temperature_text, kText,
                            kMutedText, cjk_font_ready_);
        temperature_canvas_.pushCanvas(kTemperatureCanvasX, kTemperatureCanvasY,
                                       UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kTemperatureCanvasX, kTemperatureCanvasY,
                          temperature_canvas_.width(),
                          temperature_canvas_.height(), UPDATE_MODE_GC16);
        ++partial_refresh_count_;
        temperature_digit_partial_counts_.fill(0);
    }
    if (comfort_changed) {
        drawComfortInfo(comfort_canvas_, comfort_face, kText, cjk_font_ready_);
        comfort_canvas_.pushCanvas(kComfortCanvasX, kComfortCanvasY,
                                   UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kComfortCanvasX, kComfortCanvasY,
                          comfort_canvas_.width(), comfort_canvas_.height(),
                          UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
    last_humidity_text_rendered_ = humidity_text;
    last_temperature_text_rendered_ = temperature_text;
    last_comfort_face_rendered_ = comfort_face;
}

void ClockApp::updateDateCanvas(bool full_refresh) {
    rtc_date_t current_date;
    M5.RTC.getDate(&current_date);

    const String date_text = formatDateOnly(current_date);
    const uint8_t week = calculateWeekday(current_date);
    const logic::HolidayDisplay holiday_display = logic::HolidayDisplayForDate(
        current_date.year, current_date.mon, current_date.day);
    const String holiday_signature =
        formatHolidayDisplaySignature(holiday_display);
    if (date_text == last_date_text_rendered_ &&
        week == last_weekday_rendered_ &&
        holiday_signature == last_holiday_display_rendered_) {
        return;
    }

    const bool fast_ascii_only = full_refresh && fast_dashboard_entry_render_;
    bool use_date_cjk = date_cjk_font_ready_ && !fast_ascii_only;
    if (use_date_cjk &&
        !ensureCjkCanvas(date_cjk_canvas_, kDateW, kDateH, {2, 3, 7})) {
        use_date_cjk = false;
        date_cjk_font_ready_ = false;
    }
    M5EPD_Canvas& active_date_canvas =
        use_date_cjk ? date_cjk_canvas_ : date_canvas_;

    active_date_canvas.fillCanvas(kWhite);
    const String weekday_text = weekdayLabel(week);

    active_date_canvas.setTextDatum(CL_DATUM);
    active_date_canvas.setTextColor(kText);
    if (use_date_cjk) {
        setCanvasTextSize(active_date_canvas, true, kDateCjkTextSize);
        if (!date_text.isEmpty()) {
            drawFauxBoldString(active_date_canvas, date_text, kDateCjkDateX,
                               kDateH / 2);
        }
        if (!weekday_text.isEmpty()) {
            drawFauxBoldString(active_date_canvas, weekday_text,
                               kDateCjkWeekdayX, kDateH / 2);
        }
        if (holiday_display.valid()) {
            bool show_progress =
                holiday_display.state == logic::HolidayDisplayState::InHoliday;
            drawHolidayDisplay(active_date_canvas, kDateCjkHolidayX,
                               holiday_display, show_progress);
        }
    } else {
        setCanvasTextSize(active_date_canvas, false, 3);
        if (!date_text.isEmpty()) {
            active_date_canvas.drawString(date_text, 0, kDateH / 2);
        }
    }
    active_date_canvas.pushCanvas(kDateX, kDateY, UPDATE_MODE_NONE);

    if (!full_refresh) {
        M5.EPD.UpdateArea(kDateX, kDateY, kDateW, kDateH, UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
    if (fast_ascii_only) {
        last_date_text_rendered_ = "";
        last_holiday_display_rendered_ = "";
        last_weekday_rendered_ = 255;
    } else {
        last_date_text_rendered_ = date_text;
        last_holiday_display_rendered_ = holiday_signature;
        last_weekday_rendered_ = week;
    }
}

void ClockApp::updateBatteryCanvas(bool full_refresh) {
    const uint8_t battery = batteryPercentage();
    const bool wifi_connected = connectivity_.isConnected();
    const uint8_t wifi_signal_level = wifi_connected
                                          ? wifiBitmapLevel(
                                                wifiSignalLevelFromRssi(WiFi.RSSI()))
                                          : 1;
    if (!full_refresh && battery == last_battery_percentage_ &&
        wifi_connected == last_wifi_connected_ &&
        wifi_signal_level == last_wifi_signal_level_) {
        return;
    }
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
    battery_canvas_.drawString(String(battery_label), label_right,
                               kBatteryH / 2);
    drawWifiStatusIcon(battery_canvas_, wifi_x, wifi_y, wifi_connected,
                       wifi_signal_level);
    battery_canvas_.drawRoundRect(body_x, body_y, body_w, body_h, 3, kText);
    battery_canvas_.fillRect(cap_x, cap_y, cap_w, cap_h, kText);
    if (fill_w > 0) {
        battery_canvas_.fillRect(inner_x, inner_y, fill_w, inner_h, kText);
    }
    battery_canvas_.pushCanvas(kBatteryX, kBatteryY, UPDATE_MODE_NONE);

    if (full_refresh) {
        battery_partial_count_ = 0;
    } else {
        const m5epd_update_mode_t mode = nextPartialMode(battery_partial_count_);
        M5.EPD.UpdateArea(kBatteryX, kBatteryY, kBatteryW, kBatteryH,
                          mode);
        ++battery_partial_count_;
    }
    last_battery_percentage_ = battery;
    last_wifi_connected_ = wifi_connected;
    last_wifi_signal_level_ = wifi_signal_level;
}

void ClockApp::updateDashboardCalendarCanvas(bool full_refresh) {
    rtc_date_t current_date;
    M5.RTC.getDate(&current_date);
    const bool fast_ascii_only = full_refresh && fast_dashboard_entry_render_;

    dashboard_calendar_canvas_.fillCanvas(kWhite);
    dashboard_calendar_canvas_.setTextColor(kText);
    dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
    setCanvasTextSize(dashboard_calendar_canvas_, cjk_font_ready_, 5);
    dashboard_calendar_canvas_.drawString(formatDashboardMonth(current_date),
                                          kDashboardCalendarW / 2, 24);

    const int16_t cell_w = 42;
    const int16_t cell_h = 24;
    const int16_t grid_w = cell_w * 7;
    const int16_t start_x = (kDashboardCalendarW - grid_w) / 2;
    const int16_t header_y = 54;
    const int16_t grid_y = 86;

    for (int col = 0; col < 7; ++col) {
        const int16_t center_x = start_x + col * cell_w + (cell_w - 2) / 2;
        if (dashboard_calendar_cjk_font_ready_ && !fast_ascii_only) {
            dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
            dashboard_calendar_canvas_.setTextColor(kText);
            setCanvasTextSize(dashboard_calendar_canvas_, true, 2);
            dashboard_calendar_canvas_.drawString(
                calendarWeekdayLabel(static_cast<uint8_t>(col)),
                center_x, header_y + 8);
        } else {
            static constexpr const char* kFallbackWeekdayLabels[] = {"S", "M", "T",
                                                                     "W", "T", "F",
                                                                     "S"};
            dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
            dashboard_calendar_canvas_.setTextColor(kText);
            setCanvasTextSize(dashboard_calendar_canvas_, false, 2);
            dashboard_calendar_canvas_.drawString(
                kFallbackWeekdayLabels[col], center_x, header_y + 8);
        }
    }

    rtc_date_t first_day = current_date;
    first_day.day = 1;
    const uint8_t first_weekday = calculateWeekday(first_day);
    const uint8_t month_days = daysInMonth(current_date.year, current_date.mon);

    setCanvasTextSize(dashboard_calendar_canvas_, cjk_font_ready_, 2);
    dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            const int day = row * 7 + col - first_weekday + 1;
            if (day < 1 || day > month_days) {
                continue;
            }

            const int16_t x = start_x + col * cell_w;
            const int16_t y = grid_y + row * cell_h;
            const bool is_today = day == current_date.day;
            const bool is_past = day < current_date.day;

            if (is_today) {
                dashboard_calendar_canvas_.fillRect(x, y, cell_w - 2, cell_h - 2,
                                                    kCalendarTodayFill);
            } else if (is_past) {
                dashboard_calendar_canvas_.fillRect(x, y, cell_w - 2, cell_h - 2,
                                                    kCalendarPastFill);
            }
            dashboard_calendar_canvas_.drawRect(x, y, cell_w - 2, cell_h - 2,
                                                kBorder);
            dashboard_calendar_canvas_.setTextColor(is_today ? kWhite : kText);
            dashboard_calendar_canvas_.drawString(
                String(day), x + (cell_w - 2) / 2, y + (cell_h - 2) / 2);
        }
    }

    dashboard_calendar_canvas_.pushCanvas(kDashboardCalendarX, kDashboardCalendarY,
                                          UPDATE_MODE_NONE);
    if (!full_refresh) {
        M5.EPD.UpdateArea(kDashboardCalendarX, kDashboardCalendarY,
                          kDashboardCalendarW, kDashboardCalendarH,
                          UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
}

void ClockApp::updateDashboardTimeCanvas(bool full_refresh) {
    rtc_time_t current_time;
    M5.RTC.getTime(&current_time);

    const String time_digits = formatTimeDigits(current_time);
    if (!full_refresh && time_digits == last_time_text_rendered_) {
        return;
    }

    const String time_text =
        time_digits.substring(0, 2) + ":" + time_digits.substring(2);
    const int16_t text_w =
        renderer_.measureText(time_text, kDashboardTimeDigitW, kDashboardTimeGap);
    const int16_t draw_x = max<int16_t>(0, (kDashboardTimeW - text_w) / 2);
    const int16_t draw_y = (kDashboardTimeH - kDashboardTimeDigitH) / 2;

    dashboard_time_canvas_.fillCanvas(kWhite);
    renderer_.drawText(dashboard_time_canvas_, draw_x, draw_y, time_text,
                       kDashboardTimeDigitW, kDashboardTimeDigitH,
                       kDashboardTimeGap, kText, kTimeEdgeText);
    dashboard_time_canvas_.pushCanvas(kDashboardTimeX, kDashboardTimeY,
                                      UPDATE_MODE_NONE);
    if (!full_refresh) {
        M5.EPD.UpdateArea(kDashboardTimeX, kDashboardTimeY, kDashboardTimeW,
                          kDashboardTimeH, UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
    last_time_text_rendered_ = time_digits;
}

void ClockApp::updateDashboardSummaryCanvas(bool full_refresh,
                                            bool allow_fetch) {
    if (allow_fetch) {
        refreshMarketQuote(full_refresh);
    } else if (connectivity_.isConnected() &&
               (!market_quote_.valid ||
                millis() - last_market_fetch_ms_ > kDashboardMarketIntervalMs)) {
        pending_market_refresh_ = true;
    }
    const bool wifi_connected = connectivity_.isConnected();
    rtc_date_t current_date;
    rtc_time_t current_time;
    M5.RTC.getDate(&current_date);
    M5.RTC.getTime(&current_time);
    const bool market_open = logic::IsCnAShareMarketOpen(
        current_date.year, current_date.mon, current_date.day, current_time.hour,
        current_time.min);
    const String summary_signature =
        marketSummarySignature(market_quote_, wifi_connected);
    if (!full_refresh && summary_signature == last_market_summary_rendered_) {
        return;
    }

    const bool fast_ascii_only = full_refresh && fast_dashboard_entry_render_;
    bool use_summary_cjk = dashboard_summary_cjk_font_ready_ && !fast_ascii_only;
    if (use_summary_cjk &&
        !ensureCjkCanvas(dashboard_summary_cjk_canvas_, kDashboardSummaryW,
                         kDashboardSummaryH, {2, 3, 7})) {
        use_summary_cjk = false;
        dashboard_summary_cjk_font_ready_ = false;
    }
    M5EPD_Canvas& active_summary_canvas =
        use_summary_cjk ? dashboard_summary_cjk_canvas_
                        : dashboard_summary_canvas_;

    active_summary_canvas.fillCanvas(kWhite);
    active_summary_canvas.drawRoundRect(0, 0, kDashboardSummaryW,
                                        kDashboardSummaryH, 6, kBorder);
    active_summary_canvas.setTextDatum(TL_DATUM);
    active_summary_canvas.setTextColor(kText);
    setCanvasTextSize(active_summary_canvas, use_summary_cjk, 2);
    const String market_title =
        marketDisplayLabel(market_quote_, use_summary_cjk);
    if (use_summary_cjk) {
        drawFauxBoldString(active_summary_canvas, market_title,
                           kDashboardSummaryTitleX, kDashboardSummaryTitleY);
    } else {
        active_summary_canvas.drawString(market_title,
                                         kDashboardSummaryTitleX,
                                         kDashboardSummaryTitleY);
    }

    if (market_quote_.valid) {
        active_summary_canvas.setTextDatum(TR_DATUM);
        setCanvasTextSize(active_summary_canvas, false, 4);
        active_summary_canvas.setTextColor(kText);
        active_summary_canvas.drawString(market_quote_.price,
                                         kDashboardSummaryPriceRight,
                                         kDashboardSummaryPriceY);

        const String change_prefix = market_quote_.positive ? "+" : "";
        const String change_value = change_prefix + market_quote_.change;
        const String percent_value =
            change_prefix + market_quote_.change_percent + "%";
        const String status_label =
            marketStatusLabel(market_quote_, market_open, use_summary_cjk);
        setCanvasTextSize(active_summary_canvas, false, 2);
        active_summary_canvas.setTextColor(kText);
        active_summary_canvas.setTextDatum(TR_DATUM);
        const int16_t percent_right = kDashboardSummaryPriceRight;
        const int16_t percent_width =
            active_summary_canvas.textWidth(percent_value);
        const int16_t change_right =
            percent_right - percent_width - kDashboardSummaryValueGap;
        const int16_t change_width =
            active_summary_canvas.textWidth(change_value);
        const int16_t change_left = change_right - change_width;
        const int16_t arrow_center_x = change_left - kDashboardSummaryArrowGap;
        const int16_t arrow_center_y = kDashboardSummaryBottomY + 5;
        if (market_quote_.positive) {
            active_summary_canvas.fillTriangle(arrow_center_x - 8,
                                               arrow_center_y + 6,
                                               arrow_center_x,
                                               arrow_center_y - 6,
                                               arrow_center_x + 8,
                                               arrow_center_y + 6, kText);
        } else {
            active_summary_canvas.fillTriangle(arrow_center_x - 8,
                                               arrow_center_y - 6,
                                               arrow_center_x,
                                               arrow_center_y + 6,
                                               arrow_center_x + 8,
                                               arrow_center_y - 6, kText);
        }
        active_summary_canvas.setTextDatum(TL_DATUM);
        active_summary_canvas.setTextColor(kMutedText);
        setCanvasTextSize(active_summary_canvas, use_summary_cjk, 2);
        active_summary_canvas.drawString(status_label, 16, 58);
        active_summary_canvas.setTextColor(kText);
        setCanvasTextSize(active_summary_canvas, false, 2);
        active_summary_canvas.setTextDatum(TR_DATUM);
        active_summary_canvas.drawString(change_value, change_right,
                                         kDashboardSummaryBottomY);
        active_summary_canvas.drawString(percent_value, percent_right,
                                         kDashboardSummaryBottomY);
    } else {
        active_summary_canvas.setTextDatum(TL_DATUM);
        setCanvasTextSize(active_summary_canvas, false, 3);
        active_summary_canvas.setTextColor(kText);
        active_summary_canvas.drawString("No quote", 16, 34);
        setCanvasTextSize(active_summary_canvas, false, 2);
        active_summary_canvas.setTextColor(kMutedText);
        const String status_detail =
            !market_quote_.error_message.isEmpty()
                ? market_quote_.error_message
                : (wifi_connected ? "Data unavailable" : "Wi-Fi offline");
        active_summary_canvas.drawString(status_detail, 16, 62);
    }

    active_summary_canvas.pushCanvas(kDashboardSummaryX, kDashboardSummaryY,
                                     UPDATE_MODE_NONE);
    if (!full_refresh) {
        M5.EPD.UpdateArea(kDashboardSummaryX, kDashboardSummaryY,
                          kDashboardSummaryW, kDashboardSummaryH,
                          UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
    last_market_summary_rendered_ = fast_ascii_only ? String("") : summary_signature;
}

void ClockApp::updateDashboardClimateCanvas(bool full_refresh) {
    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = formatDashboardHumidity(last_environment_);
    const String temperature_text = formatDashboardTemperature(last_environment_);
    const String comfort_face =
        comfortFace(last_environment_, settings_.comfort_settings);
    const bool humidity_changed = humidity_text != last_humidity_text_rendered_;
    const bool temperature_changed =
        temperature_text != last_temperature_text_rendered_;
    const bool comfort_changed = comfort_face != last_comfort_face_rendered_;

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

    dashboard_climate_canvas_.pushCanvas(kDashboardClimateX, kDashboardClimateY,
                                         UPDATE_MODE_NONE);
    if (!full_refresh) {
        M5.EPD.UpdateArea(kDashboardClimateX, kDashboardClimateY,
                          kDashboardClimateW, kDashboardClimateH,
                          UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
    last_humidity_text_rendered_ = humidity_text;
    last_temperature_text_rendered_ = temperature_text;
    last_comfort_face_rendered_ = comfort_face;
}

bool ClockApp::refreshMarketQuote(bool force) {
    if (!force && millis() - last_market_fetch_ms_ < kDashboardMarketIntervalMs) {
        return market_quote_.valid;
    }

    if (!connectivity_.isConnected()) {
        return market_quote_.valid;
    }

    rtc_date_t current_date;
    rtc_time_t current_time;
    M5.RTC.getDate(&current_date);
    M5.RTC.getTime(&current_time);

    const bool market_open = logic::IsCnAShareMarketOpen(
        current_date.year, current_date.mon, current_date.day, current_time.hour,
        current_time.min);
    if (!force && !market_open && market_quote_.valid) {
        return true;
    }

    last_market_fetch_ms_ = millis();

    const MarketQuote fetched = market_.fetchQuote(settings_.market_symbol);
    if (!fetched.valid) {
        if (!market_quote_.valid) {
            seedMarketQuoteFromSettings();
            market_quote_.error_message = fetched.error_message;
        }
        return market_quote_.valid;
    }

    market_quote_ = fetched;
    if (!settings_.market_name.isEmpty()) {
        market_quote_.display_name = settings_.market_name;
    }
    return true;
}

void ClockApp::updatePasswordFieldCanvas(m5epd_update_mode_t mode) {
    password_field_canvas_.fillCanvas(kWhite);
    password_field_canvas_.drawRoundRect(0, 0, kPasswordFieldW, kPasswordFieldH,
                                         6, kBorder);
    password_field_canvas_.setTextDatum(CL_DATUM);
    password_field_canvas_.setTextColor(kText);
    setCanvasTextSize(password_field_canvas_, cjk_font_ready_, 2);
    password_field_canvas_.drawString(maskedPassword(), 16, kPasswordFieldH / 2);
    password_field_canvas_.pushCanvas(kPasswordFieldX, kPasswordFieldY,
                                      UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(kPasswordFieldX, kPasswordFieldY, kPasswordFieldW,
                      kPasswordFieldH, mode);
}

void ClockApp::updatePasswordStatusCanvas(m5epd_update_mode_t mode) {
    password_status_canvas_.fillCanvas(kWhite);
    password_status_canvas_.setTextDatum(CL_DATUM);
    setCanvasTextSize(password_status_canvas_, cjk_font_ready_, 2);
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
    const bool local_cjk_font_ready = false;
    settings_status_canvas.fillCanvas(kWhite);
    drawSettingsStatusCard(settings_status_canvas, 0, 0, local_cjk_font_ready);
    settings_status_canvas.pushCanvas(kSettingsStatusX, kSettingsStatusY,
                                      UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(kSettingsStatusX, kSettingsStatusY, kSettingsStatusW,
                      kSettingsStatusH, mode);
}

void ClockApp::drawHeader(const String& title, bool show_back) {
    (void)show_back;
    page_canvas_.drawFastHLine(0, kHeaderHeight - 1, kScreenWidth, kBorder);
    page_canvas_.setTextDatum(CC_DATUM);
    setCanvasTextSize(page_canvas_, cjk_font_ready_, 4);
    page_canvas_.setTextColor(kText);
    page_canvas_.drawString(title, kScreenWidth / 2, 26);
}

void ClockApp::drawButton(const Button& button, bool pressed) {
    drawButton(page_canvas_, button, pressed, cjk_font_ready_);
}

void ClockApp::drawButton(M5EPD_Canvas& canvas, const Button& button, bool pressed,
                          bool use_cjk_font) {
    if (!button.visible) {
        return;
    }

    const uint8_t fill = pressed ? kPressedFill :
                         (button.primary ? kPrimaryFill : kWhite);
    const uint8_t border = button.enabled ? kBorder : 12;
    canvas.fillRoundRect(button.bounds.x, button.bounds.y, button.bounds.w,
                         button.bounds.h, 6, fill);
    canvas.drawRoundRect(button.bounds.x, button.bounds.y, button.bounds.w,
                         button.bounds.h, 6, border);
    canvas.setTextDatum(CC_DATUM);
    canvas.setTextColor(button.enabled ? kText : kMutedText);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString(button.label, button.bounds.x + button.bounds.w / 2,
                      button.bounds.y + button.bounds.h / 2);
}

void ClockApp::drawCard(const Rect& rect, uint8_t fill, uint8_t border) {
    page_canvas_.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, fill);
    page_canvas_.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, border);
}

void ClockApp::drawSettingsStatusCard(M5EPD_Canvas& canvas, int16_t origin_x,
                                      int16_t origin_y, bool use_cjk_font) {
    canvas.fillRoundRect(origin_x, origin_y, kSettingsStatusW, kSettingsStatusH, 6,
                         kWhite);
    canvas.drawRoundRect(origin_x, origin_y, kSettingsStatusW, kSettingsStatusH, 6,
                         kBorder);
    canvas.setTextDatum(TL_DATUM);
    setCanvasTextSize(canvas, use_cjk_font, 2);
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
    setCanvasTextSize(page_canvas_, cjk_font_ready_, 2);
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
    setCanvasTextSize(page_canvas_, cjk_font_ready_, 2);
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
    setCanvasTextSize(page_canvas_, cjk_font_ready_, 2);
    page_canvas_.drawString(maskedPassword(), rect.x + 16, rect.y + rect.h / 2);
}

void ClockApp::updateButtonCanvas(const Button& button, m5epd_update_mode_t mode,
                                  bool pressed) {
    M5EPD_Canvas button_canvas(&M5.EPD);
    button_canvas.createCanvas(button.bounds.w, button.bounds.h);
    const bool local_cjk_font_ready = false;
    button_canvas.fillCanvas(kWhite);
    Button local_button = button;
    local_button.bounds.x = 0;
    local_button.bounds.y = 0;
    drawButton(button_canvas, local_button, pressed, local_cjk_font_ready);
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

void ClockApp::handleSerialConfig() {
    while (Serial.available() > 0) {
        const char ch = static_cast<char>(Serial.read());
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            String line = serial_config_rx_buffer_;
            serial_config_rx_buffer_.clear();
            line.trim();
            if (!line.isEmpty()) {
                processConfigLine(line, ConfigTransport::Serial);
            }
            continue;
        }

        if (serial_config_rx_buffer_.length() < 1024) {
            serial_config_rx_buffer_ += ch;
        }
    }
}

void ClockApp::beginBleConfig() {
    if (ble_config_ready_) {
        return;
    }

    BLEDevice::init(kBleDeviceName);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleConfigServerCallbacks(this));

    BLEService* service = server->createService(kBleServiceUuid);
    ble_config_tx_characteristic_ = service->createCharacteristic(
        kBleTxCharacteristicUuid,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    ble_config_tx_characteristic_->addDescriptor(new BLE2902());

    BLECharacteristic* rx_characteristic = service->createCharacteristic(
        kBleRxCharacteristicUuid,
        BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);
    rx_characteristic->setCallbacks(new BleConfigRxCallbacks(this));

    service->start();
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(kBleServiceUuid);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    ble_config_ready_ = true;
    Serial.println("[ble] config service ready");
}

void ClockApp::enqueueBleConfigChunk(const std::string& chunk) {
    if (chunk.empty()) {
        return;
    }

    portENTER_CRITICAL(&ble_config_rx_mux_);
    if (ble_config_pending_chunks_.length() + chunk.size() > 2048) {
        ble_config_pending_chunks_.clear();
        ble_config_rx_buffer_.clear();
        portEXIT_CRITICAL(&ble_config_rx_mux_);
        Serial.println("[ble] config RX overflow");
        return;
    }

    for (char ch : chunk) {
        ble_config_pending_chunks_ += ch;
    }
    portEXIT_CRITICAL(&ble_config_rx_mux_);
}

void ClockApp::handleBleConfig() {
    if (ble_config_pending_chunks_.isEmpty()) {
        return;
    }

    portENTER_CRITICAL(&ble_config_rx_mux_);
    const String pending = ble_config_pending_chunks_;
    ble_config_pending_chunks_.clear();
    portEXIT_CRITICAL(&ble_config_rx_mux_);

    for (uint32_t index = 0; index < pending.length(); ++index) {
        const char ch = pending.charAt(index);
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            String line = ble_config_rx_buffer_;
            ble_config_rx_buffer_.clear();
            line.trim();
            if (!line.isEmpty()) {
                processConfigLine(line, ConfigTransport::Bluetooth);
            }
            continue;
        }

        if (ble_config_rx_buffer_.length() < 1024) {
            ble_config_rx_buffer_ += ch;
        }
    }
}

void ClockApp::processConfigLine(const String& line, ConfigTransport transport) {
    if (!line.startsWith("{")) {
        return;
    }

    StaticJsonDocument<1024> request_doc;
    DeserializationError parse_error =
        deserializeJson(request_doc, line.c_str());
    DynamicJsonDocument response_doc(12288);
    const uint32_t request_id = request_doc["id"] | 0;
    response_doc["id"] = request_id;

    if (parse_error) {
        response_doc["ok"] = false;
        response_doc["error"] = "Invalid JSON request";
        sendConfigDoc(response_doc, transport);
        return;
    }

    const String command = request_doc["cmd"] | "";
    if (command == "get_status") {
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "scan_wifi") {
        wifi_networks_ = scanWiFiNetworks(true);
        wifi_page_index_ = 0;
        status_message_ =
            wifi_networks_.empty() ? "No network found" : "Scan complete";
        status_error_ = wifi_networks_.empty();

        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        JsonArray networks = data.createNestedArray("networks");
        for (const WiFiNetwork& network : wifi_networks_) {
            JsonObject entry = networks.createNestedObject();
            entry["ssid"] = network.ssid;
            entry["rssi"] = network.rssi;
        }
        data["count"] = wifi_networks_.size();
        populateSerialStatus(data.createNestedObject("status"));
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "apply_settings") {
        JsonObject data = request_doc["data"].as<JsonObject>();
        if (data.isNull()) {
            response_doc["ok"] = false;
            response_doc["error"] = "Missing settings payload";
            sendConfigDoc(response_doc, transport);
            return;
        }

        bool settings_changed = false;
        bool wifi_changed = false;
        const bool connect_now = data["connectNow"] | false;
        const bool sync_time = data["syncTime"] | false;

        if (data.containsKey("ssid")) {
            settings_.ssid = String(data["ssid"].as<const char*>());
            settings_changed = true;
            wifi_changed = true;
        }
        if (data.containsKey("password")) {
            settings_.password = String(data["password"].as<const char*>());
            settings_changed = true;
            wifi_changed = true;
        }
        if (data.containsKey("timezone")) {
            settings_.timezone =
                logic::ClampTimeZone(data["timezone"].as<int>());
            store_.saveTimezone(settings_.timezone);
            settings_changed = true;
        }
        if (data.containsKey("clockStyle")) {
            const String style_value =
                String(data["clockStyle"].as<const char*>());
            ClockStyle next_style = clock_style_;
            if (style_value == "dashboard") {
                next_style = ClockStyle::Dashboard;
            } else if (style_value == "classic") {
                next_style = ClockStyle::Classic;
            }
            clock_style_ = next_style;
            settings_.clock_style =
                clock_style_ == ClockStyle::Dashboard ? 1 : 0;
            store_.saveClockStyle(settings_.clock_style);
            settings_changed = true;
        }
        logic::ComfortSettings next_comfort_settings = settings_.comfort_settings;
        bool comfort_settings_changed = false;
        if (data.containsKey("comfortTemperatureMin")) {
            next_comfort_settings.min_temperature =
                data["comfortTemperatureMin"].as<float>();
            comfort_settings_changed = true;
        }
        if (data.containsKey("comfortTemperatureMax")) {
            next_comfort_settings.max_temperature =
                data["comfortTemperatureMax"].as<float>();
            comfort_settings_changed = true;
        }
        if (data.containsKey("comfortHumidityMin")) {
            next_comfort_settings.min_humidity =
                data["comfortHumidityMin"].as<float>();
            comfort_settings_changed = true;
        }
        if (data.containsKey("comfortHumidityMax")) {
            next_comfort_settings.max_humidity =
                data["comfortHumidityMax"].as<float>();
            comfort_settings_changed = true;
        }
        if (comfort_settings_changed) {
            settings_.comfort_settings =
                logic::NormalizeComfortSettings(next_comfort_settings);
            store_.saveComfortSettings(settings_.comfort_settings);
            settings_changed = true;
        }
        if (data.containsKey("marketSymbol")) {
            settings_.market_symbol =
                String(data["marketSymbol"].as<const char*>());
            if (data.containsKey("marketName")) {
                settings_.market_name =
                    String(data["marketName"].as<const char*>());
            } else {
                settings_.market_name =
                    marketCodeFromRequestSymbol(settings_.market_symbol);
            }
            store_.saveMarket(settings_.market_symbol, settings_.market_name);
            seedMarketQuoteFromSettings();
            last_market_summary_rendered_ = "";
            last_market_fetch_ms_ = 0;
            settings_changed = true;
        }

        if (wifi_changed) {
            store_.saveWifi(settings_.ssid, settings_.password);
            settings_.time_synced = false;
            store_.saveTimeSynced(false);
            auto_connect_attempted_ = !connect_now;
        }

        bool ok = true;
        String error_message;
        if (connect_now) {
            cancelBackgroundConnectivity();
            status_message_ = "Connecting to " + settings_.ssid + "...";
            status_error_ = false;
            ok = connectivity_.ensureConnected(settings_.ssid, settings_.password);
            if (ok) {
                auto_connect_attempted_ = true;
                status_message_ = "Connected";
                status_error_ = false;
            } else {
                status_message_ = "Wi-Fi connect failed";
                status_error_ = true;
                error_message = status_message_;
            }
        }

        if (ok && sync_time) {
            ok = trySyncTime(true);
            if (!ok) {
                error_message = status_message_;
            }
        }

        if (ok && connect_now && sync_time && current_page_ != PageId::Clock) {
            switchPage(PageId::Clock);
        } else if (settings_changed || connect_now || sync_time) {
            refreshCurrentPage();
        }

        response_doc["ok"] = ok;
        if (!ok) {
            response_doc["error"] =
                error_message.isEmpty() ? "Apply settings failed" : error_message;
        }
        JsonObject data_out = response_doc.createNestedObject("data");
        populateSerialStatus(data_out);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "search_market") {
        const JsonObject data = request_doc["data"].as<JsonObject>();
        const char* query_value = data["query"] | "";
        const String normalized_query = String(
            logic::NormalizeMarketSearchQuery(std::string(query_value)).c_str());
        std::vector<MarketSearchResult> results =
            connectivity_.isConnected() ? market_.searchMarkets(normalized_query)
                                        : MarketService::hotMarkets();
        if (!connectivity_.isConnected()) {
            results.erase(
                std::remove_if(
                    results.begin(), results.end(),
                    [&](const MarketSearchResult& result) {
                        return !marketSearchMatchesQuery(result, normalized_query);
                    }),
                results.end());
        }

        response_doc["ok"] = true;
        JsonObject data_out = response_doc.createNestedObject("data");
        data_out["query"] = normalized_query;
        data_out["wifiConnected"] = connectivity_.isConnected();
        data_out["count"] = static_cast<uint32_t>(results.size());
        if (!connectivity_.isConnected()) {
            data_out["message"] = "Wi-Fi offline, showing hot symbols only";
        }

        JsonArray entries = data_out.createNestedArray("results");
        for (const MarketSearchResult& result : results) {
            JsonObject entry = entries.createNestedObject();
            entry["requestSymbol"] = result.request_symbol;
            entry["code"] = result.code;
            entry["displayName"] = result.display_name;
            entry["kind"] = result.kind;
            entry["current"] = result.request_symbol == settings_.market_symbol;
        }
        populateSerialStatus(data_out.createNestedObject("status"));
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "set_market") {
        const JsonObject data = request_doc["data"].as<JsonObject>();
        const String request_symbol = String(data["requestSymbol"] | "");
        String display_name = String(data["displayName"] | "");
        if (request_symbol.isEmpty()) {
            response_doc["ok"] = false;
            response_doc["error"] = "Missing requestSymbol";
            sendConfigDoc(response_doc, transport);
            return;
        }

        settings_.market_symbol = request_symbol;
        if (display_name.isEmpty()) {
            display_name = marketCodeFromRequestSymbol(request_symbol);
        }
        settings_.market_name = display_name;
        seedMarketQuoteFromSettings();
        last_market_summary_rendered_ = "";
        last_market_fetch_ms_ = 0;

        if (connectivity_.isConnected()) {
            const MarketQuote fetched = market_.fetchQuote(settings_.market_symbol);
            if (fetched.valid) {
                market_quote_ = fetched;
                if (!settings_.market_name.isEmpty()) {
                    market_quote_.display_name = settings_.market_name;
                }
            } else {
                market_quote_.error_message = fetched.error_message;
            }
        }

        store_.saveMarket(settings_.market_symbol, settings_.market_name);
        if (current_page_ == PageId::Clock) {
            refreshCurrentPage();
        }

        response_doc["ok"] = true;
        JsonObject data_out = response_doc.createNestedObject("data");
        populateSerialStatus(data_out);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "sync_time") {
        const bool ok = trySyncTime(true);
        if (ok && current_page_ != PageId::Clock &&
            connectivity_.isConnected()) {
            switchPage(PageId::Clock);
        } else {
            refreshCurrentPage();
        }

        response_doc["ok"] = ok;
        if (!ok) {
            response_doc["error"] = status_message_;
        }
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "refresh_screen") {
        refreshCurrentPage();
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "reboot") {
        pending_serial_reboot_ = true;
        pending_serial_reboot_at_ms_ = millis() + 250;
        response_doc["ok"] = true;
        response_doc["data"]["message"] = "Reboot scheduled";
        sendConfigDoc(response_doc, transport);
        return;
    }

    response_doc["ok"] = false;
    response_doc["error"] = "Unknown command";
    sendConfigDoc(response_doc, transport);
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
    if (current_page_ == PageId::Clock) {
        if (M5.BtnL.wasReleased()) {
            cycleClockStyle(-1);
            return;
        }
        if (M5.BtnR.wasReleased()) {
            cycleClockStyle(1);
            return;
        }
    }

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
    const uint32_t started_ms = millis();
    Serial.printf("[perf] switchPage from=%d to=%d force_full=%d at=%lu\n",
                  static_cast<int>(current_page_), static_cast<int>(page),
                  force_full_refresh ? 1 : 0, started_ms);
    const PageId previous_page = current_page_;
    current_page_ = page;
    if (page == PageId::Clock && previous_page != PageId::Clock &&
        usesDashboardClockStyle()) {
        fast_dashboard_entry_render_ = true;
        pending_dashboard_date_cjk_render_ = false;
        pending_dashboard_calendar_cjk_render_ = false;
    }
    buttons_.clear();
    renderPage(force_full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16,
               force_full_refresh);
    Serial.printf("[perf] switchPage to=%d done total=%lu\n",
                  static_cast<int>(page), millis() - started_ms);
}

void ClockApp::refreshCurrentPage() {
    if (current_page_ == PageId::Clock) {
        renderClockPage(true);
        startBackgroundReconnect();
        return;
    }
    renderPage(UPDATE_MODE_GC16, true);
}

void ClockApp::startBackgroundReconnect() {
    if (current_page_ != PageId::Clock || settings_.ssid.isEmpty() ||
        connectivity_.isConnected() ||
        background_connectivity_task_ != BackgroundConnectivityTask::Idle) {
        return;
    }

    background_connectivity_task_ =
        BackgroundConnectivityTask::ReconnectScheduled;
    background_connectivity_due_ms_ = millis() + kBackgroundReconnectStartDelayMs;
}

void ClockApp::handleBackgroundConnectivity() {
    if (background_connectivity_task_ == BackgroundConnectivityTask::Idle) {
        return;
    }

    if (background_connectivity_task_ ==
        BackgroundConnectivityTask::ReconnectScheduled) {
        if (current_page_ != PageId::Clock || settings_.ssid.isEmpty() ||
            connectivity_.isConnected()) {
            cancelBackgroundConnectivity();
            return;
        }
        if (millis() < background_connectivity_due_ms_) {
            return;
        }

        const ConnectivityService::AsyncConnectState state =
            connectivity_.beginConnectAsync(settings_.ssid, settings_.password,
                                            kBackgroundReconnectTimeoutMs);
        if (state == ConnectivityService::AsyncConnectState::InProgress) {
            background_connectivity_task_ = BackgroundConnectivityTask::Reconnecting;
            return;
        }

        if (state == ConnectivityService::AsyncConnectState::Succeeded) {
            background_connectivity_task_ = BackgroundConnectivityTask::SyncingTime;
            updateBatteryCanvas(false);
            if (usesDashboardClockStyle()) {
                refreshMarketQuote(true);
                updateDashboardSummaryCanvas(false);
            }
            connectivity_.beginTimeSyncAsync(settings_.timezone,
                                             kBackgroundTimeSyncTimeoutMs);
            return;
        }

        cancelBackgroundConnectivity();
        return;
    }

    if (background_connectivity_task_ == BackgroundConnectivityTask::Reconnecting) {
        const ConnectivityService::AsyncConnectState state =
            connectivity_.pollConnectAsync();
        if (state == ConnectivityService::AsyncConnectState::InProgress ||
            state == ConnectivityService::AsyncConnectState::Idle) {
            return;
        }

        if (state == ConnectivityService::AsyncConnectState::Failed) {
            background_connectivity_task_ = BackgroundConnectivityTask::Idle;
            connectivity_.cancelConnectAsync();
            return;
        }

        connectivity_.cancelConnectAsync();
        background_connectivity_task_ = BackgroundConnectivityTask::SyncingTime;
        if (current_page_ == PageId::Clock) {
            updateBatteryCanvas(false);
            if (usesDashboardClockStyle()) {
                refreshMarketQuote(true);
                updateDashboardSummaryCanvas(false);
            }
        }
        connectivity_.beginTimeSyncAsync(settings_.timezone,
                                         kBackgroundTimeSyncTimeoutMs);
        return;
    }

    const ConnectivityService::AsyncTimeSyncState sync_state =
        connectivity_.pollTimeSyncAsync();
    if (sync_state == ConnectivityService::AsyncTimeSyncState::InProgress ||
        sync_state == ConnectivityService::AsyncTimeSyncState::Idle) {
        return;
    }

    connectivity_.cancelTimeSyncAsync();
    background_connectivity_task_ = BackgroundConnectivityTask::Idle;

    if (sync_state != ConnectivityService::AsyncTimeSyncState::Succeeded) {
        return;
    }

    settings_.time_synced = true;
    store_.saveTimeSynced(true);
    last_time_ = {};
    last_date_ = {};
    if (current_page_ == PageId::Clock) {
        updateClockPage();
    }
}

void ClockApp::cancelBackgroundConnectivity() {
    background_connectivity_task_ = BackgroundConnectivityTask::Idle;
    background_connectivity_due_ms_ = 0;
    connectivity_.cancelConnectAsync();
    connectivity_.cancelTimeSyncAsync();
}

void ClockApp::cycleClockStyle(int delta) {
    const int current_style = static_cast<int>(clock_style_);
    const int next_style = (current_style + delta + 2) % 2;
    const ClockStyle style = static_cast<ClockStyle>(next_style);
    if (style == clock_style_) {
        return;
    }

    clock_style_ = style;
    settings_.clock_style = static_cast<uint8_t>(next_style);
    store_.saveClockStyle(settings_.clock_style);

    if (current_page_ == PageId::Clock) {
        renderClockPage(true);
    }
}

bool ClockApp::usesDashboardClockStyle() const {
    return clock_style_ == ClockStyle::Dashboard;
}

void ClockApp::autoConnectIfNeeded() {
    const uint32_t started_ms = millis();
    Serial.printf("[perf] autoConnectIfNeeded start attempted=%d ssid=%d at=%lu\n",
                  auto_connect_attempted_ ? 1 : 0,
                  settings_.ssid.isEmpty() ? 0 : 1, started_ms);
    if (auto_connect_attempted_ || settings_.ssid.isEmpty()) {
        return;
    }

    auto_connect_attempted_ = true;
    status_message_ = "Connecting to " + settings_.ssid + "...";
    status_error_ = false;
    uint32_t step_started_ms = millis();
    updateSettingsStatusCanvas(UPDATE_MODE_GL16);
    Serial.printf("[perf] autoConnect status(connecting) took=%lu\n",
                  millis() - step_started_ms);

    step_started_ms = millis();
    if (!connectivity_.ensureConnected(settings_.ssid, settings_.password)) {
        Serial.printf("[perf] autoConnect ensureConnected failed after=%lu\n",
                      millis() - step_started_ms);
        status_message_ = "Wi-Fi connect failed";
        status_error_ = true;
        updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        return;
    }
    Serial.printf("[perf] autoConnect ensureConnected ok after=%lu\n",
                  millis() - step_started_ms);

    status_message_ = "Wi-Fi connected, syncing time...";
    step_started_ms = millis();
    updateSettingsStatusCanvas(UPDATE_MODE_GL16);
    Serial.printf("[perf] autoConnect status(syncing) took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    if (trySyncTime(false)) {
        Serial.printf("[perf] autoConnect trySyncTime ok after=%lu\n",
                      millis() - step_started_ms);
        switchPage(PageId::Clock);
        Serial.printf("[perf] autoConnect switchPage done total=%lu\n",
                      millis() - started_ms);
    }
}

void ClockApp::scanWiFi() {
    cancelBackgroundConnectivity();
    status_message_ = "Scanning Wi-Fi...";
    status_error_ = false;
    renderPage(UPDATE_MODE_GL16);

    wifi_networks_ = scanWiFiNetworks(false);
    wifi_page_index_ = 0;

    status_message_ = wifi_networks_.empty() ? "No network found" : "Scan complete";
    status_error_ = wifi_networks_.empty();
    renderPage(UPDATE_MODE_GC16);
}

std::vector<ClockApp::WiFiNetwork> ClockApp::scanWiFiNetworks(bool update_status) {
    cancelBackgroundConnectivity();
    if (update_status) {
        status_message_ = "Scanning Wi-Fi...";
        status_error_ = false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    std::vector<WiFiNetwork> networks;
    const int count = WiFi.scanNetworks();
    for (int index = 0; index < count; ++index) {
        WiFiNetwork network;
        network.ssid = WiFi.SSID(index);
        network.rssi = WiFi.RSSI(index);
        if (!network.ssid.isEmpty()) {
            networks.push_back(network);
        }
    }
    WiFi.scanDelete();

    std::sort(networks.begin(), networks.end(),
              [](const WiFiNetwork& left, const WiFiNetwork& right) {
                  return left.rssi > right.rssi;
              });
    networks.erase(
        std::unique(networks.begin(), networks.end(),
                    [](const WiFiNetwork& left, const WiFiNetwork& right) {
                        return left.ssid == right.ssid;
                    }),
        networks.end());
    return networks;
}

bool ClockApp::trySyncTime(bool allow_connect) {
    const uint32_t started_ms = millis();
    Serial.printf("[perf] trySyncTime start allow_connect=%d connected=%d at=%lu\n",
                  allow_connect ? 1 : 0, connectivity_.isConnected() ? 1 : 0,
                  started_ms);
    if (allow_connect && !connectivity_.isConnected()) {
        if (settings_.ssid.isEmpty()) {
            status_message_ = "Configure Wi-Fi first";
            status_error_ = true;
            if (current_page_ == PageId::Settings) {
                updateSettingsStatusCanvas(UPDATE_MODE_GL16);
            }
            return false;
        }
        const uint32_t connect_started_ms = millis();
        if (!connectivity_.ensureConnected(settings_.ssid, settings_.password)) {
            Serial.printf("[perf] trySyncTime ensureConnected failed after=%lu\n",
                          millis() - connect_started_ms);
            status_message_ = "Wi-Fi connect failed";
            status_error_ = true;
            if (current_page_ == PageId::Settings) {
                updateSettingsStatusCanvas(UPDATE_MODE_GL16);
            }
            return false;
        }
        Serial.printf("[perf] trySyncTime ensureConnected ok after=%lu\n",
                      millis() - connect_started_ms);
    }

    if (!connectivity_.isConnected()) {
        status_message_ = "Wi-Fi is offline";
        status_error_ = true;
        if (current_page_ == PageId::Settings) {
            updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        }
        return false;
    }

    const uint32_t sync_started_ms = millis();
    if (connectivity_.syncTimeToRtc(settings_.timezone)) {
        Serial.printf("[perf] trySyncTime syncTimeToRtc ok after=%lu\n",
                      millis() - sync_started_ms);
        settings_.time_synced = true;
        store_.saveTimeSynced(true);
        status_message_ = "Time sync successful";
        status_error_ = false;
        M5.RTC.getTime(&last_time_);
        M5.RTC.getDate(&last_date_);
    } else {
        Serial.printf("[perf] trySyncTime syncTimeToRtc failed after=%lu\n",
                      millis() - sync_started_ms);
        status_message_ = "Time sync failed";
        status_error_ = true;
    }

    if (current_page_ == PageId::Settings) {
        const uint32_t status_started_ms = millis();
        updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        Serial.printf("[perf] trySyncTime settings status update took=%lu\n",
                      millis() - status_started_ms);
    }
    Serial.printf("[perf] trySyncTime done success=%d total=%lu\n",
                  status_error_ ? 0 : 1, millis() - started_ms);
    return !status_error_;
}

void ClockApp::connectSelectedNetwork() {
    cancelBackgroundConnectivity();
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

void ClockApp::populateSerialStatus(JsonObject data) const {
    data["deviceName"] = "M5Paper Ink Clock";
    data["protocolVersion"] = 1;
    data["page"] = currentPageName();
    data["clockStyle"] = clockStyleName();
    data["savedSsid"] = settings_.ssid;
    data["wifiConnected"] = connectivity_.isConnected();
    data["currentSsid"] = connectivity_.currentSsid();
    data["ipAddress"] = currentIpAddress();
    data["timezone"] = settings_.timezone;
    data["timeSynced"] = settings_.time_synced;
    data["marketSymbol"] = settings_.market_symbol;
    data["marketCode"] = currentMarketCode();
    data["marketDisplayName"] = currentMarketDisplayName();
    data["rtc"] = formatRtcTimestamp();
    data["batteryPercent"] = batteryPercentage();
    data["comfortTemperatureMin"] = settings_.comfort_settings.min_temperature;
    data["comfortTemperatureMax"] = settings_.comfort_settings.max_temperature;
    data["comfortHumidityMin"] = settings_.comfort_settings.min_humidity;
    data["comfortHumidityMax"] = settings_.comfort_settings.max_humidity;
    data["statusMessage"] = status_message_;
    data["statusError"] = status_error_;
}

void ClockApp::sendConfigDoc(const JsonDocument& doc,
                             ConfigTransport transport) const {
    String line = "@cfg:";
    serializeJson(doc, line);
    line += "\n";
    sendConfigLine(line, transport);
}

void ClockApp::sendConfigLine(const String& line,
                              ConfigTransport transport) const {
    if (transport == ConfigTransport::Serial) {
        Serial.print(line);
        return;
    }

    if (!ble_config_client_connected_ || !ble_config_tx_characteristic_) {
        return;
    }

    for (uint32_t offset = 0; offset < line.length();
         offset += kBleNotifyChunkSize) {
        const String chunk = line.substring(
            offset,
            std::min<uint32_t>(offset + kBleNotifyChunkSize, line.length()));
        std::vector<uint8_t> chunk_bytes(chunk.begin(), chunk.end());
        ble_config_tx_characteristic_->setValue(chunk_bytes.data(),
                                                chunk_bytes.size());
        ble_config_tx_characteristic_->notify();
        delay(4);
    }
}

const char* ClockApp::currentPageName() const {
    switch (current_page_) {
        case PageId::Settings:
            return "settings";
        case PageId::WifiScan:
            return "wifi_scan";
        case PageId::Password:
            return "password";
        case PageId::Clock:
        default:
            return "clock";
    }
}

const char* ClockApp::clockStyleName() const {
    return clock_style_ == ClockStyle::Dashboard ? "dashboard" : "classic";
}

String ClockApp::currentIpAddress() const {
    return connectivity_.isConnected() ? WiFi.localIP().toString() : String("");
}

String ClockApp::currentMarketCode() const {
    return marketCodeFromRequestSymbol(settings_.market_symbol);
}

String ClockApp::currentMarketDisplayName() const {
    if (!settings_.market_name.isEmpty()) {
        return settings_.market_name;
    }
    if (!market_quote_.display_name.isEmpty()) {
        return market_quote_.display_name;
    }
    return currentMarketCode();
}

void ClockApp::seedMarketQuoteFromSettings() {
    market_quote_ = MarketQuote {};
    market_quote_.request_symbol = settings_.market_symbol.isEmpty()
                                       ? String("sh000001")
                                       : settings_.market_symbol;
    market_quote_.symbol =
        marketCodeFromRequestSymbol(market_quote_.request_symbol);
    if (!settings_.market_name.isEmpty()) {
        market_quote_.display_name = settings_.market_name;
        return;
    }

    const std::string fallback_name =
        logic::KnownMarketDisplayName(std::string(market_quote_.symbol.c_str()));
    market_quote_.display_name = fallback_name.empty()
                                     ? market_quote_.symbol
                                     : String(fallback_name.c_str());
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
