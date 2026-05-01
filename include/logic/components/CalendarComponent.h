#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kCalendarComponent = {
    DashboardComponentId::Calendar, "calendar-main", "calendar", 72, 106, 304,
    232, true, "month-grid"};
static constexpr DashboardComponentDefinition kCalendarComponentDefinition = {
    kCalendarComponent, "calendar", kUpdateOnFullRefresh | kUpdateOnDate};

}  // namespace logic
