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

}  // namespace logic
