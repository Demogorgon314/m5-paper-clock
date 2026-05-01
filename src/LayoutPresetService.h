#pragma once

#include <Arduino.h>

#include "AppSettingsTypes.h"

namespace layoutpreset {

struct ApplyPresetResult {
    bool market_changed = false;
    String market_symbol;
    String market_name;
};

int ActiveDashboardPresetIndex(const AppSettings& settings);
bool UsesDashboardLayout(const AppSettings& settings);
void ResetToDefaultDashboard(AppSettings& settings);
ApplyPresetResult ApplyDashboardPreset(AppSettings& settings,
                                        const SavedDashboardLayout& preset);
bool UpsertActiveDashboardPreset(AppSettings& settings,
                                 const String& name = String());

}  // namespace layoutpreset
