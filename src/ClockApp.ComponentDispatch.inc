// Included by ClockApp.cpp. This is the runtime component dispatch table.
// Keep it indexed by DashboardComponentId so enum/count changes fail at compile
// time instead of silently skipping a component at runtime.
ClockApp::ComponentUpdateHandler ClockApp::componentUpdateHandler(
    logic::DashboardComponentId id) const {
    static constexpr std::array<ComponentUpdateHandler,
                                logic::kDashboardComponentCount>
        kHandlers = {{
            &ClockApp::updateDateComponent,
            &ClockApp::updateBatteryComponent,
            &ClockApp::updateCalendarComponent,
            &ClockApp::updateTimeComponent,
            &ClockApp::updateMarketComponent,
            &ClockApp::updateClimateComponent,
            &ClockApp::updateClassicTimeComponent,
            &ClockApp::updateClassicInfoComponent,
        }};

    if (id == logic::DashboardComponentId::Count) {
        return nullptr;
    }
    return kHandlers[static_cast<size_t>(id)];
}

void ClockApp::updateDateComponent(bool full_refresh, bool) {
    updateDateCanvas(full_refresh);
}

void ClockApp::updateBatteryComponent(bool full_refresh, bool) {
    updateBatteryCanvas(full_refresh);
}

void ClockApp::updateCalendarComponent(bool full_refresh, bool) {
    updateDashboardCalendarCanvas(full_refresh);
}

void ClockApp::updateTimeComponent(bool full_refresh, bool) {
    updateDashboardTimeCanvas(full_refresh);
}

void ClockApp::updateMarketComponent(bool full_refresh, bool allow_fetch) {
    updateDashboardSummaryCanvas(full_refresh, allow_fetch);
}

void ClockApp::updateClimateComponent(bool full_refresh, bool) {
    updateDashboardClimateCanvas(full_refresh);
}

void ClockApp::updateClassicTimeComponent(bool full_refresh, bool) {
    updateTimeCanvas(full_refresh);
}

void ClockApp::updateClassicInfoComponent(bool full_refresh, bool) {
    updateInfoCanvas(full_refresh);
}
