#pragma once

#include <array>
#include <cstddef>

#include "logic/ComponentRegistry.h"

namespace logic {

struct ComponentUpdateGroup {
    std::array<DashboardComponentId, kDashboardComponentCount> ids {};
    size_t count = 0;
};

inline ComponentUpdateGroup ComponentUpdateGroupFor(ComponentUpdateFlag flag) {
    ComponentUpdateGroup group {};
    for (size_t index = 0; index < kDashboardComponentDefinitions.size();
         ++index) {
        const DashboardComponentDefinition& definition =
            kDashboardComponentDefinitions[index];
        if (HasComponentUpdateFlag(definition.update_flags, flag)) {
            group.ids[group.count] = definition.default_item.id;
            ++group.count;
        }
    }
    return group;
}

static const ComponentUpdateGroup kFullRefreshComponents =
    ComponentUpdateGroupFor(kUpdateOnFullRefresh);
static const ComponentUpdateGroup kMinuteComponents =
    ComponentUpdateGroupFor(kUpdateOnMinute);
static const ComponentUpdateGroup kDateComponents =
    ComponentUpdateGroupFor(kUpdateOnDate);
static const ComponentUpdateGroup kSensorComponents =
    ComponentUpdateGroupFor(kUpdateOnSensor);
static const ComponentUpdateGroup kMarketComponents =
    ComponentUpdateGroupFor(kUpdateOnMarket);

}  // namespace logic
