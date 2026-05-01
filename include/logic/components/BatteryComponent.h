#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kBatteryComponent = {
    DashboardComponentId::Battery, "battery-main", "battery", 768, 20, 176,
    44, true, "status"};
static constexpr DashboardComponentDefinition kBatteryComponentDefinition = {
    kBatteryComponent, "battery", kUpdateOnFullRefresh};

}  // namespace logic
