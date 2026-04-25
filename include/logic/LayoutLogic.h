#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace logic {

static constexpr int16_t kLayoutScreenWidth = 960;
static constexpr int16_t kLayoutScreenHeight = 540;
static constexpr const char* kDefaultLayoutId = "dashboard-default";
static constexpr const char* kDefaultLayoutName = "仪表盘";

enum class DashboardComponentId : uint8_t {
    Date = 0,
    Battery,
    Calendar,
    Time,
    Summary,
    Climate,
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

static constexpr size_t kDashboardComponentCount =
    static_cast<size_t>(DashboardComponentId::Count);

using DashboardLayout =
    std::array<DashboardLayoutItem, kDashboardComponentCount>;

inline DashboardLayout DefaultDashboardLayout() {
    return {{
        {DashboardComponentId::Date, "date-main", "date", 24, 20, 744, 44,
         true, "header"},
        {DashboardComponentId::Battery, "battery-main", "battery", 768, 20,
         176, 44, true, "status"},
        {DashboardComponentId::Calendar, "calendar-main", "calendar", 72, 106,
         304, 232, true, "month-grid"},
        {DashboardComponentId::Time, "time-main", "time", 412, 104, 472, 236,
         true, "large-digits"},
        {DashboardComponentId::Summary, "market-1", "market", 72, 392, 332,
         86, true, "summary-card"},
        {DashboardComponentId::Climate, "climate-main", "climate", 428, 392,
         456, 86, true, "compact-card"},
    }};
}

inline int DashboardComponentIndex(DashboardComponentId id) {
    return static_cast<int>(id);
}

inline const DashboardLayoutItem& DashboardLayoutDefaultItem(
    DashboardComponentId id) {
    static const DashboardLayout defaults = DefaultDashboardLayout();
    return defaults[DashboardComponentIndex(id)];
}

inline const DashboardLayoutItem* FindDashboardLayoutItem(
    const DashboardLayout& layout, DashboardComponentId id) {
    for (const DashboardLayoutItem& item : layout) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

inline DashboardLayoutItem ClampDashboardLayoutItem(
    const DashboardLayoutItem& item) {
    DashboardLayoutItem clamped = item;
    if (clamped.w <= 0 || clamped.h <= 0) {
        const DashboardLayoutItem& default_item =
            DashboardLayoutDefaultItem(clamped.id);
        clamped.w = default_item.w;
        clamped.h = default_item.h;
    }
    if (clamped.w > kLayoutScreenWidth) {
        clamped.w = kLayoutScreenWidth;
    }
    if (clamped.h > kLayoutScreenHeight) {
        clamped.h = kLayoutScreenHeight;
    }

    const int16_t max_x = kLayoutScreenWidth - clamped.w;
    const int16_t max_y = kLayoutScreenHeight - clamped.h;
    if (clamped.x < 0) {
        clamped.x = 0;
    } else if (clamped.x > max_x) {
        clamped.x = max_x;
    }
    if (clamped.y < 0) {
        clamped.y = 0;
    } else if (clamped.y > max_y) {
        clamped.y = max_y;
    }
    return clamped;
}

inline DashboardLayout NormalizeDashboardLayout(DashboardLayout layout) {
    for (DashboardLayoutItem& item : layout) {
        item = ClampDashboardLayoutItem(item);
    }
    return layout;
}

inline bool StringsEqual(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0' && *left == *right) {
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

inline DashboardComponentId DashboardComponentIdFromKey(const char* key,
                                                        bool& found) {
    const DashboardLayout defaults = DefaultDashboardLayout();
    for (const DashboardLayoutItem& item : defaults) {
        if (StringsEqual(item.instance_id, key) || StringsEqual(item.type, key)) {
            found = true;
            return item.id;
        }
    }
    found = false;
    return DashboardComponentId::Date;
}

}  // namespace logic
