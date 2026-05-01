#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kClassicTimeComponent = {
    DashboardComponentId::ClassicTime, "classic-time", "classic-time", 100, 72,
    760, 300, false, "large-digits"};
static constexpr DashboardComponentDefinition kClassicTimeComponentDefinition = {
    kClassicTimeComponent, "classic-time",
    kUpdateOnFullRefresh | kUpdateOnMinute};

}  // namespace logic
