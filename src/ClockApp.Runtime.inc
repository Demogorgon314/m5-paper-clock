// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
void ClockApp::switchPage(PageId page, bool force_full_refresh) {
    const uint32_t started_ms = millis();
    logPerf("[perf] switchPage from=%d to=%d force_full=%d at=%lu\n",
                  static_cast<int>(current_page_), static_cast<int>(page),
                  force_full_refresh ? 1 : 0, started_ms);
    const PageId previous_page = current_page_;
    current_page_ = page;
    if (page == PageId::Clock && previous_page != PageId::Clock &&
        usesDashboardLayout()) {
        fast_dashboard_entry_render_ = true;
        pending_dashboard_date_cjk_render_ = false;
        pending_dashboard_calendar_cjk_render_ = false;
    }
    buttons_.clear();
    renderPage(force_full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16,
               force_full_refresh);
    logPerf("[perf] switchPage to=%d done total=%lu\n",
                  static_cast<int>(page), millis() - started_ms);
}

void ClockApp::refreshCurrentPage() {
    if (current_page_ == PageId::Clock) {
        renderClockPage(true);
        startBackgroundReconnect();
        return;
    }
    renderPage(UPDATE_MODE_GC16, true);
}

void ClockApp::startBackgroundReconnect() {
    if (current_page_ != PageId::Clock || settings_.ssid.isEmpty() ||
        connectivity_.isConnected() ||
        background_connectivity_task_ != BackgroundConnectivityTask::Idle) {
        return;
    }

    background_connectivity_task_ =
        BackgroundConnectivityTask::ReconnectScheduled;
    background_connectivity_due_ms_ = millis() + kBackgroundReconnectStartDelayMs;
}

void ClockApp::handleBackgroundConnectivity() {
    if (background_connectivity_task_ == BackgroundConnectivityTask::Idle) {
        return;
    }

    if (background_connectivity_task_ ==
        BackgroundConnectivityTask::ReconnectScheduled) {
        if (current_page_ != PageId::Clock || settings_.ssid.isEmpty() ||
            connectivity_.isConnected()) {
            cancelBackgroundConnectivity();
            return;
        }
        if (millis() < background_connectivity_due_ms_) {
            return;
        }

        const ConnectivityService::AsyncConnectState state =
            connectivity_.beginConnectAsync(settings_.ssid, settings_.password,
                                            kBackgroundReconnectTimeoutMs);
        if (state == ConnectivityService::AsyncConnectState::InProgress) {
            background_connectivity_task_ = BackgroundConnectivityTask::Reconnecting;
            return;
        }

        if (state == ConnectivityService::AsyncConnectState::Succeeded) {
            background_connectivity_task_ = BackgroundConnectivityTask::SyncingTime;
            updateBatteryCanvas(false);
            if (usesDashboardLayout()) {
                refreshMarketQuote(true);
                updateDashboardSummaryCanvas(false);
            }
            connectivity_.beginTimeSyncAsync(settings_.timezone,
                                             kBackgroundTimeSyncTimeoutMs);
            return;
        }

        cancelBackgroundConnectivity();
        return;
    }

    if (background_connectivity_task_ == BackgroundConnectivityTask::Reconnecting) {
        const ConnectivityService::AsyncConnectState state =
            connectivity_.pollConnectAsync();
        if (state == ConnectivityService::AsyncConnectState::InProgress ||
            state == ConnectivityService::AsyncConnectState::Idle) {
            return;
        }

        if (state == ConnectivityService::AsyncConnectState::Failed) {
            background_connectivity_task_ = BackgroundConnectivityTask::Idle;
            connectivity_.cancelConnectAsync();
            return;
        }

        connectivity_.cancelConnectAsync();
        background_connectivity_task_ = BackgroundConnectivityTask::SyncingTime;
        if (current_page_ == PageId::Clock) {
            updateBatteryCanvas(false);
            if (usesDashboardLayout()) {
                refreshMarketQuote(true);
                updateDashboardSummaryCanvas(false);
            }
        }
        connectivity_.beginTimeSyncAsync(settings_.timezone,
                                         kBackgroundTimeSyncTimeoutMs);
        return;
    }

    const ConnectivityService::AsyncTimeSyncState sync_state =
        connectivity_.pollTimeSyncAsync();
    if (sync_state == ConnectivityService::AsyncTimeSyncState::InProgress ||
        sync_state == ConnectivityService::AsyncTimeSyncState::Idle) {
        return;
    }

    connectivity_.cancelTimeSyncAsync();
    background_connectivity_task_ = BackgroundConnectivityTask::Idle;

    if (sync_state != ConnectivityService::AsyncTimeSyncState::Succeeded) {
        return;
    }

    settings_.time_synced = true;
    store_.saveTimeSynced(true);
    last_time_ = {};
    last_date_ = {};
    if (current_page_ == PageId::Clock) {
        updateClockPage();
    }
}

