#pragma once

#include "logic/ComponentTypes.h"

namespace logic {

static constexpr DashboardLayoutItem kMarketComponent = {
    DashboardComponentId::Summary, "market-1", "market", 72, 392, 332, 86,
    true, "summary-card"};
static constexpr DashboardComponentDefinition kMarketComponentDefinition = {
    kMarketComponent, "summary",
    kUpdateOnFullRefresh | kUpdateOnDate | kUpdateOnMarket};

}  // namespace logic
