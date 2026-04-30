#pragma once

#include <cstdint>

namespace logic {

inline bool DateDisplayStringsEqual(const char* left, const char* right) {
    if (left == nullptr || right == nullptr) {
        return left == right;
    }
    while (*left != '\0' && *right != '\0' && *left == *right) {
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

inline const char* NormalizeDateFormat(const char* value) {
    if (DateDisplayStringsEqual(value, "yyyy/mm/dd") ||
        DateDisplayStringsEqual(value, "mm-dd") ||
        DateDisplayStringsEqual(value, u8"yyyy年m月d日")) {
        return value;
    }
    return "yyyy-mm-dd";
}

inline const char* NormalizeWeekdayFormat(const char* value) {
    if (DateDisplayStringsEqual(value, "long") ||
        DateDisplayStringsEqual(value, "narrow") ||
        DateDisplayStringsEqual(value, "english-short")) {
        return value;
    }
    return "short";
}

inline const char* NormalizeDateLayout(const char* value) {
    if (DateDisplayStringsEqual(value, "date-only") ||
        DateDisplayStringsEqual(value, "holiday-only") ||
        DateDisplayStringsEqual(value, "two-line")) {
        return value;
    }
    return "inline";
}

inline uint8_t NormalizeDateTextSize(int value) {
    if (value == 2 || value == 3 || value == 7) {
        return static_cast<uint8_t>(value);
    }
    return 7;
}

inline bool IsLeapYear(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

inline uint8_t DaysInMonth(int year, int month) {
    static constexpr uint8_t kMonthDays[] = {31, 28, 31, 30, 31, 30,
                                             31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) {
        return 30;
    }
    if (month == 2 && IsLeapYear(year)) {
        return 29;
    }
    return kMonthDays[month - 1];
}

}  // namespace logic
