#pragma once

#include <array>
#include <cstddef>

#include "logic/LayoutTypes.h"
#include "logic/components/BatteryComponent.h"
#include "logic/components/CalendarComponent.h"
#include "logic/components/ClassicInfoComponent.h"
#include "logic/components/ClassicTimeComponent.h"
#include "logic/components/ClimateComponent.h"
#include "logic/components/DateComponent.h"
#include "logic/components/MarketComponent.h"
#include "logic/components/TimeComponent.h"

namespace logic {

static constexpr size_t kDashboardComponentCount =
    static_cast<size_t>(DashboardComponentId::Count);

using DashboardLayout =
    std::array<DashboardLayoutItem, kDashboardComponentCount>;

struct DashboardComponentDefinition {
    DashboardLayoutItem default_item;
    const char* perf_label;
};

struct ClassicLayoutComponentDefinition {
    const char* instance_id;
    DashboardComponentId source_id;
    const char* type;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    const char* variant;
};

static constexpr std::array<DashboardComponentDefinition,
                            kDashboardComponentCount>
    kDashboardComponentDefinitions = {{
        {kDateComponent, "date"},
        {kBatteryComponent, "battery"},
        {kCalendarComponent, "calendar"},
        {kTimeComponent, "time"},
        {kMarketComponent, "summary"},
        {kClimateComponent, "climate"},
        {kClassicTimeComponent, "classic-time"},
        {kClassicInfoComponent, "classic-info"},
    }};

static constexpr std::array<ClassicLayoutComponentDefinition, 4>
    kClassicLayoutComponentDefinitions = {{
        {"classic-date", DashboardComponentId::Date, "date", 24, 20, 744, 44,
         "classic-header"},
        {"classic-battery", DashboardComponentId::Battery, "battery", 768, 20,
         176, 44, "classic-status"},
        {"classic-time", DashboardComponentId::ClassicTime, "classic-time", 100,
         72, 760, 300, "large-digits"},
        {"classic-info", DashboardComponentId::ClassicInfo, "classic-info", 100,
         378, 760, 120, "metrics"},
    }};

inline DashboardLayout DefaultDashboardLayout() {
    DashboardLayout layout {};
    for (size_t index = 0; index < kDashboardComponentDefinitions.size();
         ++index) {
        layout[index] = kDashboardComponentDefinitions[index].default_item;
    }
    return layout;
}

inline const DashboardComponentDefinition& DashboardComponentDefinitionForId(
    DashboardComponentId id) {
    return kDashboardComponentDefinitions[static_cast<size_t>(id)];
}

inline const char* DashboardComponentPerfLabel(DashboardComponentId id) {
    if (id == DashboardComponentId::Count) {
        return "unknown";
    }
    return DashboardComponentDefinitionForId(id).perf_label;
}

inline const DashboardLayoutItem& MarketComponentDefaultItem() {
    return DashboardComponentDefinitionForId(
        DashboardComponentId::Summary).default_item;
}

}  // namespace logic
