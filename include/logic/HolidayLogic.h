#pragma once

#include <cstdint>

namespace logic {

enum class HolidayId : uint8_t {
    None = 0,
    YuanDan,
    ChunJie,
    QingMing,
    LaoDong,
    DuanWu,
    ZhongQiu,
    GuoQing,
};

struct HolidayEntry {
    constexpr HolidayEntry(HolidayId holiday_id = HolidayId::None,
                           int holiday_year = 0,
                           uint8_t holiday_month = 0,
                           uint8_t holiday_day = 0) noexcept
        : id(holiday_id),
          year(holiday_year),
          month(holiday_month),
          day(holiday_day) {
    }

    HolidayId id;
    int year;
    uint8_t month;
    uint8_t day;
};

struct HolidayPeriod {
    constexpr HolidayPeriod(HolidayId holiday_id = HolidayId::None,
                            int holiday_start_year = 0,
                            uint8_t holiday_start_month = 0,
                            uint8_t holiday_start_day = 0,
                            int holiday_end_year = 0,
                            uint8_t holiday_end_month = 0,
                            uint8_t holiday_end_day = 0,
                            uint8_t holiday_total_days = 0) noexcept
        : id(holiday_id),
          start_year(holiday_start_year),
          start_month(holiday_start_month),
          start_day(holiday_start_day),
          end_year(holiday_end_year),
          end_month(holiday_end_month),
          end_day(holiday_end_day),
          total_days(holiday_total_days) {
    }

    HolidayId id;
    int start_year;
    uint8_t start_month;
    uint8_t start_day;
    int end_year;
    uint8_t end_month;
    uint8_t end_day;
    uint8_t total_days;
};

struct HolidayCountdown {
    constexpr HolidayCountdown(HolidayId holiday_id = HolidayId::None,
                               int remaining_days = 0,
                               bool is_valid = false) noexcept
        : id(holiday_id),
          days_remaining(remaining_days),
          valid(is_valid) {
    }

    HolidayId id;
    int days_remaining;
    bool valid;
};

enum class HolidayDisplayState : uint8_t {
    None = 0,
    Countdown,
    InHoliday,
    LastDay,
};

struct HolidayDisplay {
    constexpr HolidayDisplay(
        HolidayDisplayState display_state = HolidayDisplayState::None,
        HolidayId holiday_id = HolidayId::None,
        int remaining_days = 0,
        uint8_t current_holiday_day = 0,
        uint8_t holiday_length = 0) noexcept
        : state(display_state),
          id(holiday_id),
          days_remaining(remaining_days),
          holiday_day_index(current_holiday_day),
          holiday_total_days(holiday_length) {
    }

    HolidayDisplayState state;
    HolidayId id;
    int days_remaining;
    uint8_t holiday_day_index;
    uint8_t holiday_total_days;

    bool valid() const {
        return state != HolidayDisplayState::None;
    }
};

#include "logic/HolidayData.h"

inline int DaysFromCivil(int year, unsigned month, unsigned day) noexcept {
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned year_of_era = static_cast<unsigned>(year - era * 400);
    const unsigned day_of_year =
        (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const unsigned day_of_era = year_of_era * 365 + year_of_era / 4 -
                                year_of_era / 100 + day_of_year;
    return era * 146097 + static_cast<int>(day_of_era) - 719468;
}

inline HolidayCountdown NextHolidayCountdown(int year, int month, int day) {
    HolidayCountdown countdown;
    if (year <= 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return countdown;
    }

    const int current_days =
        DaysFromCivil(year, static_cast<unsigned>(month),
                      static_cast<unsigned>(day));
    bool found = false;
    int best_days = 0;
    for (const HolidayEntry& entry : kHolidayEntries) {
        const int entry_days =
            DaysFromCivil(entry.year, entry.month, entry.day);
        if (entry_days <= current_days) {
            continue;
        }
        if (!found || entry_days < best_days) {
            found = true;
            best_days = entry_days;
            countdown.id = entry.id;
            countdown.days_remaining = entry_days - current_days;
            countdown.valid = true;
        }
    }
    return countdown;
}

inline HolidayDisplay HolidayDisplayForDate(int year, int month, int day) {
    HolidayDisplay display;
    if (year <= 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return display;
    }

    const int current_days =
        DaysFromCivil(year, static_cast<unsigned>(month),
                      static_cast<unsigned>(day));
    bool found_future = false;
    int best_start_days = 0;
    HolidayId next_holiday_id = HolidayId::None;

    for (const HolidayPeriod& period : kHolidayPeriods) {
        const int start_days =
            DaysFromCivil(period.start_year, period.start_month,
                          period.start_day);
        const int end_days =
            DaysFromCivil(period.end_year, period.end_month, period.end_day);
        if (current_days >= start_days && current_days <= end_days) {
            const uint8_t holiday_day_index =
                static_cast<uint8_t>(current_days - start_days + 1);
            if (current_days == end_days) {
                return HolidayDisplay(HolidayDisplayState::LastDay, period.id,
                                      0, holiday_day_index,
                                      period.total_days);
            }
            return HolidayDisplay(HolidayDisplayState::InHoliday, period.id, 0,
                                  holiday_day_index, period.total_days);
        }

        if (start_days <= current_days) {
            continue;
        }
        if (!found_future || start_days < best_start_days) {
            found_future = true;
            best_start_days = start_days;
            next_holiday_id = period.id;
        }
    }

    if (found_future) {
        return HolidayDisplay(HolidayDisplayState::Countdown, next_holiday_id,
                              best_start_days - current_days, 0, 0);
    }
    return display;
}

inline uint8_t WeekdayFromCivil(int year, int month, int day) {
    if (year <= 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }
    const int civil_days =
        DaysFromCivil(year, static_cast<unsigned>(month),
                      static_cast<unsigned>(day));
    const int weekday = (civil_days + 4) % 7;
    return static_cast<uint8_t>(weekday < 0 ? weekday + 7 : weekday);
}

inline bool IsHolidayDate(int year, int month, int day) {
    if (year <= 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    const int current_days =
        DaysFromCivil(year, static_cast<unsigned>(month),
                      static_cast<unsigned>(day));
    for (const HolidayPeriod& period : kHolidayPeriods) {
        const int start_days =
            DaysFromCivil(period.start_year, period.start_month,
                          period.start_day);
        const int end_days =
            DaysFromCivil(period.end_year, period.end_month, period.end_day);
        if (current_days >= start_days && current_days <= end_days) {
            return true;
        }
    }
    return false;
}

inline const char* HolidayNameZh(HolidayId id) {
    switch (id) {
        case HolidayId::YuanDan:
            return u8"元旦";
        case HolidayId::ChunJie:
            return u8"春节";
        case HolidayId::QingMing:
            return u8"清明节";
        case HolidayId::LaoDong:
            return u8"劳动节";
        case HolidayId::DuanWu:
            return u8"端午节";
        case HolidayId::ZhongQiu:
            return u8"中秋节";
        case HolidayId::GuoQing:
            return u8"国庆节";
        case HolidayId::None:
        default:
            return "";
    }
}

}  // namespace logic
