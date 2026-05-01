// Included by ClockApp.Config.inc. Adapts network config commands to services.
void ClockApp::handleScanWifiConfigCommand(DynamicJsonDocument& response_doc,
                                           ConfigTransport transport) {
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
}

void ClockApp::handleSearchMarketConfigCommand(
    const JsonDocument& request_doc, DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    const JsonObjectConst data = request_doc["data"].as<JsonObjectConst>();
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
                    return !MarketSearchMatchesQuery(result, normalized_query);
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
}

void ClockApp::handleSetMarketConfigCommand(
    const JsonDocument& request_doc, DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    const JsonObjectConst data = request_doc["data"].as<JsonObjectConst>();
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
        display_name = MarketCodeFromRequestSymbol(request_symbol);
    }
    settings_.market_name = display_name;
    seedMarketQuoteFromSettings();
    for (MarketRenderState& state : market_render_states_) {
        state.render_state = render::DashboardMarketRenderState {};
    }
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
}
