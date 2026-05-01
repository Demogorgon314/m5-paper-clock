#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kClassicInfoComponent = {
    DashboardComponentId::ClassicInfo, "classic-info", "classic-info", 100,
    378, 760, 120, false, "metrics"};
static constexpr DashboardComponentDefinition kClassicInfoComponentDefinition = {
    kClassicInfoComponent, "classic-info",
    kUpdateOnFullRefresh | kUpdateOnSensor};

}  // namespace logic
