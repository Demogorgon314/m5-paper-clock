#pragma once

#include "logic/ComponentRegistry.h"

namespace logic {

inline int DashboardComponentIndex(DashboardComponentId id) {
    return static_cast<int>(id);
}

inline const DashboardLayoutItem& DashboardLayoutDefaultItem(
    DashboardComponentId id) {
    return DashboardComponentDefinitionForId(id).default_item;
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
    for (const DashboardComponentDefinition& definition :
         kDashboardComponentDefinitions) {
        const DashboardLayoutItem& item = definition.default_item;
        if (StringsEqual(item.instance_id, key) || StringsEqual(item.type, key)) {
            found = true;
            return item.id;
        }
    }
    found = false;
    return DashboardComponentId::Date;
}

}  // namespace logic
