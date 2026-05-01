#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kClimateComponent = {
    DashboardComponentId::Climate, "climate-main", "climate", 428, 392, 456,
    86, true, "compact-card"};
static constexpr DashboardComponentDefinition kClimateComponentDefinition = {
    kClimateComponent, "climate", kUpdateOnFullRefresh | kUpdateOnSensor};

}  // namespace logic