void ClockApp::cancelBackgroundConnectivity() {
    background_connectivity_task_ = BackgroundConnectivityTask::Idle;
    background_connectivity_due_ms_ = 0;
    connectivity_.cancelConnectAsync();
    connectivity_.cancelTimeSyncAsync();
}

void ClockApp::cycleClockLayout(int delta) {
    const int dashboard_count =
        static_cast<int>(settings_.layout_presets.size());
    const int cycle_count = dashboard_count + 1;
    if (cycle_count <= 1) {
        return;
    }
    int current_index = 0;
    if (usesDashboardLayout()) {
        const int preset_index = activeDashboardPresetIndex();
        current_index = preset_index >= 0 ? preset_index + 1 : 1;
    }
    int next_index = (current_index + delta) % cycle_count;
    if (next_index < 0) {
        next_index += cycle_count;
    }

    if (next_index == 0) {
        settings_.active_layout_id = logic::kClassicLayoutId;
    } else {
        const int preset_index = next_index - 1;
        applyDashboardPreset(settings_.layout_presets[preset_index]);
    }
    store_.saveActiveLayoutId(settings_.active_layout_id);

    if (current_page_ == PageId::Clock) {
        renderClockPage(true);
    }
}

bool ClockApp::usesDashboardLayout() const {
    return settings_.active_layout_id != logic::kClassicLayoutId;
}

void ClockApp::autoConnectIfNeeded() {
    if (auto_connect_attempted_ || settings_.ssid.isEmpty()) {
        return;
    }

    const uint32_t started_ms = millis();
    logPerf("[perf] autoConnectIfNeeded start attempted=%d ssid=%d at=%lu\n",
                  auto_connect_attempted_ ? 1 : 0,
                  settings_.ssid.isEmpty() ? 0 : 1, started_ms);
    auto_connect_attempted_ = true;
    status_message_ = "Connecting to " + settings_.ssid + "...";
    status_error_ = false;
    uint32_t step_started_ms = millis();
    updateSettingsStatusCanvas(UPDATE_MODE_GL16);
    logPerf("[perf] autoConnect status(connecting) took=%lu\n",
                  millis() - step_started_ms);

    step_started_ms = millis();
    if (!connectivity_.ensureConnected(settings_.ssid, settings_.password)) {
        logPerf("[perf] autoConnect ensureConnected failed after=%lu\n",
                      millis() - step_started_ms);
        status_message_ = "Wi-Fi connect failed";
        status_error_ = true;
        updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        return;
    }
    logPerf("[perf] autoConnect ensureConnected ok after=%lu\n",
                  millis() - step_started_ms);

    status_message_ = "Wi-Fi connected, syncing time...";
    step_started_ms = millis();
    updateSettingsStatusCanvas(UPDATE_MODE_GL16);
    logPerf("[perf] autoConnect status(syncing) took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    if (trySyncTime(false)) {
        logPerf("[perf] autoConnect trySyncTime ok after=%lu\n",
                      millis() - step_started_ms);
        switchPage(PageId::Clock);
        logPerf("[perf] autoConnect switchPage done total=%lu\n",
                      millis() - started_ms);
    }
}

