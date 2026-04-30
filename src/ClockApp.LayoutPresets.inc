// Included by ClockApp.cpp so layout preset state stays separate from
// connectivity/runtime orchestration while the app class is being split.
int ClockApp::activeDashboardPresetIndex() const {
    for (size_t index = 0; index < settings_.layout_presets.size(); ++index) {
        if (settings_.layout_presets[index].id == settings_.active_layout_id) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

void ClockApp::applyDashboardPreset(const SavedDashboardLayout& preset) {
    settings_.active_layout_id = preset.id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : preset.id;
    settings_.dashboard_layout = preset.dashboard_layout;
    settings_.market_layout = preset.market_layout;
    if (!settings_.market_layout.empty() &&
        !settings_.market_layout.front().symbol.isEmpty()) {
        settings_.market_symbol = settings_.market_layout.front().symbol;
        const String market_code =
            marketCodeFromRequestSymbol(settings_.market_symbol);
        const std::string known_name =
            logic::KnownMarketDisplayName(std::string(market_code.c_str()));
        settings_.market_name = known_name.empty()
            ? market_code
            : String(known_name.c_str());
        store_.saveMarket(settings_.market_symbol, settings_.market_name);
    }
    syncMarketRenderStates();
    seedMarketQuoteFromSettings();
    resetClockRenderState();
}

void ClockApp::upsertActiveDashboardPreset(const String& name) {
    if (!usesDashboardLayout()) {
        return;
    }
    const String preset_id = settings_.active_layout_id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : settings_.active_layout_id;
    SavedDashboardLayout preset;
    preset.id = preset_id;
    preset.name = name.isEmpty() ? preset_id : name;
    preset.dashboard_layout = settings_.dashboard_layout;
    preset.market_layout = settings_.market_layout;
    for (SavedDashboardLayout& existing : settings_.layout_presets) {
        if (existing.id == preset_id) {
            if (!name.isEmpty()) {
                existing.name = name;
            } else if (existing.name.isEmpty()) {
                existing.name = preset.name;
            }
            existing.dashboard_layout = preset.dashboard_layout;
            existing.market_layout = preset.market_layout;
            store_.saveLayoutPresets(settings_.layout_presets);
            return;
        }
    }
    settings_.layout_presets.push_back(preset);
    if (settings_.layout_presets.size() > 5) {
        for (auto it = settings_.layout_presets.begin();
             it != settings_.layout_presets.end(); ++it) {
            if (it->id != logic::kDefaultLayoutId &&
                it->id != settings_.active_layout_id) {
                settings_.layout_presets.erase(it);
                break;
            }
        }
        if (settings_.layout_presets.size() > 5) {
            settings_.layout_presets.erase(settings_.layout_presets.begin());
        }
    }
    store_.saveLayoutPresets(settings_.layout_presets);
}
