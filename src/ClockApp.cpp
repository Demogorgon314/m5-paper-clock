#include "ClockApp.h"

#include <WiFi.h>

#include "logic/HolidayLogic.h"
#include "logic/UiLogic.h"
#include "resources/holiday_countdown_bitmaps.h"
#include "resources/wifi_bitmaps.h"
#include "resources/weekday_bitmaps.h"

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
constexpr int16_t kDateW = 640;
constexpr int16_t kDateH = 44;
constexpr int16_t kDateWeekdayGap = 24;
constexpr int16_t kDateCountdownGap = 18;
constexpr int16_t kDateCountdownNumberGap = 4;
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
constexpr float kComfortMinTemperature = 19.0f;
constexpr float kComfortMaxTemperature = 27.0f;
constexpr float kComfortMinHumidity = 20.0f;
constexpr float kComfortMaxHumidity = 85.0f;
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
constexpr int16_t kDashboardClimateX = 430;
constexpr int16_t kDashboardClimateY = 392;
constexpr int16_t kDashboardClimateW = 452;
constexpr int16_t kDashboardClimateH = 86;
constexpr int16_t kDashboardClimateHumidityDividerX = 126;
constexpr int16_t kDashboardClimateTemperatureDividerX = 286;
constexpr int16_t kDashboardWeekdayCropX = 36;
constexpr int16_t kDashboardWeekdayCropW = 28;
constexpr uint8_t kCalendarPastFill = 4;
constexpr uint8_t kCalendarTodayFill = 15;

struct PartialRegion {
    int16_t update_x = 0;
    int16_t update_y = 0;
    int16_t draw_x = 0;
    int16_t draw_y = 0;
};

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

String formatHolidayCountdownSignature(const logic::HolidayCountdown& countdown) {
    if (!countdown.valid) {
        return String("");
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "%u:%d",
             static_cast<unsigned>(countdown.id), countdown.days_remaining);
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
        return String("valid|") + quote.symbol + "|" + quote.price + "|" +
               quote.change + "|" + quote.change_percent + "|" +
               (quote.positive ? "1" : "0");
    }

    const String status_detail =
        !quote.error_message.isEmpty()
            ? quote.error_message
            : (wifi_connected ? "Data unavailable" : "Wi-Fi offline");
    return String("invalid|") + quote.symbol + "|" + status_detail;
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

uint8_t bitmapPixel4(const uint8_t* data, uint16_t width, int16_t x, int16_t y) {
    const uint16_t row_bytes = (width + 1) / 2;
    const uint8_t value = data[static_cast<size_t>(y) * row_bytes + (x / 2)];
    return (x % 2 == 0) ? ((value >> 4) & 0x0F) : (value & 0x0F);
}

void drawBitmapCrop(M5EPD_Canvas& canvas, const WeekdayBitmap& bitmap,
                    int16_t src_x, int16_t src_y, int16_t crop_w,
                    int16_t crop_h, int16_t dst_x, int16_t dst_y) {
    const int16_t max_w = min<int16_t>(crop_w, bitmap.width - src_x);
    const int16_t max_h = min<int16_t>(crop_h, bitmap.height - src_y);
    for (int16_t y = 0; y < max_h; ++y) {
        for (int16_t x = 0; x < max_w; ++x) {
            const uint8_t pixel =
                bitmapPixel4(bitmap.data, bitmap.width, src_x + x, src_y + y);
            if (pixel > 0) {
                canvas.drawPixel(dst_x + x, dst_y + y, pixel);
            }
        }
    }
}

