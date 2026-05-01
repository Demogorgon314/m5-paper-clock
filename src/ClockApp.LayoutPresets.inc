// Included by ClockApp.cpp so layout preset state stays separate from
// connectivity/runtime orchestration while the app class is being split.
int ClockApp::activeDashboardPresetIndex() const {
    return layoutpreset::ActiveDashboardPresetIndex(settings_);
}

void ClockApp::applyDashboardPreset(const SavedDashboardLayout& preset) {
    const layoutpreset::ApplyPresetResult result =
        layoutpreset::ApplyDashboardPreset(settings_, preset);
    if (!result.market_symbol.isEmpty()) {
        store_.saveMarket(settings_.market_symbol, settings_.market_name);
    }
    syncMarketRenderStates();
    seedMarketQuoteFromSettings();
    resetClockRenderState();
}

void ClockApp::upsertActiveDashboardPreset(const String& name) {
    if (layoutpreset::UpsertActiveDashboardPreset(settings_, name)) {
        store_.saveLayoutPresets(settings_.layout_presets);
    }
}
