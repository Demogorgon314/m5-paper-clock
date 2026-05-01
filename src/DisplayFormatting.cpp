#include "DisplayFormatting.h"

#include <string>

#include "logic/ClimateDisplayLogic.h"
#include "logic/DateDisplayLogic.h"
#include "logic/MarketLogic.h"
#include "logic/UiLogic.h"

namespace displayfmt {

String TwoDigits(int value) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d", value);
    return String(buf);
}

String TimeDigits(const rtc_time_t& time) {
    return TwoDigits(time.hour) + TwoDigits(time.min);
}

String DateTime(const rtc_date_t& date, const rtc_time_t& time) {
    if (date.year < 2020) {
        return String("--/--/-- --:--");
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d", date.year, date.mon,
             date.day, time.hour, time.min);
    return String(buf);
}

String DateOnly(const rtc_date_t& date) {
    if (date.year < 2020) {
        return String("");
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", date.year, date.mon,
             date.day);
    return String(buf);
}

String ConfiguredDate(const rtc_date_t& date, const String& format) {
    if (date.year < 2020) {
        return String("");
    }

    const String normalized = NormalizeDateFormat(format);
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

String ClassicHumidityField(const EnvironmentReading& reading) {
    return String(logic::FormatClassicHumidityField(reading.humidity,
                                                    reading.valid).c_str());
}

String ClassicTemperatureField(const EnvironmentReading& reading) {
    return String(logic::FormatClassicTemperatureField(reading.temperature,
                                                       reading.valid).c_str());
}

String DashboardHumidity(const EnvironmentReading& reading) {
    return String(logic::FormatDashboardHumidity(reading.humidity,
                                                 reading.valid).c_str());
}

String DashboardTemperature(const EnvironmentReading& reading) {
    return String(logic::FormatDashboardTemperature(reading.temperature,
                                                    reading.valid).c_str());
}

String NormalizeDateFormat(const String& value) {
    return String(logic::NormalizeDateFormat(value.c_str()));
}

String NormalizeWeekdayFormat(const String& value) {
    return String(logic::NormalizeWeekdayFormat(value.c_str()));
}

String NormalizeDateLayout(const String& value) {
    return String(logic::NormalizeDateLayout(value.c_str()));
}

uint8_t NormalizeDateTextSize(uint8_t value) {
    return logic::NormalizeDateTextSize(value);
}

String HolidayDisplaySignature(const logic::HolidayDisplay& display) {
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

String HolidayDisplayText(const logic::HolidayDisplay& display,
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

String HolidayText(const rtc_date_t& date, bool show_holiday) {
    if (!show_holiday) {
        return String("");
    }
    const logic::HolidayDisplay display =
        logic::HolidayDisplayForDate(date.year, date.mon, date.day);
    return HolidayDisplayText(display, false);
}

String WeekdayLabel(uint8_t week) {
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

String WeekdayLabel(uint8_t week, const String& format) {
    const uint8_t normalized_week = week % 7;
    const String normalized = NormalizeWeekdayFormat(format);
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
    return WeekdayLabel(normalized_week);
}

uint8_t CalculateWeekday(const rtc_date_t& date) {
    return logic::WeekdayFromCivil(date.year, date.mon, date.day);
}

uint8_t DaysInMonth(int year, int month) {
    return logic::DaysInMonth(year, month);
}

String DashboardMonth(const rtc_date_t& date) {
    return TwoDigits(date.mon);
}

String MarketQuoteTimeLabel(const MarketQuote& quote) {
    return String(logic::MarketQuoteTimeLabel(
        std::string(quote.updated_at.c_str())).c_str());
}

String MarketStatusLabel(const MarketQuote& quote,
                         bool market_open,
                         bool use_cjk_font) {
    return String(logic::MarketStatusLabel(
        std::string(quote.updated_at.c_str()), market_open,
        use_cjk_font).c_str());
}

String MarketDisplayLabel(const MarketQuote& quote, bool prefer_display_name) {
    return String(logic::MarketDisplayLabel(
        std::string(quote.symbol.c_str()),
        std::string(quote.display_name.c_str()),
        prefer_display_name).c_str());
}

String MarketCodeFromRequestSymbol(const String& request_symbol) {
    return String(logic::MarketCodeFromRequestSymbol(
        std::string(request_symbol.c_str())).c_str());
}

bool MarketSearchMatchesQuery(const MarketSearchResult& result,
                              const String& query) {
    return logic::MarketSearchMatchesQuery(
        std::string(result.request_symbol.c_str()),
        std::string(result.code.c_str()), std::string(query.c_str()));
}

String ComfortFace(const EnvironmentReading& reading,
                   const logic::ComfortSettings& comfort_settings) {
    return String(logic::ComfortFaceText(logic::ComfortStateForReading(
        reading.temperature, reading.humidity, reading.valid,
        comfort_settings)));
}

uint8_t WifiSignalLevelFromRssi(int32_t rssi) {
    return static_cast<uint8_t>(logic::WifiSignalLevelFromRssi(rssi));
}

uint8_t WifiBitmapLevel(uint8_t signal_level) {
    return static_cast<uint8_t>(logic::WifiBitmapLevel(signal_level));
}

}  // namespace displayfmt
