#include "LayoutPresetService.h"

#include "logic/MarketLogic.h"

namespace layoutpreset {

namespace {

String marketDisplayNameForSymbol(const String& request_symbol) {
    const std::string market_code =
        logic::MarketCodeFromRequestSymbol(std::string(request_symbol.c_str()));
    const std::string known_name = logic::KnownMarketDisplayName(market_code);
    return known_name.empty() ? String(market_code.c_str())
                              : String(known_name.c_str());
}

}  // namespace

int ActiveDashboardPresetIndex(const AppSettings& settings) {
    for (size_t index = 0; index < settings.layout_presets.size(); ++index) {
        if (settings.layout_presets[index].id == settings.active_layout_id) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

bool UsesDashboardLayout(const AppSettings& settings) {
    return settings.active_layout_id != logic::kClassicLayoutId;
}

void ResetToDefaultDashboard(AppSettings& settings) {
    settings.dashboard_layout = logic::DefaultDashboardLayout();
    settings.active_layout_id = logic::kDefaultLayoutId;
    settings.market_layout = {MarketLayoutItem {}};

    SavedDashboardLayout preset;
    preset.id = logic::kDefaultLayoutId;
    preset.name = logic::kDefaultLayoutName;
    preset.dashboard_layout = settings.dashboard_layout;
    preset.market_layout = settings.market_layout;
    settings.layout_presets = {preset};
}

ApplyPresetResult ApplyDashboardPreset(AppSettings& settings,
                                       const SavedDashboardLayout& preset) {
    ApplyPresetResult result;
    settings.active_layout_id = preset.id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : preset.id;
    settings.dashboard_layout = preset.dashboard_layout;
    settings.market_layout = preset.market_layout;
    if (!settings.market_layout.empty() &&
        !settings.market_layout.front().symbol.isEmpty()) {
        const String market_symbol = settings.market_layout.front().symbol;
        if (market_symbol != settings.market_symbol) {
            result.market_changed = true;
        }
        settings.market_symbol = market_symbol;
        settings.market_name = marketDisplayNameForSymbol(settings.market_symbol);
        result.market_symbol = settings.market_symbol;
        result.market_name = settings.market_name;
    }
    return result;
}

bool UpsertActiveDashboardPreset(AppSettings& settings, const String& name) {
    if (!UsesDashboardLayout(settings)) {
        return false;
    }
    const String preset_id = settings.active_layout_id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : settings.active_layout_id;
    SavedDashboardLayout preset;
    preset.id = preset_id;
    preset.name = name.isEmpty() ? preset_id : name;
    preset.dashboard_layout = settings.dashboard_layout;
    preset.market_layout = settings.market_layout;
    for (SavedDashboardLayout& existing : settings.layout_presets) {
        if (existing.id == preset_id) {
            if (!name.isEmpty()) {
                existing.name = name;
            } else if (existing.name.isEmpty()) {
                existing.name = preset.name;
            }
            existing.dashboard_layout = preset.dashboard_layout;
            existing.market_layout = preset.market_layout;
            return true;
        }
    }

    settings.layout_presets.push_back(preset);
    if (settings.layout_presets.size() > 5) {
        for (auto it = settings.layout_presets.begin();
             it != settings.layout_presets.end(); ++it) {
            if (it->id != logic::kDefaultLayoutId &&
                it->id != settings.active_layout_id) {
                settings.layout_presets.erase(it);
                break;
            }
        }
        if (settings.layout_presets.size() > 5) {
            settings.layout_presets.erase(settings.layout_presets.begin());
        }
    }
    return true;
}

}  // namespace layoutpreset
