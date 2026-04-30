// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
#include "ClockApp.ConfigTransport.inc"

#include "ClockApp.ConfigBlePairing.inc"

#include "ClockApp.ConfigOtaCommands.inc"

void ClockApp::processConfigLine(const String& line, ConfigTransport transport) {
    if (!line.startsWith("{")) {
        return;
    }

    const ConfigConnectionIcon previous_connection_icon =
        activeConfigConnectionIcon();
    if (transport == ConfigTransport::Serial) {
        last_serial_config_at_ms_ = millis();
    }
    if (activeConfigConnectionIcon() != previous_connection_icon) {
        battery_status_dirty_ = true;
    }

    DynamicJsonDocument request_doc(12288);
    DeserializationError parse_error =
        deserializeJson(request_doc, line.c_str());
    DynamicJsonDocument response_doc(16384);
    const uint32_t request_id = request_doc["id"] | 0;
    response_doc["id"] = request_id;

    if (parse_error) {
        response_doc["ok"] = false;
        response_doc["error"] = "Invalid JSON request";
        sendConfigDoc(response_doc, transport);
        return;
    }

    const String command = request_doc["cmd"] | "";
    if (transport == ConfigTransport::Bluetooth) {
        if (!ble_config_session_authorized_) {
            authorizeBleRequest(request_doc);
        }
        if (bleCommandRequiresAuth(command) &&
            !ble_config_session_authorized_) {
            response_doc["ok"] = false;
            response_doc["error"] = "Bluetooth pairing required";
            sendConfigDoc(response_doc, transport);
            delay(20);
            if (ble_config_server_ && ble_config_conn_id_ != 0) {
                ble_config_server_->disconnect(ble_config_conn_id_);
            }
            return;
        }
    }

    if (command == "pair_begin") {
        if (transport != ConfigTransport::Bluetooth) {
            response_doc["ok"] = false;
            response_doc["error"] = "Bluetooth transport required";
        } else {
            beginBlePairing(response_doc);
        }
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "pair_verify") {
        if (transport != ConfigTransport::Bluetooth) {
            response_doc["ok"] = false;
            response_doc["error"] = "Bluetooth transport required";
        } else {
            verifyBlePairing(request_doc, response_doc);
        }
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "get_status") {
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "layout_preview_state") {
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateLayoutPreviewState(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "scan_wifi") {
        wifi_networks_ = scanWiFiNetworks(true);
        wifi_page_index_ = 0;
        status_message_ =
            wifi_networks_.empty() ? "No network found" : "Scan complete";
        status_error_ = wifi_networks_.empty();

        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        JsonArray networks = data.createNestedArray("networks");
        for (const WiFiNetwork& network : wifi_networks_) {
            JsonObject entry = networks.createNestedObject();
            entry["ssid"] = network.ssid;
            entry["rssi"] = network.rssi;
        }
        data["count"] = wifi_networks_.size();
        populateSerialStatus(data.createNestedObject("status"));
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "apply_settings") {
        JsonObject data = request_doc["data"].as<JsonObject>();
        if (data.isNull()) {
            response_doc["ok"] = false;
            response_doc["error"] = "Missing settings payload";
            sendConfigDoc(response_doc, transport);
            return;
        }

        bool settings_changed = false;
        bool wifi_changed = false;
        const bool connect_now = data["connectNow"] | false;
        const bool sync_time = data["syncTime"] | false;

        if (data.containsKey("ssid")) {
            settings_.ssid = String(data["ssid"].as<const char*>());
            settings_changed = true;
            wifi_changed = true;
        }
        if (data.containsKey("password")) {
            settings_.password = String(data["password"].as<const char*>());
            settings_changed = true;
            wifi_changed = true;
        }
        if (data.containsKey("timezone")) {
            settings_.timezone =
                logic::ClampTimeZone(data["timezone"].as<int>());
            store_.saveTimezone(settings_.timezone);
            settings_changed = true;
        }
        if (data.containsKey("partialCleanInterval")) {
            settings_.partial_clean_interval = static_cast<uint8_t>(
                logic::ClampPartialCleanInterval(
                    data["partialCleanInterval"].as<int>()));
            store_.savePartialCleanInterval(settings_.partial_clean_interval);
            settings_changed = true;
        }
        logic::ComfortSettings next_comfort_settings = settings_.comfort_settings;
        bool comfort_settings_changed = false;
        if (data.containsKey("comfortTemperatureMin")) {
            next_comfort_settings.min_temperature =
                data["comfortTemperatureMin"].as<float>();
            comfort_settings_changed = true;
        }
        if (data.containsKey("comfortTemperatureMax")) {
            next_comfort_settings.max_temperature =
                data["comfortTemperatureMax"].as<float>();
            comfort_settings_changed = true;
        }
        if (data.containsKey("comfortHumidityMin")) {
            next_comfort_settings.min_humidity =
                data["comfortHumidityMin"].as<float>();
            comfort_settings_changed = true;
        }
        if (data.containsKey("comfortHumidityMax")) {
            next_comfort_settings.max_humidity =
                data["comfortHumidityMax"].as<float>();
            comfort_settings_changed = true;
        }
        if (comfort_settings_changed) {
            settings_.comfort_settings =
                logic::NormalizeComfortSettings(next_comfort_settings);
            store_.saveComfortSettings(settings_.comfort_settings);
            settings_changed = true;
        }
        if (data.containsKey("marketSymbol")) {
            settings_.market_symbol =
                String(data["marketSymbol"].as<const char*>());
            if (data.containsKey("marketName")) {
                settings_.market_name =
                    String(data["marketName"].as<const char*>());
            } else {
                settings_.market_name =
                    marketCodeFromRequestSymbol(settings_.market_symbol);
            }
            store_.saveMarket(settings_.market_symbol, settings_.market_name);
            seedMarketQuoteFromSettings();
            last_market_summary_rendered_ = "";
            last_market_fetch_ms_ = 0;
            settings_changed = true;
        }

        if (wifi_changed) {
            store_.saveWifi(settings_.ssid, settings_.password);
            settings_.time_synced = false;
            store_.saveTimeSynced(false);
            auto_connect_attempted_ = !connect_now;
        }

        bool ok = true;
        String error_message;
        if (connect_now) {
            cancelBackgroundConnectivity();
            status_message_ = "Connecting to " + settings_.ssid + "...";
            status_error_ = false;
            ok = connectivity_.ensureConnected(settings_.ssid, settings_.password);
            if (ok) {
                auto_connect_attempted_ = true;
                status_message_ = "Connected";
                status_error_ = false;
            } else {
                status_message_ = "Wi-Fi connect failed";
                status_error_ = true;
                error_message = status_message_;
            }
        }

        if (ok && sync_time) {
            ok = trySyncTime(true);
            if (!ok) {
                error_message = status_message_;
            }
        }

        if (ok && connect_now && sync_time && current_page_ != PageId::Clock) {
            switchPage(PageId::Clock);
        } else if (settings_changed || connect_now || sync_time) {
            refreshCurrentPage();
        }

        response_doc["ok"] = ok;
        if (!ok) {
            response_doc["error"] =
                error_message.isEmpty() ? "Apply settings failed" : error_message;
        }
        JsonObject data_out = response_doc.createNestedObject("data");
        populateSerialStatus(data_out);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "search_market") {
        const JsonObject data = request_doc["data"].as<JsonObject>();
        const char* query_value = data["query"] | "";
        const String normalized_query = String(
            logic::NormalizeMarketSearchQuery(std::string(query_value)).c_str());
        std::vector<MarketSearchResult> results =
            connectivity_.isConnected() ? market_.searchMarkets(normalized_query)
                                        : MarketService::hotMarkets();
        if (!connectivity_.isConnected()) {
            results.erase(
                std::remove_if(
                    results.begin(), results.end(),
                    [&](const MarketSearchResult& result) {
                        return !marketSearchMatchesQuery(result, normalized_query);
                    }),
                results.end());
        }

        response_doc["ok"] = true;
        JsonObject data_out = response_doc.createNestedObject("data");
        data_out["query"] = normalized_query;
        data_out["wifiConnected"] = connectivity_.isConnected();
        data_out["count"] = static_cast<uint32_t>(results.size());
        if (!connectivity_.isConnected()) {
            data_out["message"] = "Wi-Fi offline, showing hot symbols only";
        }

        JsonArray entries = data_out.createNestedArray("results");
        for (const MarketSearchResult& result : results) {
            JsonObject entry = entries.createNestedObject();
            entry["requestSymbol"] = result.request_symbol;
            entry["code"] = result.code;
            entry["displayName"] = result.display_name;
            entry["kind"] = result.kind;
            entry["current"] = result.request_symbol == settings_.market_symbol;
        }
        populateSerialStatus(data_out.createNestedObject("status"));
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "set_market") {
        const JsonObject data = request_doc["data"].as<JsonObject>();
        const String request_symbol = String(data["requestSymbol"] | "");
        String display_name = String(data["displayName"] | "");
        if (request_symbol.isEmpty()) {
            response_doc["ok"] = false;
            response_doc["error"] = "Missing requestSymbol";
            sendConfigDoc(response_doc, transport);
            return;
        }

        settings_.market_symbol = request_symbol;
        if (display_name.isEmpty()) {
            display_name = marketCodeFromRequestSymbol(request_symbol);
        }
        settings_.market_name = display_name;
        seedMarketQuoteFromSettings();
        last_market_summary_rendered_ = "";
        last_market_fetch_ms_ = 0;

        if (connectivity_.isConnected()) {
            const MarketQuote fetched = market_.fetchQuote(settings_.market_symbol);
            if (fetched.valid) {
                market_quote_ = fetched;
                if (!settings_.market_name.isEmpty()) {
                    market_quote_.display_name = settings_.market_name;
                }
            } else {
                market_quote_.error_message = fetched.error_message;
            }
        }

        store_.saveMarket(settings_.market_symbol, settings_.market_name);
        if (current_page_ == PageId::Clock) {
            refreshCurrentPage();
        }

        response_doc["ok"] = true;
        JsonObject data_out = response_doc.createNestedObject("data");
        populateSerialStatus(data_out);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "apply_layout") {
        const JsonObject data = request_doc["data"].as<JsonObject>();
        String error_message;
        bool ok = true;
        if (data["reset"] | false) {
            settings_.dashboard_layout = logic::DefaultDashboardLayout();
            settings_.active_layout_id = logic::kDefaultLayoutId;
            settings_.market_layout = {MarketLayoutItem {}};
            SavedDashboardLayout preset;
            preset.id = logic::kDefaultLayoutId;
            preset.name = logic::kDefaultLayoutName;
            preset.dashboard_layout = settings_.dashboard_layout;
            preset.market_layout = settings_.market_layout;
            settings_.layout_presets = {preset};
            store_.saveDashboardLayout(settings_.dashboard_layout);
            store_.saveMarketLayout(settings_.market_layout);
            store_.saveLayoutPresets(settings_.layout_presets);
            store_.saveActiveLayoutId(settings_.active_layout_id);
        } else if (data["layoutDocument"].is<JsonObjectConst>()) {
            ok = applyLayoutDocument(data["layoutDocument"].as<JsonObjectConst>(),
                                     error_message);
        } else {
            ok = applyDashboardLayout(
                data["dashboardLayout"].as<JsonArrayConst>(), error_message);
            if (ok) {
                settings_.active_layout_id = logic::kDefaultLayoutId;
                store_.saveActiveLayoutId(settings_.active_layout_id);
            }
        }

        if (ok && current_page_ == PageId::Clock &&
            usesDashboardLayout()) {
            renderClockPage(true);
        } else if (ok) {
            refreshCurrentPage();
        }

        response_doc["ok"] = ok;
        if (!ok) {
            response_doc["error"] =
                error_message.isEmpty() ? "Apply layout failed" : error_message;
        }
        JsonObject data_out = response_doc.createNestedObject("data");
        populateSerialStatus(data_out);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "sync_time") {
        const bool ok = trySyncTime(true);
        if (ok && current_page_ != PageId::Clock &&
            connectivity_.isConnected()) {
            switchPage(PageId::Clock);
        } else {
            refreshCurrentPage();
        }

        response_doc["ok"] = ok;
        if (!ok) {
            response_doc["error"] = status_message_;
        }
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "ota_update") {
        handleRemoteOtaConfigCommand(request_doc, response_doc, transport);
        return;
    }

    if (command == "local_ota_begin") {
        handleLocalOtaBeginConfigCommand(request_doc, response_doc, transport);
        return;
    }

    if (command == "local_ota_chunk") {
        handleLocalOtaChunkConfigCommand(request_doc, response_doc, transport);
        return;
    }

    if (command == "local_ota_status") {
        handleLocalOtaStatusConfigCommand(response_doc, transport);
        return;
    }

    if (command == "local_ota_end") {
        handleLocalOtaEndConfigCommand(response_doc, transport);
        return;
    }

    if (command == "local_ota_abort") {
        handleLocalOtaAbortConfigCommand(response_doc);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "refresh_screen") {
        refreshCurrentPage();
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    if (command == "reboot") {
        pending_serial_reboot_ = true;
        pending_serial_reboot_at_ms_ = millis() + 250;
        response_doc["ok"] = true;
        response_doc["data"]["message"] = "Reboot scheduled";
        sendConfigDoc(response_doc, transport);
        return;
    }

    response_doc["ok"] = false;
    response_doc["error"] = "Unknown command";
    sendConfigDoc(response_doc, transport);
}