void drawHolidayCountdown(M5EPD_Canvas& canvas, int16_t start_x,
                          const logic::HolidayCountdown& countdown) {
    if (!countdown.valid || countdown.id == logic::HolidayId::None) {
        return;
    }

    const HolidayCountdownBitmap& label =
        holidayCountdownLabelBitmap(countdown.id);
    const HolidayCountdownBitmap& suffix = holidayCountdownDaySuffixBitmap();
    const int16_t label_y = (kDateH - label.height) / 2;
    canvas.pushImage(start_x, label_y, label.width, label.height, label.data);

    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    canvas.setTextSize(2);
    const String days_text = String(countdown.days_remaining);
    const int16_t number_x = start_x + label.width + 6;
    const int16_t number_y = kDateH / 2;
    canvas.drawString(days_text, number_x, number_y);

    const int16_t days_width = canvas.textWidth(days_text);
    const int16_t suffix_x = number_x + days_width + kDateCountdownNumberGap;
    const int16_t suffix_y = (kDateH - suffix.height) / 2;
    canvas.pushImage(suffix_x, suffix_y, suffix.width, suffix.height,
                     suffix.data);
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
                      uint8_t edge_color) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    renderer.drawText(canvas, kHumidityDrawX, kInfoDigitY, humidity_text,
                      kSmallDigitWidth, kSmallDigitHeight, kSmallGap,
                      body_color, edge_color);
    canvas.setTextColor(body_color);
    canvas.setTextSize(3);
    canvas.drawString("%", kHumidityUnitDrawX, kHumidityUnitY);
}

void drawTemperatureInfo(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                         const String& temperature_text, uint8_t body_color,
                         uint8_t edge_color) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    const String temperature_display =
        temperature_text.substring(0, 2) + "." + temperature_text.substring(2);
    renderer.drawText(canvas, kTemperatureDrawX, kInfoDigitY, temperature_display,
                      kSmallDigitWidth, kSmallDigitHeight, kSmallGap,
                      body_color, edge_color);
    canvas.setTextColor(body_color);
    canvas.setTextSize(3);
    canvas.drawString(" C", kTemperatureUnitDrawX, kTemperatureUnitY);
    canvas.setTextSize(2);
    canvas.drawString("o", kTemperatureDegreeDrawX, kTemperatureDegreeY);
}

bool isComfortable(const EnvironmentReading& reading) {
    if (!reading.valid) {
        return false;
    }

    return reading.temperature >= kComfortMinTemperature &&
           reading.temperature <= kComfortMaxTemperature &&
           reading.humidity >= kComfortMinHumidity &&
           reading.humidity <= kComfortMaxHumidity;
}

String comfortFace(const EnvironmentReading& reading) {
    if (!reading.valid) {
        return String("(-_-)");
    }

    if (isComfortable(reading)) {
        return String("(^_^)");
    }
    return String("(-^-)");
}

