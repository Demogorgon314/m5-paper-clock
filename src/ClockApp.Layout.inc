// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
void ClockApp::populateSerialStatus(JsonObject data) const {
    ConfigStatusSnapshot snapshot;
    snapshot.firmware_version = FIRMWARE_VERSION;
    snapshot.firmware_build_sha = FIRMWARE_GIT_SHA;
    snapshot.firmware_build_time = FIRMWARE_BUILD_TIME;
    snapshot.page = currentPageName();
    snapshot.layout_kind = activeLayoutKind();
    snapshot.current_ssid = connectivity_.currentSsid();
    snapshot.ip_address = currentIpAddress();
    snapshot.market_code = currentMarketCode();
    snapshot.market_display_name = currentMarketDisplayName();
    snapshot.rtc = formatRtcTimestamp();
    snapshot.status_message = status_message_;
    snapshot.wifi_connected = connectivity_.isConnected();
    snapshot.battery_percent = batteryPercentage();
    snapshot.bluetooth_authorized = ble_config_session_authorized_;
    snapshot.bluetooth_pairing_active = blePairingCodeActive();
    snapshot.status_error = status_error_;
    configstatus::PopulateStatusDocument(data, settings_, snapshot);
}

void ClockApp::populateLayoutPreviewState(JsonObject data) const {
    layoutpreview::PreviewSnapshot snapshot;
    snapshot.generated_at_ms = millis();
    snapshot.layout_kind = activeLayoutKind();
    M5.RTC.getTime(&snapshot.time);
    M5.RTC.getDate(&snapshot.date);
    snapshot.wifi_connected = connectivity_.isConnected();
    snapshot.wifi_rssi = connectivity_.currentRssi();
    snapshot.battery_percent = batteryPercentage();
    snapshot.config_connection_icon = activeConfigConnectionIcon();
    snapshot.market_quote = market_quote_;
    snapshot.environment = last_environment_;
    for (const MarketLayoutItem& item : settings_.market_layout) {
        if (!item.visible) {
            continue;
        }
        MarketRenderState& state =
            const_cast<ClockApp*>(this)->marketRenderStateFor(item);
        layoutpreview::MarketPreviewItem preview_item;
        preview_item.id = item.instance_id;
        preview_item.request_symbol = state.symbol;
        preview_item.quote = state.quote;
        snapshot.markets.push_back(preview_item);
    }
    layoutpreview::PopulatePreviewDocument(data, settings_, snapshot);
}

const char* ClockApp::currentPageName() const {
    switch (current_page_) {
        case PageId::Settings:
            return "settings";
        case PageId::WifiScan:
            return "wifi_scan";
        case PageId::Password:
            return "password";
        case PageId::Clock:
        default:
            return "clock";
    }
}

const char* ClockApp::activeLayoutKind() const {
    return usesDashboardLayout() ? "dashboard" : "classic";
}

const logic::DashboardLayoutItem& ClockApp::dashboardLayoutItem(
    logic::DashboardComponentId id) const {
    const logic::DashboardLayoutItem* item =
        logic::FindDashboardLayoutItem(settings_.dashboard_layout, id);
    return item ? *item : logic::DashboardLayoutDefaultItem(id);
}

bool ClockApp::dashboardComponentVisible(logic::DashboardComponentId id) const {
    return dashboardLayoutItem(id).visible;
}

int16_t ClockApp::dashboardComponentX(logic::DashboardComponentId id) const {
    return dashboardLayoutItem(id).x;
}

int16_t ClockApp::dashboardComponentY(logic::DashboardComponentId id) const {
    return dashboardLayoutItem(id).y;
}

bool ClockApp::applyLayoutDocument(JsonObjectConst document,
                                   String& error_message) {
    layoutdoc::LayoutDocumentPlan plan;
    if (!layoutdoc::BuildLayoutDocumentPlan(document, settings_, plan,
                                            error_message)) {
        return false;
    }

    settings_.layout_presets = plan.presets;
    if (plan.active_is_classic) {
        settings_.active_layout_id = logic::kClassicLayoutId;
        if (!settings_.layout_presets.empty()) {
            store_.saveLayoutPresets(settings_.layout_presets);
        }
        store_.saveActiveLayoutId(settings_.active_layout_id);
        return true;
    }

    settings_.active_layout_id = plan.active_layout_id;
    if (plan.has_active_dashboard_draft) {
        commitLayoutApplyDraft(plan.active_dashboard_draft);
    }
    store_.saveActiveLayoutId(settings_.active_layout_id);
    store_.saveLayoutPresets(settings_.layout_presets);
    return true;
}

bool ClockApp::applyDashboardLayout(JsonArrayConst components,
                                    String& error_message,
                                    bool apply_shared_settings) {
    layoutdoc::LayoutApplyDraft draft;
    if (!layoutdoc::BuildLayoutApplyDraft(components, settings_,
                                          apply_shared_settings, draft,
                                          error_message)) {
        return false;
    }
    commitLayoutApplyDraft(draft);
    return true;
}

void ClockApp::commitLayoutApplyDraft(
    const layoutdoc::LayoutApplyDraft& draft) {
    const layoutsettings::ApplyDraftResult result =
        layoutsettings::ApplyLayoutDraft(settings_, draft);
    store_.saveDashboardLayout(settings_.dashboard_layout);
    store_.saveMarketLayout(settings_.market_layout);
    syncMarketRenderStates();
    upsertActiveDashboardPreset();
    if (result.market_changed) {
        seedMarketQuoteFromSettings();
        for (MarketRenderState& state : market_render_states_) {
            state.render_state = render::DashboardMarketRenderState {};
        }
        last_market_fetch_ms_ = 0;
        store_.saveMarket(settings_.market_symbol, settings_.market_name);
    }
    if (result.comfort_changed) {
        render_cache_.classic_info = render::ClassicInfoRenderState {};
        render_cache_.dashboard_climate = render::DashboardClimateRenderState {};
        store_.saveComfortSettings(settings_.comfort_settings);
    }
    if (result.date_changed) {
        render_cache_.date_status = render::DateStatusRenderState {};
        store_.saveDateDisplay(settings_.date_format, settings_.weekday_format,
                               settings_.date_layout, settings_.date_text_size,
                               settings_.show_holiday);
    }
}
