#pragma once

#include <cstdint>

#include "logic/LayoutTypes.h"

namespace logic {

enum ComponentUpdateFlag : uint8_t {
    kUpdateOnFullRefresh = 1 << 0,
    kUpdateOnMinute = 1 << 1,
    kUpdateOnDate = 1 << 2,
    kUpdateOnSensor = 1 << 3,
    kUpdateOnMarket = 1 << 4,
};

constexpr uint8_t operator|(ComponentUpdateFlag left,
                            ComponentUpdateFlag right) {
    return static_cast<uint8_t>(left) | static_cast<uint8_t>(right);
}

constexpr uint8_t operator|(uint8_t left, ComponentUpdateFlag right) {
    return left | static_cast<uint8_t>(right);
}

constexpr bool HasComponentUpdateFlag(uint8_t flags,
                                      ComponentUpdateFlag flag) {
    return (flags & static_cast<uint8_t>(flag)) != 0;
}

struct DashboardComponentDefinition {
    DashboardLayoutItem default_item;
    const char* perf_label;
    uint8_t update_flags;
};

}  // namespace logic
