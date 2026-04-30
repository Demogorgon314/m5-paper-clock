// Included by ClockApp.cpp. This is the runtime component dispatch table:
// adding a dashboard component should register its update handler here.
const ClockApp::ComponentUpdateBinding* ClockApp::componentUpdateBinding(
    logic::DashboardComponentId id) const {
    static const ComponentUpdateBinding kBindings[] = {
        {logic::DashboardComponentId::Date, &ClockApp::updateDateComponent},
        {logic::DashboardComponentId::Battery, &ClockApp::updateBatteryComponent},
        {logic::DashboardComponentId::Calendar, &ClockApp::updateCalendarComponent},
        {logic::DashboardComponentId::Time, &ClockApp::updateTimeComponent},
        {logic::DashboardComponentId::Summary, &ClockApp::updateMarketComponent},
        {logic::DashboardComponentId::Climate, &ClockApp::updateClimateComponent},
        {logic::DashboardComponentId::ClassicTime,
         &ClockApp::updateClassicTimeComponent},
        {logic::DashboardComponentId::ClassicInfo,
         &ClockApp::updateClassicInfoComponent},
    };

    for (const ComponentUpdateBinding& binding : kBindings) {
        if (binding.id == id) {
            return &binding;
        }
    }
    return nullptr;
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
