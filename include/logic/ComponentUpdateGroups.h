#pragma once

#include <cstddef>

#include "logic/LayoutTypes.h"

namespace logic {

template <size_t N>
struct ComponentUpdateGroup {
    DashboardComponentId ids[N];
    size_t count;
};

static constexpr ComponentUpdateGroup<8> kFullRefreshComponents = {{
    DashboardComponentId::Date,
    DashboardComponentId::Battery,
    DashboardComponentId::Calendar,
    DashboardComponentId::Time,
    DashboardComponentId::ClassicTime,
    DashboardComponentId::Summary,
    DashboardComponentId::Climate,
    DashboardComponentId::ClassicInfo,
}, 8};

static constexpr ComponentUpdateGroup<2> kMinuteComponents = {{
    DashboardComponentId::Time,
    DashboardComponentId::ClassicTime,
}, 2};

static constexpr ComponentUpdateGroup<3> kDateComponents = {{
    DashboardComponentId::Date,
    DashboardComponentId::Calendar,
    DashboardComponentId::Summary,
}, 3};

static constexpr ComponentUpdateGroup<2> kSensorComponents = {{
    DashboardComponentId::Climate,
    DashboardComponentId::ClassicInfo,
}, 2};

static constexpr ComponentUpdateGroup<1> kMarketComponents = {{
    DashboardComponentId::Summary,
}, 1};

}  // namespace logic

