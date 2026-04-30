#include "ClockApp.h"

#include <algorithm>
#include <array>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include <memory>

#include "logic/ComfortLogic.h"
#include "logic/ClimateDisplayLogic.h"
#include "logic/ComponentUpdateGroups.h"
#include "logic/DateDisplayLogic.h"
#include "logic/HolidayLogic.h"
#include "logic/MarketLogic.h"
#include "logic/UiLogic.h"
#include "resources/wifi_bitmaps.h"

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

#ifndef FIRMWARE_GIT_SHA
#define FIRMWARE_GIT_SHA "unknown"
#endif

#ifndef FIRMWARE_BUILD_TIME
#define FIRMWARE_BUILD_TIME "unknown"
#endif

#ifndef M5_CLOCK_ENABLE_PERF_LOGS
#define M5_CLOCK_ENABLE_PERF_LOGS 0
#endif

namespace {

constexpr bool kPerfLogs = M5_CLOCK_ENABLE_PERF_LOGS != 0;

template <typename... Args>
void logPerf(const char* format, Args... args) {
    if (kPerfLogs) {
        Serial.printf(format, args...);
    }
}

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
constexpr uint32_t kSerialConfigActiveMs = 60000;
constexpr uint32_t kBlePairingCodeTtlMs = 60000;
constexpr uint8_t kBlePairingMaxAttempts = 5;
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
constexpr int16_t kDateDateAreaX = 0;
constexpr int16_t kDateDateAreaW = alignUpTo4(kDateCjkWeekdayX + 8);
constexpr int16_t kDateWeekdayAreaX = alignDownTo4(kDateCjkWeekdayX - 8);
constexpr int16_t kDateWeekdayAreaW =
    alignUpTo4((kDateCjkHolidayX + 8) - kDateWeekdayAreaX);
constexpr int16_t kDateHolidayAreaX = alignDownTo4(kDateCjkHolidayX - 8);
constexpr int16_t kDateHolidayAreaW = kDateW - kDateHolidayAreaX;
constexpr int16_t kBatteryX = 768;
constexpr int16_t kBatteryY = 20;
constexpr int16_t kBatteryW = 176;
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
constexpr int16_t kDashboardCalendarW = 304;
constexpr int16_t kDashboardCalendarH = 232;
constexpr int16_t kDashboardCalendarCellW = 42;
constexpr int16_t kDashboardCalendarCellH = 24;
constexpr int16_t kDashboardCalendarGridW = kDashboardCalendarCellW * 7;
constexpr int16_t kDashboardCalendarStartX =
    (kDashboardCalendarW - kDashboardCalendarGridW) / 2;
constexpr int16_t kDashboardCalendarHeaderY = 54;
constexpr int16_t kDashboardCalendarGridY = 86;
constexpr int16_t kDashboardTimeW = 472;
constexpr int16_t kDashboardTimeH = 236;
constexpr int16_t kDashboardTimeDigitW = 92;
constexpr int16_t kDashboardTimeDigitH = 210;
constexpr int16_t kDashboardTimeGap = 18;
constexpr int16_t kDashboardTimeColonW = kDashboardTimeDigitW / 3;
constexpr int16_t kDashboardTimeTextW =
    (kDashboardTimeDigitW * 4) + kDashboardTimeColonW +
    (kDashboardTimeGap * 4);
constexpr int16_t kDashboardTimeDrawX =
    kDashboardTimeW > kDashboardTimeTextW
        ? ((kDashboardTimeW - kDashboardTimeTextW) / 2)
        : 0;
constexpr int16_t kDashboardTimeDigitY =
    (kDashboardTimeH - kDashboardTimeDigitH) / 2;
constexpr int16_t kDashboardMinuteLocalX =
    kDashboardTimeDrawX + ((kDashboardTimeDigitW + kDashboardTimeGap) * 2);
constexpr int16_t kDashboardMinuteContentW =
    kDashboardTimeColonW + kDashboardTimeGap + kDashboardTimeDigitW +
    kDashboardTimeGap + kDashboardTimeDigitW;
constexpr int16_t kDashboardMinuteCanvasW =
    alignUpTo4(kDashboardMinuteContentW + 3);
constexpr int16_t kDashboardMinuteCanvasH = kDashboardTimeH;
constexpr int16_t kDashboardSummaryW = 332;
constexpr int16_t kDashboardSummaryH = 86;
constexpr int16_t kDashboardSummaryTitleX = 16;
constexpr int16_t kDashboardSummaryTitleY = 9;
constexpr int16_t kDashboardSummaryPriceRight = kDashboardSummaryW - 14;
constexpr int16_t kDashboardSummaryPriceY = 10;
constexpr int16_t kDashboardSummaryBottomY = 58;
constexpr int16_t kDashboardSummaryArrowGap = 12;
constexpr int16_t kDashboardSummaryValueGap = 14;
constexpr int16_t kDashboardSummaryTitleAreaX = 8;
constexpr int16_t kDashboardSummaryTitleAreaY = 4;
constexpr int16_t kDashboardSummaryTitleAreaW = alignUpTo4(180);
constexpr int16_t kDashboardSummaryTitleAreaH = 34;
constexpr int16_t kDashboardSummaryPriceAreaX = alignDownTo4(124);
constexpr int16_t kDashboardSummaryPriceAreaY = 4;
constexpr int16_t kDashboardSummaryPriceAreaW =
    kDashboardSummaryW - kDashboardSummaryPriceAreaX - 8;
constexpr int16_t kDashboardSummaryPriceAreaH = 52;
constexpr int16_t kDashboardSummaryBottomAreaX = 8;
constexpr int16_t kDashboardSummaryBottomAreaY = 52;
constexpr int16_t kDashboardSummaryBottomAreaW = kDashboardSummaryW - 16;
constexpr int16_t kDashboardSummaryBottomAreaH = 26;
constexpr int16_t kDashboardClimateW = 456;
constexpr int16_t kDashboardClimateH = 86;
constexpr int16_t kDashboardClimateHumidityDividerX = 126;
constexpr int16_t kDashboardClimateTemperatureDividerX = 286;
constexpr int16_t kDashboardClimateContentY = 8;
constexpr int16_t kDashboardClimateContentH = 70;
constexpr int16_t kDashboardClimateHumidityAreaX = 8;
constexpr int16_t kDashboardClimateHumidityAreaW = 112;
constexpr int16_t kDashboardClimateTemperatureAreaX =
    alignUpTo4(kDashboardClimateHumidityDividerX + 4);
constexpr int16_t kDashboardClimateTemperatureAreaW = 148;
constexpr int16_t kDashboardClimateComfortAreaX =
    alignUpTo4(kDashboardClimateTemperatureDividerX + 4);
constexpr int16_t kDashboardClimateComfortAreaW = 156;
constexpr uint8_t kCalendarPastFill = 4;
constexpr uint8_t kCalendarTodayFill = 15;

struct PartialRegion {
    int16_t update_x = 0;
    int16_t update_y = 0;
    int16_t draw_x = 0;
    int16_t draw_y = 0;
    int16_t update_w = 0;
    int16_t update_h = 0;
};

struct DirtyRect {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
    bool valid = false;
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

String joinDateTextParts(const String& first, const String& second,
                         const String& third) {
    String output;
    const String parts[] = {first, second, third};
    for (const String& part : parts) {
        if (part.isEmpty()) {
            continue;
        }
        if (!output.isEmpty()) {
            output += "  ";
        }
        output += part;
    }
    return output;
}

uint8_t fitDateTextSize(M5EPD_Canvas& canvas, const String& text,
                        uint8_t preferred_size, uint8_t minimum_size,
                        int16_t max_width) {
    constexpr uint8_t kDateTextSizeCandidates[] = {7, 3, 2};
    for (uint8_t size : kDateTextSizeCandidates) {
        if (size > preferred_size || size < minimum_size) {
            continue;
        }
        setCanvasTextSize(canvas, true, size);
        if (canvas.textWidth(text) + 1 <= max_width) {
            return size;
        }
    }
    setCanvasTextSize(canvas, true, minimum_size);
    return minimum_size;
}

String formatTwoDigits(int value) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d", value);
    return String(buf);
}

String padLeft(const String& value, size_t width) {
    return String(logic::PadLeft(std::string(value.c_str()), width).c_str());
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
    return String(logic::FormatTemperature(value).c_str());
}

String formatHumidity(float value) {
    return String(logic::FormatHumidity(value).c_str());
}

String formatHumidityField(const EnvironmentReading& reading) {
    return String(logic::FormatClassicHumidityField(reading.humidity,
                                                    reading.valid).c_str());
}

String formatTemperatureField(const EnvironmentReading& reading) {
    return String(logic::FormatClassicTemperatureField(reading.temperature,
                                                       reading.valid).c_str());
}

String formatDateOnly(const rtc_date_t& date) {
    if (date.year < 2020) {
        return String("");
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", date.year, date.mon, date.day);
    return String(buf);
}

String normalizeDateFormat(const String& value) {
    return String(logic::NormalizeDateFormat(value.c_str()));
}

String normalizeWeekdayFormat(const String& value) {
    return String(logic::NormalizeWeekdayFormat(value.c_str()));
}

String normalizeDateLayout(const String& value) {
    return String(logic::NormalizeDateLayout(value.c_str()));
}

uint8_t normalizeDateTextSize(uint8_t value) {
    return logic::NormalizeDateTextSize(value);
}

String formatConfiguredDate(const rtc_date_t& date, const String& format) {
    if (date.year < 2020) {
        return String("");
    }

    const String normalized = normalizeDateFormat(format);
    char buf[32];
    if (normalized == "yyyy/mm/dd") {
        snprintf(buf, sizeof(buf), "%04d/%02d/%02d", date.year, date.mon,
                 date.day);
    } else if (normalized == "mm-dd") {
        snprintf(buf, sizeof(buf), "%02d-%02d", date.mon, date.day);
    } else if (normalized == "yyyy年m月d日") {
        snprintf(buf, sizeof(buf), "%04d年%d月%d日", date.year, date.mon,
                 date.day);
    } else {
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", date.year, date.mon,
                 date.day);
    }
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

String bytesToHex(const uint8_t* bytes, size_t length) {
    static const char* hex = "0123456789abcdef";
    String output;
    output.reserve(length * 2);
    for (size_t index = 0; index < length; ++index) {
        output += hex[bytes[index] >> 4];
        output += hex[bytes[index] & 0x0f];
    }
    return output;
}

String normalizedSha256(String value) {
    value.trim();
    value.toLowerCase();
    return value;
}

String formatHolidayDisplayText(const logic::HolidayDisplay& display,
                                bool show_progress) {
    if (!display.valid() || display.id == logic::HolidayId::None) {
        return String("");
    }

    switch (display.state) {
        case logic::HolidayDisplayState::Countdown:
            return String(u8"距 ") + String(logic::HolidayNameZh(display.id)) +
                   String(u8" 还有 ") + String(display.days_remaining) +
                   String(u8" 天");
        case logic::HolidayDisplayState::InHoliday: {
            String text = String(logic::HolidayNameZh(display.id)) +
                          String(u8"假期中");
            if (show_progress && display.holiday_day_index > 0 &&
                display.holiday_total_days > 0) {
                text += " " + String(display.holiday_day_index) + "/" +
                        String(display.holiday_total_days);
            }
            return text;
        }
        case logic::HolidayDisplayState::LastDay:
            return String(logic::HolidayNameZh(display.id)) +
                   String(u8"最后一天");
        case logic::HolidayDisplayState::None:
        default:
            return String("");
    }
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

String weekdayLabel(uint8_t week, const String& format) {
    const uint8_t normalized_week = week % 7;
    const String normalized = normalizeWeekdayFormat(format);
    if (normalized == "long") {
        switch (normalized_week) {
            case 0:
                return String(u8"星期日");
            case 1:
                return String(u8"星期一");
            case 2:
                return String(u8"星期二");
            case 3:
                return String(u8"星期三");
            case 4:
                return String(u8"星期四");
            case 5:
                return String(u8"星期五");
            case 6:
            default:
                return String(u8"星期六");
        }
    }
    if (normalized == "narrow") {
        switch (normalized_week) {
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
    if (normalized == "english-short") {
        static constexpr const char* kLabels[] = {"Sun", "Mon", "Tue", "Wed",
                                                  "Thu", "Fri", "Sat"};
        return String(kLabels[normalized_week]);
    }
    return weekdayLabel(normalized_week);
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
    return logic::WeekdayFromCivil(date.year, date.mon, date.day);
}

uint8_t daysInMonth(int year, int month) {
    return logic::DaysInMonth(year, month);
}

DirtyRect dashboardCalendarCellRect(const rtc_date_t& date) {
    const uint8_t month_days = daysInMonth(date.year, date.mon);
    if (date.day < 1 || date.day > month_days) {
        return {};
    }

    rtc_date_t first_day = date;
    first_day.day = 1;
    const uint8_t first_weekday = calculateWeekday(first_day);
    const int index = first_weekday + date.day - 1;
    const int row = index / 7;
    const int col = index % 7;
    DirtyRect rect;
    rect.x = static_cast<int16_t>(kDashboardCalendarStartX +
                                  col * kDashboardCalendarCellW);
    rect.y = static_cast<int16_t>(kDashboardCalendarGridY +
                                  row * kDashboardCalendarCellH);
    rect.w = kDashboardCalendarCellW - 2;
    rect.h = kDashboardCalendarCellH - 2;
    rect.valid = true;
    return rect;
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
    return String(logic::MarketQuoteTimeLabel(
        std::string(quote.updated_at.c_str())).c_str());
}

String marketStatusLabel(const MarketQuote& quote, bool market_open,
                         bool use_cjk_font) {
    return String(logic::MarketStatusLabel(
        std::string(quote.updated_at.c_str()), market_open,
        use_cjk_font).c_str());
}

String marketDisplayLabel(const MarketQuote& quote, bool prefer_display_name) {
    return String(logic::MarketDisplayLabel(
        std::string(quote.symbol.c_str()),
        std::string(quote.display_name.c_str()),
        prefer_display_name).c_str());
}

String marketCodeFromRequestSymbol(const String& request_symbol) {
    return String(logic::MarketCodeFromRequestSymbol(
        std::string(request_symbol.c_str())).c_str());
}

bool marketSearchMatchesQuery(const MarketSearchResult& result,
                              const String& query) {
    return logic::MarketSearchMatchesQuery(
        std::string(result.request_symbol.c_str()),
        std::string(result.code.c_str()), std::string(query.c_str()));
}

String formatDashboardHumidity(const EnvironmentReading& reading) {
    return String(logic::FormatDashboardHumidity(reading.humidity,
                                                 reading.valid).c_str());
}

String formatDashboardTemperature(const EnvironmentReading& reading) {
    return String(logic::FormatDashboardTemperature(reading.temperature,
                                                    reading.valid).c_str());
}

void drawHolidayDisplay(M5EPD_Canvas& canvas, int16_t start_x,
                        int16_t center_y,
                        const logic::HolidayDisplay& display,
                        bool show_progress) {
    if (!display.valid() || display.id == logic::HolidayId::None) {
        return;
    }

    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, true, kDateCjkTextSize);
    const String text = formatHolidayDisplayText(display, show_progress);
    if (!text.isEmpty()) {
        drawFauxBoldString(canvas, text, start_x, center_y);
    }
}

void drawHolidayDisplay(M5EPD_Canvas& canvas, int16_t start_x,
                        const logic::HolidayDisplay& display,
                        bool show_progress) {
    drawHolidayDisplay(canvas, start_x, kDateH / 2, display, show_progress);
}

PartialRegion makePartialRegion(int16_t x, int16_t y, int16_t w, int16_t h) {
    const int16_t update_x = alignDownTo4(x);
    const int16_t update_y = alignDownTo4(y);
    const int16_t update_right = alignUpTo4(x + w);
    const int16_t update_bottom = y + h;
    PartialRegion region;
    region.update_x = update_x;
    region.update_y = update_y;
    region.draw_x = x - update_x;
    region.draw_y = y - update_y;
    region.update_w = update_right - update_x;
    region.update_h = update_bottom - update_y;
    return region;
}

void updateAlignedArea(int16_t x, int16_t y, int16_t w, int16_t h,
                       m5epd_update_mode_t mode) {
    const PartialRegion region = makePartialRegion(x, y, w, h);
    M5.EPD.UpdateArea(region.update_x, region.update_y, region.update_w,
                      region.update_h, mode);
}

void includeDirtyRect(DirtyRect& dirty, const DirtyRect& rect) {
    if (!rect.valid) {
        return;
    }
    if (!dirty.valid) {
        dirty = rect;
        return;
    }

    const int16_t left = min<int16_t>(dirty.x, rect.x);
    const int16_t top = min<int16_t>(dirty.y, rect.y);
    const int16_t right =
        max<int16_t>(static_cast<int16_t>(dirty.x + dirty.w),
                     static_cast<int16_t>(rect.x + rect.w));
    const int16_t bottom =
        max<int16_t>(static_cast<int16_t>(dirty.y + dirty.h),
                     static_cast<int16_t>(rect.y + rect.h));
    dirty.x = left;
    dirty.y = top;
    dirty.w = right - left;
    dirty.h = bottom - top;
    dirty.valid = true;
}

void pushAndRefreshCanvas(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                          m5epd_update_mode_t mode) {
    canvas.pushCanvas(x, y, UPDATE_MODE_NONE);
    updateAlignedArea(x, y, canvas.width(), canvas.height(), mode);
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
                    const String& time_digits, int16_t draw_x,
                    uint8_t body_color, uint8_t edge_color) {
    if (time_digits.length() != 4) {
        return;
    }

    const String minute_text = ":" + time_digits.substring(2);
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextColor(body_color);
    renderer.drawText(canvas, draw_x, kTimeDigitY, minute_text,
                      kTimeDigitWidth, kTimeDigitHeight, kTimeGap, body_color,
                      edge_color);
}

void drawDashboardMinuteTime(const SegmentRenderer& renderer,
                             M5EPD_Canvas& canvas, const String& time_digits,
                             int16_t draw_x,
                             uint8_t body_color, uint8_t edge_color) {
    if (time_digits.length() != 4) {
        return;
    }

    const String minute_text = ":" + time_digits.substring(2);
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextColor(body_color);
    renderer.drawText(canvas, draw_x, kDashboardTimeDigitY, minute_text,
                      kDashboardTimeDigitW, kDashboardTimeDigitH,
                      kDashboardTimeGap, body_color, edge_color);
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
    return static_cast<uint8_t>(logic::WifiSignalLevelFromRssi(rssi));
}

uint8_t wifiBitmapLevel(uint8_t signal_level) {
    return static_cast<uint8_t>(logic::WifiBitmapLevel(signal_level));
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

void drawThickLine(M5EPD_Canvas& canvas, int16_t x0, int16_t y0, int16_t x1,
                   int16_t y1, uint8_t color) {
    canvas.drawLine(x0, y0, x1, y1, color);
    canvas.drawLine(x0 + 1, y0, x1 + 1, y1, color);
}

void drawUsbStatusIcon(M5EPD_Canvas& canvas, int16_t origin_x, int16_t origin_y) {
    const int16_t cx = origin_x + 9;
    canvas.fillTriangle(cx, origin_y + 1, origin_x + 6, origin_y + 7,
                        origin_x + 12, origin_y + 7, kText);
    canvas.fillRect(cx, origin_y + 7, 2, 11, kText);

    canvas.drawLine(cx, origin_y + 16, origin_x + 5, origin_y + 13, kText);
    canvas.drawLine(origin_x + 5, origin_y + 13, origin_x + 5,
                    origin_y + 11, kText);
    canvas.fillCircle(origin_x + 5, origin_y + 9, 2, kText);

    canvas.drawLine(cx + 1, origin_y + 14, origin_x + 15, origin_y + 10,
                    kText);
    canvas.drawLine(origin_x + 15, origin_y + 10, origin_x + 15,
                    origin_y + 8, kText);
    canvas.fillRect(origin_x + 13, origin_y + 5, 4, 4, kText);

    canvas.fillCircle(cx + 1, origin_y + 20, 2, kText);
}

void drawBluetoothStatusIcon(M5EPD_Canvas& canvas, int16_t origin_x,
                             int16_t origin_y) {
    const int16_t spine_x = origin_x + 8;
    const int16_t top_y = origin_y + 2;
    const int16_t mid_y = origin_y + 12;
    const int16_t bottom_y = origin_y + 22;
    const int16_t arm_x = origin_x + 16;

    drawThickLine(canvas, spine_x, top_y, spine_x, bottom_y, kText);
    drawThickLine(canvas, spine_x, top_y, arm_x, origin_y + 8, kText);
    drawThickLine(canvas, arm_x, origin_y + 8, spine_x, mid_y, kText);
    drawThickLine(canvas, spine_x, mid_y, arm_x, origin_y + 16, kText);
    drawThickLine(canvas, arm_x, origin_y + 16, spine_x, bottom_y, kText);
    drawThickLine(canvas, origin_x + 3, origin_y + 6, spine_x, mid_y, kText);
    drawThickLine(canvas, origin_x + 3, origin_y + 18, spine_x, mid_y, kText);
}

m5epd_update_mode_t nextPartialMode(uint8_t& count, uint8_t clean_interval) {
    const uint8_t interval = static_cast<uint8_t>(
        logic::ClampPartialCleanInterval(clean_interval));
    if (++count >= interval) {
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
        app_->ble_config_conn_id_ = app_->ble_config_server_
                                        ? app_->ble_config_server_->getConnId()
                                        : 0;
        app_->ble_config_client_connected_ = true;
        app_->ble_config_session_authorized_ = false;
        app_->battery_status_dirty_ = true;
        Serial.println("[ble] config client connected");
    }

    void onConnect(BLEServer*, esp_ble_gatts_cb_param_t* param) override {
        if (!app_) {
            return;
        }
        app_->ble_config_conn_id_ = param ? param->connect.conn_id : 0;
        app_->ble_config_client_connected_ = true;
        app_->ble_config_session_authorized_ = false;
        app_->battery_status_dirty_ = true;
        Serial.printf("[ble] config client connected conn=%u\n",
                      app_->ble_config_conn_id_);
    }

    void onDisconnect(BLEServer*) override {
        if (!app_) {
            return;
        }
        app_->ble_config_client_connected_ = false;
        app_->ble_config_session_authorized_ = false;
        app_->ble_config_conn_id_ = 0;
        app_->clearBlePairingPrompt();
        app_->battery_status_dirty_ = true;
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
    settings_.date_format = normalizeDateFormat(settings_.date_format);
    settings_.weekday_format = normalizeWeekdayFormat(settings_.weekday_format);
    settings_.date_layout = normalizeDateLayout(settings_.date_layout);
    settings_.date_text_size = normalizeDateTextSize(settings_.date_text_size);
    settings_.comfort_settings =
        logic::NormalizeComfortSettings(settings_.comfort_settings);
    if (settings_.active_layout_id.isEmpty()) {
        settings_.active_layout_id = logic::kDefaultLayoutId;
    }
    if (settings_.layout_presets.empty()) {
        SavedDashboardLayout preset;
        preset.id = logic::kDefaultLayoutId;
        preset.name = logic::kDefaultLayoutName;
        preset.dashboard_layout = settings_.dashboard_layout;
        preset.market_layout = settings_.market_layout;
        settings_.layout_presets.push_back(preset);
    }
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

    if (current_page_ == PageId::Settings && !local_ota_active_ &&
        activeConfigConnectionIcon() == ConfigConnectionIcon::None) {
        autoConnectIfNeeded();
    }

    if (ble_pairing_prompt_visible_ && !blePairingCodeActive()) {
        clearBlePairingPrompt();
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
        if (battery_status_dirty_) {
            updateBatteryCanvas(false);
        }
        updateClockPage();
    }
}

void ClockApp::initializeHardware() {
    Serial.setRxBufferSize(16384);
    Serial.begin(1500000);
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
    dashboard_minute_canvas_.createCanvas(kDashboardMinuteCanvasW,
                                          kDashboardMinuteCanvasH);
    dashboard_minute_canvas_.useFreetypeFont(false);
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
    if (usesDashboardLayout()) {
        renderDashboardClockPage(full_refresh);
        return;
    }
    renderClassicClockPage(full_refresh);
}

void ClockApp::renderClassicClockPage(bool full_refresh) {
    const uint32_t render_started_ms = millis();
    logPerf("[perf] renderClassicClockPage start full=%d at=%lu\n",
                  full_refresh ? 1 : 0, render_started_ms);
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);

    resetClockRenderState();

    uint32_t step_started_ms = millis();
    updateTimeCanvas(true);
    logPerf("[perf] classic updateTimeCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateBatteryCanvas(true);
    logPerf("[perf] classic updateBatteryCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateInfoCanvas(true);
    logPerf("[perf] classic updateInfoCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateDateCanvas(true);
    logPerf("[perf] classic updateDateCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    const m5epd_update_mode_t mode =
        full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16;
    M5.EPD.UpdateFull(mode);
    logPerf("[perf] classic UpdateFull mode=%d took=%lu\n",
                  static_cast<int>(mode), millis() - step_started_ms);
    partial_refresh_count_ = 0;
    logPerf("[perf] renderClassicClockPage total=%lu\n",
                  millis() - render_started_ms);
}

void ClockApp::renderDashboardClockPage(bool full_refresh) {
    const uint32_t render_started_ms = millis();
    logPerf("[perf] renderDashboardClockPage start full=%d at=%lu\n",
                  full_refresh ? 1 : 0, render_started_ms);
    page_canvas_.fillCanvas(kWhite);
    page_canvas_.pushCanvas(0, 0, UPDATE_MODE_NONE);

    resetClockRenderState();
    const bool fast_entry = full_refresh && fast_dashboard_entry_render_;
    updateDashboardLayoutComponents(true);
    const m5epd_update_mode_t mode =
        full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16;
    M5.EPD.UpdateFull(mode);
    partial_refresh_count_ = 0;
    if (fast_entry) {
        pending_dashboard_date_cjk_render_ = true;
        pending_dashboard_calendar_cjk_render_ = true;
    }
    fast_dashboard_entry_render_ = false;
    logPerf("[perf] renderDashboardClockPage total=%lu\n",
                  millis() - render_started_ms);
}

void ClockApp::resetClockRenderState() {
    last_time_text_rendered_ = "";
    last_dashboard_time_text_rendered_ = "";
    last_humidity_text_rendered_ = "";
    last_temperature_text_rendered_ = "";
    last_comfort_face_rendered_ = "";
    last_dashboard_humidity_text_rendered_ = "";
    last_dashboard_temperature_text_rendered_ = "";
    last_dashboard_comfort_face_rendered_ = "";
    last_market_summary_rendered_ = "";
    last_dashboard_summary_title_rendered_ = "";
    last_dashboard_summary_price_rendered_ = "";
    last_dashboard_summary_bottom_rendered_ = "";
    for (MarketRenderState& state : market_render_states_) {
        state.last_signature = "";
        state.last_title = "";
        state.last_price = "";
        state.last_bottom = "";
        state.last_valid = false;
    }
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
    classic_time_partial_count_ = 0;
    classic_humidity_partial_count_ = 0;
    classic_temperature_partial_count_ = 0;
    classic_comfort_partial_count_ = 0;
    date_partial_count_ = 0;
    dashboard_calendar_partial_count_ = 0;
    dashboard_summary_partial_count_ = 0;
    dashboard_time_partial_count_ = 0;
    dashboard_climate_partial_count_ = 0;
    last_dashboard_summary_valid_ = false;
}

void ClockApp::updateLayoutComponent(logic::DashboardComponentId id,
                                     bool full_refresh, bool allow_fetch) {
    uint32_t step_started_ms = millis();
    const ComponentUpdateBinding* binding = componentUpdateBinding(id);
    if (binding && binding->handler) {
        (this->*binding->handler)(full_refresh, allow_fetch);
    }
    const char* label = logic::DashboardComponentPerfLabel(id);
    logPerf("[perf] dashboard update %s took=%lu\n", label,
                  millis() - step_started_ms);
}

void ClockApp::updateLayoutComponents(const logic::DashboardComponentId* ids,
                                      size_t count, bool full_refresh,
                                      bool allow_fetch) {
    for (size_t index = 0; index < count; ++index) {
        updateLayoutComponent(ids[index], full_refresh, allow_fetch);
    }
}

void ClockApp::updateDashboardLayoutComponents(bool full_refresh) {
    updateLayoutComponents(logic::kFullRefreshComponents.ids,
                           logic::kFullRefreshComponents.count,
                           full_refresh,
                           false);
}

void ClockApp::updateDashboardMinuteComponents() {
    updateLayoutComponents(logic::kMinuteComponents.ids,
                           logic::kMinuteComponents.count,
                           false);
}

void ClockApp::updateDashboardDateComponents() {
    updateLayoutComponents(logic::kDateComponents.ids,
                           logic::kDateComponents.count,
                           false);
}

void ClockApp::updateDashboardSensorComponents() {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Climate) &&
        !dashboardComponentVisible(logic::DashboardComponentId::ClassicInfo)) {
        last_sensor_read_ms_ = millis();
        return;
    }
    updateLayoutComponents(logic::kSensorComponents.ids,
                           logic::kSensorComponents.count,
                           false);
}

void ClockApp::updateDashboardMarketComponents() {
    updateLayoutComponents(logic::kMarketComponents.ids,
                           logic::kMarketComponents.count, false);
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

    if (usesDashboardLayout()) {
        if (minute_changed) {
            updateDashboardMinuteComponents();
        }
        if (date_changed) {
            updateDashboardDateComponents();
        }
        if (time_for_market) {
            updateDashboardMarketComponents();
        }
        if (minute_changed || time_for_sensor) {
            updateBatteryCanvas(false);
        }
        if (time_for_sensor) {
            updateDashboardSensorComponents();
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

#include "ClockApp.Components.inc"

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

#include "ClockApp.Config.inc"

#include "ClockApp.Input.inc"

#include "ClockApp.Runtime.inc"

#include "ClockApp.LayoutPresets.inc"

#include "ClockApp.Ota.inc"

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

#include "ClockApp.Layout.inc"

#include "ClockApp.MarketState.inc"

#include "ClockApp.Status.inc"
