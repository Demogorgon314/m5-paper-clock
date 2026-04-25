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

inline DashboardLayout DefaultDashboardLayout() {
    return {{
        kDateComponent,
        kBatteryComponent,
        kCalendarComponent,
        kTimeComponent,
        kMarketComponent,
        kClimateComponent,
        kClassicTimeComponent,
        kClassicInfoComponent,
    }};
}

}  // namespace logic

