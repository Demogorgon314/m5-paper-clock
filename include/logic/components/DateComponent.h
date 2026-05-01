#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kDateComponent = {
    DashboardComponentId::Date, "date-main", "date", 24, 20, 744, 44, true,
    "header"};
static constexpr DashboardComponentDefinition kDateComponentDefinition = {
    kDateComponent, "date", kUpdateOnFullRefresh | kUpdateOnDate};

}  // namespace logic