void drawComfortInfoAt(M5EPD_Canvas& canvas, int16_t center_x, int16_t center_y,
                       const String& face, uint8_t color) {
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
    canvas.setTextSize(6);
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

void drawComfortInfo(M5EPD_Canvas& canvas, const String& face, uint8_t color) {
    canvas.fillCanvas(kWhite);
    drawComfortInfoAt(canvas, kComfortFaceDrawX, kComfortFaceDrawY, face, color);
}

void drawCompactComfortInfoAt(M5EPD_Canvas& canvas, int16_t center_x,
                              int16_t center_y, const String& face,
                              uint8_t color) {
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
    canvas.setTextSize(4);
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

void ClockApp::begin() {
    initializeHardware();
    createCanvases();
    store_.begin();
    settings_ = store_.load();
    settings_.timezone = logic::ClampTimeZone(settings_.timezone);
    clock_style_ = settings_.clock_style == 1 ? ClockStyle::Dashboard
                                              : ClockStyle::Classic;
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
    if (usesDashboardClockStyle()) {
        renderDashboardClockPage(full_refresh);
        return;
    }
    renderClassicClockPage(full_refresh);
}

void ClockApp::renderClassicClockPage(bool full_refresh) {
    (void)full_refresh;
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(UPDATE_MODE_GC16);

    last_time_text_rendered_ = "";
    last_humidity_text_rendered_ = "";
    last_temperature_text_rendered_ = "";
    last_comfort_face_rendered_ = "";
    last_market_summary_rendered_ = "";
    last_date_text_rendered_ = "";
    last_holiday_countdown_rendered_ = "";
    last_weekday_rendered_ = 255;
    last_battery_percentage_ = 255;
    last_wifi_connected_ = false;
    last_wifi_signal_level_ = 255;
    time_digit_partial_counts_.fill(0);
    humidity_digit_partial_counts_.fill(0);
    temperature_digit_partial_counts_.fill(0);
    battery_partial_count_ = 0;

    updateTimeCanvas(true);
    updateBatteryCanvas(true);
    updateInfoCanvas(true);
    updateDateCanvas(true);
    partial_refresh_count_ = 0;
}

void ClockApp::renderDashboardClockPage(bool full_refresh) {
    (void)full_refresh;
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(UPDATE_MODE_GC16);

    last_time_text_rendered_ = "";
    last_humidity_text_rendered_ = "";
    last_temperature_text_rendered_ = "";
    last_comfort_face_rendered_ = "";
    last_market_summary_rendered_ = "";
    last_date_text_rendered_ = "";
    last_holiday_countdown_rendered_ = "";
    last_weekday_rendered_ = 255;
    last_battery_percentage_ = 255;
    last_wifi_connected_ = false;
    last_wifi_signal_level_ = 255;
    time_digit_partial_counts_.fill(0);
    humidity_digit_partial_counts_.fill(0);
    temperature_digit_partial_counts_.fill(0);
    battery_partial_count_ = 0;

    updateDateCanvas(true);
    updateBatteryCanvas(true);
    updateDashboardCalendarCanvas(true);
    updateDashboardTimeCanvas(true);
    updateDashboardSummaryCanvas(true);
    updateDashboardClimateCanvas(true);
    partial_refresh_count_ = 0;
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
    M5.EPD.UpdateArea(kTimeX, kTimeY, time_canvas_.width(),
                      time_canvas_.height(), UPDATE_MODE_GC16);

    if (full_refresh) {
        partial_refresh_count_ = 0;
        time_digit_partial_counts_.fill(0);
    } else {
        ++partial_refresh_count_;
    }
    last_time_text_rendered_ = time_digits;
}

void ClockApp::updateInfoCanvas(bool full_refresh) {
    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = formatHumidityField(last_environment_);
    const String temperature_text = formatTemperatureField(last_environment_);
    const String comfort_face = comfortFace(last_environment_);

    if (full_refresh) {
        info_canvas_.fillCanvas(kWhite);
        info_canvas_.setTextDatum(TL_DATUM);
        renderer_.drawText(info_canvas_, kHumidityDigitXs[0], kInfoDigitY,
                           humidity_text, kSmallDigitWidth, kSmallDigitHeight,
                           kSmallGap, kText, kMutedText);
        info_canvas_.setTextColor(kText);
        info_canvas_.setTextSize(3);
        info_canvas_.drawString("%", kHumidityUnitX, kHumidityUnitY);

        const String temperature_display = temperature_text.substring(0, 2) +
                                           "." + temperature_text.substring(2);
        renderer_.drawText(info_canvas_, kTemperatureDigitXs[0], kInfoDigitY,
                           temperature_display, kSmallDigitWidth,
                           kSmallDigitHeight, kSmallGap, kText, kMutedText);
        info_canvas_.setTextColor(kText);
        info_canvas_.drawString(" C", kTemperatureUnitX, kTemperatureUnitY);
        info_canvas_.setTextSize(2);
        info_canvas_.drawString("o", kTemperatureDegreeX, kTemperatureDegreeY);

        info_canvas_.setTextDatum(CC_DATUM);
        info_canvas_.setTextColor(kText);
        drawComfortInfoAt(info_canvas_, 640, 58, comfort_face, kText);

        info_canvas_.pushCanvas(kInfoX, kInfoY, UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kInfoX, kInfoY, info_canvas_.width(),
                          info_canvas_.height(), UPDATE_MODE_GC16);
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
                         kMutedText);
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
                            kMutedText);
        temperature_canvas_.pushCanvas(kTemperatureCanvasX, kTemperatureCanvasY,
                                       UPDATE_MODE_NONE);
        M5.EPD.UpdateArea(kTemperatureCanvasX, kTemperatureCanvasY,
                          temperature_canvas_.width(),
                          temperature_canvas_.height(), UPDATE_MODE_GC16);
        ++partial_refresh_count_;
        temperature_digit_partial_counts_.fill(0);
    }
    if (comfort_changed) {
        drawComfortInfo(comfort_canvas_, comfort_face, kText);
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
    const logic::HolidayCountdown countdown = logic::NextHolidayCountdown(
        current_date.year, current_date.mon, current_date.day);
    const String holiday_signature = formatHolidayCountdownSignature(countdown);
    if (date_text == last_date_text_rendered_ &&
        week == last_weekday_rendered_ &&
        holiday_signature == last_holiday_countdown_rendered_) {
        return;
    }

    date_canvas_.fillCanvas(kWhite);
    const WeekdayBitmap& weekday = weekdayBitmap(week);

    date_canvas_.setTextDatum(CL_DATUM);
    date_canvas_.setTextColor(kText);
    date_canvas_.setTextSize(3);
    date_canvas_.drawString(date_text, 0, kDateH / 2);

    const int16_t date_width = date_canvas_.textWidth(date_text);
    const int16_t weekday_x = date_width + kDateWeekdayGap;
    const int16_t weekday_y = (kDateH - weekday.height) / 2;
    if (!date_text.isEmpty() && weekday_x + weekday.width <= kDateW) {
        date_canvas_.pushImage(weekday_x, weekday_y, weekday.width,
                               weekday.height, weekday.data);
    }

    if (countdown.valid) {
        date_canvas_.setTextSize(2);
        const HolidayCountdownBitmap& label =
            holidayCountdownLabelBitmap(countdown.id);
        const HolidayCountdownBitmap& suffix =
            holidayCountdownDaySuffixBitmap();
        const String days_text = String(countdown.days_remaining);
        const int16_t countdown_width = label.width + 6 +
                                        date_canvas_.textWidth(days_text) +
                                        kDateCountdownNumberGap + suffix.width;
        const int16_t countdown_x = weekday_x + weekday.width + kDateCountdownGap;
        if (countdown_x + countdown_width <= kDateW) {
            drawHolidayCountdown(date_canvas_, countdown_x, countdown);
        }
    }
    date_canvas_.pushCanvas(kDateX, kDateY, UPDATE_MODE_NONE);

    if (full_refresh) {
        M5.EPD.UpdateArea(kDateX, kDateY, kDateW, kDateH, UPDATE_MODE_GC16);
    } else {
        M5.EPD.UpdateArea(kDateX, kDateY, kDateW, kDateH, UPDATE_MODE_GC16);
        ++partial_refresh_count_;
    }
    last_date_text_rendered_ = date_text;
    last_holiday_countdown_rendered_ = holiday_signature;
    last_weekday_rendered_ = week;
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
    battery_canvas_.setTextSize(2);
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
        M5.EPD.UpdateArea(kBatteryX, kBatteryY, kBatteryW, kBatteryH,
                          UPDATE_MODE_GC16);
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

    dashboard_calendar_canvas_.fillCanvas(kWhite);
    dashboard_calendar_canvas_.setTextColor(kText);
    dashboard_calendar_canvas_.setTextDatum(CC_DATUM);
    dashboard_calendar_canvas_.setTextSize(5);
    dashboard_calendar_canvas_.drawString(formatDashboardMonth(current_date),
                                          kDashboardCalendarW / 2, 24);

    const int16_t cell_w = 42;
    const int16_t cell_h = 24;
    const int16_t grid_w = cell_w * 7;
    const int16_t start_x = (kDashboardCalendarW - grid_w) / 2;
    const int16_t header_y = 54;
    const int16_t grid_y = 86;

    for (int col = 0; col < 7; ++col) {
        const WeekdayBitmap& weekday = weekdayBitmap(static_cast<uint8_t>(col));
        const int16_t dst_x = start_x + col * cell_w +
                              (cell_w - kDashboardWeekdayCropW) / 2;
        drawBitmapCrop(dashboard_calendar_canvas_, weekday,
                       kDashboardWeekdayCropX, 0, kDashboardWeekdayCropW,
                       weekday.height, dst_x, header_y);
    }

    rtc_date_t first_day = current_date;
    first_day.day = 1;
    const uint8_t first_weekday = calculateWeekday(first_day);
    const uint8_t month_days = daysInMonth(current_date.year, current_date.mon);

    dashboard_calendar_canvas_.setTextSize(2);
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
    M5.EPD.UpdateArea(kDashboardCalendarX, kDashboardCalendarY,
                      kDashboardCalendarW, kDashboardCalendarH,
                      UPDATE_MODE_GC16);
    if (!full_refresh) {
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
    M5.EPD.UpdateArea(kDashboardTimeX, kDashboardTimeY, kDashboardTimeW,
                      kDashboardTimeH, UPDATE_MODE_GC16);
    if (!full_refresh) {
        ++partial_refresh_count_;
    }
    last_time_text_rendered_ = time_digits;
}

void ClockApp::updateDashboardSummaryCanvas(bool full_refresh) {
    refreshMarketQuote(full_refresh);
    const bool wifi_connected = connectivity_.isConnected();
    const String summary_signature =
        marketSummarySignature(market_quote_, wifi_connected);
    if (!full_refresh && summary_signature == last_market_summary_rendered_) {
        return;
    }

    dashboard_summary_canvas_.fillCanvas(kWhite);
    dashboard_summary_canvas_.drawRoundRect(0, 0, kDashboardSummaryW,
                                            kDashboardSummaryH, 6, kBorder);
    const int16_t header_y = 10;
    const int16_t price_y = 32;
    const int16_t baseline_y = 64;

    dashboard_summary_canvas_.setTextDatum(TL_DATUM);
    dashboard_summary_canvas_.setTextColor(kMutedText);
    dashboard_summary_canvas_.setTextSize(2);
    dashboard_summary_canvas_.drawString(market_quote_.symbol,
                                         16, header_y);

    if (market_quote_.valid) {
        dashboard_summary_canvas_.setTextDatum(TR_DATUM);
        dashboard_summary_canvas_.setTextSize(3);
        dashboard_summary_canvas_.setTextColor(kText);
        dashboard_summary_canvas_.drawString(market_quote_.price,
                                             kDashboardSummaryW - 16, price_y);

        const String change_prefix = market_quote_.positive ? "+" : "";
        const String change_value = change_prefix + market_quote_.change;
        const String percent_value =
            change_prefix + market_quote_.change_percent + "%";
        dashboard_summary_canvas_.setTextSize(2);
        dashboard_summary_canvas_.setTextColor(kText);
        dashboard_summary_canvas_.setTextDatum(TR_DATUM);
        const int16_t percent_right = kDashboardSummaryW - 16;
        const int16_t percent_width =
            dashboard_summary_canvas_.textWidth(percent_value);
        const int16_t change_right = percent_right - percent_width - 18;
        const int16_t change_width =
            dashboard_summary_canvas_.textWidth(change_value);
        const int16_t change_left = change_right - change_width;
        const int16_t arrow_center_x = change_left - 14;
        const int16_t arrow_center_y = baseline_y + 5;
        if (market_quote_.positive) {
            dashboard_summary_canvas_.fillTriangle(arrow_center_x - 8,
                                                   arrow_center_y + 6,
                                                   arrow_center_x,
                                                   arrow_center_y - 6,
                                                   arrow_center_x + 8,
                                                   arrow_center_y + 6, kText);
        } else {
            dashboard_summary_canvas_.fillTriangle(arrow_center_x - 8,
                                                   arrow_center_y - 6,
                                                   arrow_center_x,
                                                   arrow_center_y + 6,
                                                   arrow_center_x + 8,
                                                   arrow_center_y - 6, kText);
        }
        dashboard_summary_canvas_.drawString(change_value, change_right,
                                             baseline_y);
        dashboard_summary_canvas_.drawString(percent_value, percent_right,
                                             baseline_y);
    } else {
        dashboard_summary_canvas_.setTextDatum(TL_DATUM);
        dashboard_summary_canvas_.setTextSize(3);
        dashboard_summary_canvas_.setTextColor(kText);
        dashboard_summary_canvas_.drawString("No quote", 16, 34);
        dashboard_summary_canvas_.setTextSize(2);
        dashboard_summary_canvas_.setTextColor(kMutedText);
        const String status_detail =
            !market_quote_.error_message.isEmpty()
                ? market_quote_.error_message
                : (wifi_connected ? "Data unavailable" : "Wi-Fi offline");
        dashboard_summary_canvas_.drawString(status_detail, 16, 62);
    }

    dashboard_summary_canvas_.pushCanvas(kDashboardSummaryX, kDashboardSummaryY,
                                         UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(kDashboardSummaryX, kDashboardSummaryY,
                      kDashboardSummaryW, kDashboardSummaryH,
                      UPDATE_MODE_GC16);
    if (!full_refresh) {
        ++partial_refresh_count_;
    }
    last_market_summary_rendered_ = summary_signature;
}

void ClockApp::updateDashboardClimateCanvas(bool full_refresh) {
    last_environment_ = sensor_.read();
    last_sensor_read_ms_ = millis();

    const String humidity_text = formatDashboardHumidity(last_environment_);
    const String temperature_text = formatDashboardTemperature(last_environment_);
    const String comfort_face = comfortFace(last_environment_);
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
    dashboard_climate_canvas_.setTextSize(5);
    dashboard_climate_canvas_.drawString(humidity_text, humidity_x,
                                         climate_value_y);
    const int16_t humidity_width = dashboard_climate_canvas_.textWidth(humidity_text);
    dashboard_climate_canvas_.setTextSize(2);
    dashboard_climate_canvas_.drawString("%", humidity_x + humidity_width + 6,
                                         climate_value_y + 14);

    dashboard_climate_canvas_.setTextSize(5);
    dashboard_climate_canvas_.drawString(temperature_text, temperature_x,
                                         climate_value_y);
    const int16_t temperature_width =
        dashboard_climate_canvas_.textWidth(temperature_text);
    const int16_t unit_x = temperature_x + temperature_width + 4;
    dashboard_climate_canvas_.setTextSize(2);
    dashboard_climate_canvas_.drawString("o", unit_x, climate_value_y - 8);
    dashboard_climate_canvas_.drawString("C", unit_x + 14,
                                         climate_value_y + 14);

    const int16_t face_cell_center_x =
        kDashboardClimateTemperatureDividerX +
        ((kDashboardClimateW - kDashboardClimateTemperatureDividerX) / 2);
    drawCompactComfortInfoAt(dashboard_climate_canvas_, face_cell_center_x,
                             kDashboardClimateH / 2 + 2, comfort_face, kText);

    dashboard_climate_canvas_.pushCanvas(kDashboardClimateX, kDashboardClimateY,
                                         UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(kDashboardClimateX, kDashboardClimateY,
                      kDashboardClimateW, kDashboardClimateH,
                      UPDATE_MODE_GC16);
    if (!full_refresh) {
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

    last_market_fetch_ms_ = millis();
    if (!connectivity_.isConnected()) {
        return market_quote_.valid;
    }

    const MarketQuote fetched = market_.fetchShanghaiIndex();
    if (!fetched.valid) {
        if (!market_quote_.valid) {
            market_quote_ = fetched;
        }
        return market_quote_.valid;
    }

    market_quote_ = fetched;
    return true;
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