void ClockApp::scanWiFi() {
    cancelBackgroundConnectivity();
    status_message_ = "Scanning Wi-Fi...";
    status_error_ = false;
    renderPage(UPDATE_MODE_GL16);

    wifi_networks_ = scanWiFiNetworks(false);
    wifi_page_index_ = 0;

    status_message_ = wifi_networks_.empty() ? "No network found" : "Scan complete";
    status_error_ = wifi_networks_.empty();
    renderPage(UPDATE_MODE_GC16);
}

std::vector<ClockApp::WiFiNetwork> ClockApp::scanWiFiNetworks(bool update_status) {
    cancelBackgroundConnectivity();
    if (update_status) {
        status_message_ = "Scanning Wi-Fi...";
        status_error_ = false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    std::vector<WiFiNetwork> networks;
    const int count = WiFi.scanNetworks();
    for (int index = 0; index < count; ++index) {
        WiFiNetwork network;
        network.ssid = WiFi.SSID(index);
        network.rssi = WiFi.RSSI(index);
        if (!network.ssid.isEmpty()) {
            networks.push_back(network);
        }
    }
    WiFi.scanDelete();

    std::sort(networks.begin(), networks.end(),
              [](const WiFiNetwork& left, const WiFiNetwork& right) {
                  return left.rssi > right.rssi;
              });
    networks.erase(
        std::unique(networks.begin(), networks.end(),
                    [](const WiFiNetwork& left, const WiFiNetwork& right) {
                        return left.ssid == right.ssid;
                    }),
        networks.end());
    return networks;
}

bool ClockApp::trySyncTime(bool allow_connect) {
    const uint32_t started_ms = millis();
    logPerf("[perf] trySyncTime start allow_connect=%d connected=%d at=%lu\n",
                  allow_connect ? 1 : 0, connectivity_.isConnected() ? 1 : 0,
                  started_ms);
    if (allow_connect && !connectivity_.isConnected()) {
        if (settings_.ssid.isEmpty()) {
            status_message_ = "Configure Wi-Fi first";
            status_error_ = true;
            if (current_page_ == PageId::Settings) {
                updateSettingsStatusCanvas(UPDATE_MODE_GL16);
            }
            return false;
        }
        const uint32_t connect_started_ms = millis();
        if (!connectivity_.ensureConnected(settings_.ssid, settings_.password)) {
            logPerf("[perf] trySyncTime ensureConnected failed after=%lu\n",
                          millis() - connect_started_ms);
            status_message_ = "Wi-Fi connect failed";
            status_error_ = true;
            if (current_page_ == PageId::Settings) {
                updateSettingsStatusCanvas(UPDATE_MODE_GL16);
            }
            return false;
        }
        logPerf("[perf] trySyncTime ensureConnected ok after=%lu\n",
                      millis() - connect_started_ms);
    }

    if (!connectivity_.isConnected()) {
        status_message_ = "Wi-Fi is offline";
        status_error_ = true;
        if (current_page_ == PageId::Settings) {
            updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        }
        return false;
    }

    const uint32_t sync_started_ms = millis();
    if (connectivity_.syncTimeToRtc(settings_.timezone)) {
        logPerf("[perf] trySyncTime syncTimeToRtc ok after=%lu\n",
                      millis() - sync_started_ms);
        settings_.time_synced = true;
        store_.saveTimeSynced(true);
        status_message_ = "Time sync successful";
        status_error_ = false;
        M5.RTC.getTime(&last_time_);
        M5.RTC.getDate(&last_date_);
    } else {
        logPerf("[perf] trySyncTime syncTimeToRtc failed after=%lu\n",
                      millis() - sync_started_ms);
        status_message_ = "Time sync failed";
        status_error_ = true;
    }

    if (current_page_ == PageId::Settings) {
        const uint32_t status_started_ms = millis();
        updateSettingsStatusCanvas(UPDATE_MODE_GL16);
        logPerf("[perf] trySyncTime settings status update took=%lu\n",
                      millis() - status_started_ms);
    }
    logPerf("[perf] trySyncTime done success=%d total=%lu\n",
                  status_error_ ? 0 : 1, millis() - started_ms);
    return !status_error_;
}
