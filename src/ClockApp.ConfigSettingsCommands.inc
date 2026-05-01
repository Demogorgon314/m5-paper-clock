// Included by ClockApp.Config.inc. Adapts settings config commands to services.
void ClockApp::handleApplySettingsConfigCommand(
    const JsonDocument& request_doc, DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    const JsonObjectConst data = request_doc["data"].as<JsonObjectConst>();
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
        settings_.timezone = logic::ClampTimeZone(data["timezone"].as<int>());
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
        settings_.market_symbol = String(data["marketSymbol"].as<const char*>());
        if (data.containsKey("marketName")) {
            settings_.market_name = String(data["marketName"].as<const char*>());
        } else {
            settings_.market_name =
                MarketCodeFromRequestSymbol(settings_.market_symbol);
        }
        store_.saveMarket(settings_.market_symbol, settings_.market_name);
        seedMarketQuoteFromSettings();
        for (MarketRenderState& state : market_render_states_) {
            state.render_state = render::DashboardMarketRenderState {};
        }
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
}
