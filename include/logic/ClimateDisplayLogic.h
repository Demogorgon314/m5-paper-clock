#pragma once

#include <cstdio>
#include <string>

namespace logic {

inline std::string FormatTemperature(float value) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.1f", value);
    return std::string(buf);
}

inline std::string FormatHumidity(float value) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.0f", value);
    return std::string(buf);
}

inline std::string PadLeft(std::string value, size_t width) {
    if (value.size() >= width) {
        return value.substr(value.size() - width);
    }
    return std::string(width - value.size(), ' ') + value;
}

inline std::string FormatClassicHumidityField(float humidity, bool valid) {
    if (!valid) {
        return " --";
    }
    return PadLeft(FormatHumidity(humidity), 3);
}

inline std::string FormatClassicTemperatureField(float temperature, bool valid) {
    if (!valid) {
        return "---";
    }

    const std::string value = FormatTemperature(temperature);
    const size_t dot_index = value.find('.');
    std::string whole =
        dot_index == std::string::npos ? value : value.substr(0, dot_index);
    const char tenths =
        dot_index != std::string::npos && dot_index + 1 < value.size()
            ? value[dot_index + 1]
            : ' ';
    whole = PadLeft(whole, 2);
    return whole + tenths;
}

inline std::string FormatDashboardHumidity(float humidity, bool valid) {
    return valid ? FormatHumidity(humidity) : "--";
}

inline std::string FormatDashboardTemperature(float temperature, bool valid) {
    return valid ? FormatTemperature(temperature) : "--.-";
}

}  // namespace logic
