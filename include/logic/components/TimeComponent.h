#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kTimeComponent = {
    DashboardComponentId::Time, "time-main", "time", 412, 104, 472, 236, true,
    "large-digits"};
static constexpr DashboardComponentDefinition kTimeComponentDefinition = {
    kTimeComponent, "time", kUpdateOnFullRefresh | kUpdateOnMinute};

}  // namespace logic
