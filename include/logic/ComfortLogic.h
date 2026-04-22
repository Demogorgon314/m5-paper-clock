#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace logic {

constexpr float kDefaultComfortMinTemperature = 19.0f;
constexpr float kDefaultComfortMaxTemperature = 27.0f;
constexpr float kDefaultComfortMinHumidity = 20.0f;
constexpr float kDefaultComfortMaxHumidity = 85.0f;

constexpr float kComfortTemperatureLowerBound = -20.0f;
constexpr float kComfortTemperatureUpperBound = 60.0f;
constexpr float kComfortHumidityLowerBound = 0.0f;
constexpr float kComfortHumidityUpperBound = 100.0f;

struct ComfortSettings {
    constexpr ComfortSettings(float min_temp = kDefaultComfortMinTemperature,
                              float max_temp = kDefaultComfortMaxTemperature,
                              float min_humidity = kDefaultComfortMinHumidity,
                              float max_humidity = kDefaultComfortMaxHumidity) noexcept
        : min_temperature(min_temp),
          max_temperature(max_temp),
          min_humidity(min_humidity),
          max_humidity(max_humidity) {
    }

    float min_temperature;
    float max_temperature;
    float min_humidity;
    float max_humidity;
};

enum class ComfortState : uint8_t {
    Offline = 0,
    Comfortable,
    Uncomfortable,
};

inline float ClampComfortValue(float value, float lower, float upper,
                               float fallback) {
    if (!std::isfinite(value)) {
        return fallback;
    }
    if (value < lower) {
        return lower;
    }
    if (value > upper) {
        return upper;
    }
    return value;
}

inline ComfortSettings NormalizeComfortSettings(
    const ComfortSettings& input) noexcept {
    ComfortSettings normalized(
        ClampComfortValue(input.min_temperature, kComfortTemperatureLowerBound,
                          kComfortTemperatureUpperBound,
                          kDefaultComfortMinTemperature),
        ClampComfortValue(input.max_temperature, kComfortTemperatureLowerBound,
                          kComfortTemperatureUpperBound,
                          kDefaultComfortMaxTemperature),
        ClampComfortValue(input.min_humidity, kComfortHumidityLowerBound,
                          kComfortHumidityUpperBound,
                          kDefaultComfortMinHumidity),
        ClampComfortValue(input.max_humidity, kComfortHumidityLowerBound,
                          kComfortHumidityUpperBound,
                          kDefaultComfortMaxHumidity));

    if (normalized.min_temperature > normalized.max_temperature) {
        std::swap(normalized.min_temperature, normalized.max_temperature);
    }
    if (normalized.min_humidity > normalized.max_humidity) {
        std::swap(normalized.min_humidity, normalized.max_humidity);
    }
    return normalized;
}

inline bool IsComfortable(float temperature, float humidity, bool valid,
                          const ComfortSettings& settings) noexcept {
    if (!valid) {
        return false;
    }

    const ComfortSettings normalized = NormalizeComfortSettings(settings);
    return temperature >= normalized.min_temperature &&
           temperature <= normalized.max_temperature &&
           humidity >= normalized.min_humidity &&
           humidity <= normalized.max_humidity;
}

inline ComfortState ComfortStateForReading(float temperature, float humidity,
                                           bool valid,
                                           const ComfortSettings& settings) noexcept {
    if (!valid) {
        return ComfortState::Offline;
    }

    return IsComfortable(temperature, humidity, valid, settings)
               ? ComfortState::Comfortable
               : ComfortState::Uncomfortable;
}

inline const char* ComfortFaceText(ComfortState state) noexcept {
    switch (state) {
        case ComfortState::Comfortable:
            return "(^_^)";
        case ComfortState::Uncomfortable:
            return "(-^-)";
        case ComfortState::Offline:
        default:
            return "(-_-)";
    }
}

}  // namespace logic
