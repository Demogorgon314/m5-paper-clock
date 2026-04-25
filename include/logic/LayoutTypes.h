#pragma once

#include <cstdint>

namespace logic {

static constexpr int16_t kLayoutScreenWidth = 960;
static constexpr int16_t kLayoutScreenHeight = 540;
static constexpr const char* kClassicLayoutId = "classic-default";
static constexpr const char* kClassicLayoutName = "经典数字";
static constexpr const char* kDefaultLayoutId = "dashboard-default";
static constexpr const char* kDefaultLayoutName = "仪表盘";

enum class DashboardComponentId : uint8_t {
    Date = 0,
    Battery,
    Calendar,
    Time,
    Summary,
    Climate,
    ClassicTime,
    ClassicInfo,
    Count
};

struct DashboardLayoutItem {
    DashboardComponentId id;
    const char* instance_id;
    const char* type;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    bool visible;
    const char* variant;
};

}  // namespace logic

