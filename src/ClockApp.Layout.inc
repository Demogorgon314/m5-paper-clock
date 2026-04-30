// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
void ClockApp::populateSerialStatus(JsonObject data) const {
    data["deviceName"] = "M5Paper Ink Clock";
    data["protocolVersion"] = 1;
    data["firmwareVersion"] = FIRMWARE_VERSION;
    data["firmwareBuildSha"] = FIRMWARE_GIT_SHA;
    data["firmwareBuildTime"] = FIRMWARE_BUILD_TIME;
    data["page"] = currentPageName();
    data["activeLayoutId"] = settings_.active_layout_id;
    data["layoutKind"] = activeLayoutKind();
    data["savedSsid"] = settings_.ssid;
    data["wifiConnected"] = connectivity_.isConnected();
    data["currentSsid"] = connectivity_.currentSsid();
    data["ipAddress"] = currentIpAddress();
    data["timezone"] = settings_.timezone;
    data["timeSynced"] = settings_.time_synced;
    data["marketSymbol"] = settings_.market_symbol;
    data["marketCode"] = currentMarketCode();
    data["marketDisplayName"] = currentMarketDisplayName();
    data["dateFormat"] = settings_.date_format;
    data["weekdayFormat"] = settings_.weekday_format;
    data["dateLayout"] = settings_.date_layout;
    data["dateTextSize"] = settings_.date_text_size;
    data["showHoliday"] = settings_.show_holiday;
    data["rtc"] = formatRtcTimestamp();
    data["batteryPercent"] = batteryPercentage();
    data["partialCleanInterval"] = settings_.partial_clean_interval;
    data["comfortTemperatureMin"] = settings_.comfort_settings.min_temperature;
    data["comfortTemperatureMax"] = settings_.comfort_settings.max_temperature;
    data["comfortHumidityMin"] = settings_.comfort_settings.min_humidity;
    data["comfortHumidityMax"] = settings_.comfort_settings.max_humidity;
    data["bluetoothPaired"] = !settings_.ble_pairing_token.isEmpty();
    data["bluetoothAuthorized"] = ble_config_session_authorized_;
    data["bluetoothPairingActive"] = blePairingCodeActive();
    data["statusMessage"] = status_message_;
    data["statusError"] = status_error_;
    populateLayoutDocument(data.createNestedObject("layoutDocument"));
}

void ClockApp::populateLayoutPreviewState(JsonObject data) const {
    rtc_time_t current_time;
    rtc_date_t current_date;
    M5.RTC.getTime(&current_time);
    M5.RTC.getDate(&current_date);

    data["protocolVersion"] = 1;
    data["generatedAtMs"] = millis();
    data["activeLayoutId"] = settings_.active_layout_id;
    data["layoutKind"] = activeLayoutKind();
    populateLayoutDocument(data.createNestedObject("layoutDocument"));

    JsonObject render_data = data.createNestedObject("renderData");

    JsonObject time = render_data.createNestedObject("time");
    time["hour"] = current_time.hour;
    time["minute"] = current_time.min;
    time["text"] = formatTimeDigits(current_time);
    time["display"] = formatTwoDigits(current_time.hour) + ":" +
                      formatTwoDigits(current_time.min);

    JsonObject date = render_data.createNestedObject("date");
    date["year"] = current_date.year;
    date["month"] = current_date.mon;
    date["day"] = current_date.day;
    date["iso"] = formatDateOnly(current_date);
    const uint8_t weekday = calculateWeekday(current_date);
    date["weekdayIndex"] = weekday;
    date["dateFormat"] = settings_.date_format;
    date["weekdayFormat"] = settings_.weekday_format;
    date["layoutStyle"] = settings_.date_layout;
    date["textSize"] = settings_.date_text_size;
    date["showHoliday"] = settings_.show_holiday;
    date["displayDate"] =
        formatConfiguredDate(current_date, settings_.date_format);
    date["weekdayLabel"] = weekdayLabel(weekday);
    date["displayWeekday"] = weekdayLabel(weekday, settings_.weekday_format);
    date["monthLabel"] = formatDashboardMonth(current_date);
    date["daysInMonth"] = daysInMonth(current_date.year, current_date.mon);
    rtc_date_t first_day = current_date;
    first_day.day = 1;
    date["firstWeekday"] = calculateWeekday(first_day);
    const logic::HolidayDisplay holiday_display =
        logic::HolidayDisplayForDate(current_date.year, current_date.mon,
                                     current_date.day);
    date["holidayText"] = settings_.show_holiday
        ? formatHolidayDisplayText(holiday_display, false)
        : String("");

    JsonObject status = render_data.createNestedObject("status");
    const bool wifi_connected = connectivity_.isConnected();
    status["batteryPercent"] = batteryPercentage();
    status["wifiConnected"] = wifi_connected;
    status["wifiSignalLevel"] =
        wifi_connected ? wifiBitmapLevel(wifiSignalLevelFromRssi(WiFi.RSSI()))
                       : 1;
    switch (activeConfigConnectionIcon()) {
        case ConfigConnectionIcon::Serial:
            status["configConnectionIcon"] = "serial";
            break;
        case ConfigConnectionIcon::Bluetooth:
            status["configConnectionIcon"] = "bluetooth";
            break;
        case ConfigConnectionIcon::None:
        default:
            status["configConnectionIcon"] = "none";
            break;
    }

    const bool market_open = logic::IsCnAShareMarketOpen(
        current_date.year, current_date.mon, current_date.day,
        current_time.hour, current_time.min);
    JsonObject market = render_data.createNestedObject("market");
    market["requestSymbol"] = settings_.market_symbol;
    market["code"] = currentMarketCode();
    market["displayName"] = marketDisplayLabel(market_quote_, true);
    market["valid"] = market_quote_.valid;
    market["price"] = market_quote_.valid ? market_quote_.price : "--";
    market["change"] = market_quote_.valid ? market_quote_.change : "";
    market["changePercent"] =
        market_quote_.valid ? market_quote_.change_percent : "";
    market["positive"] = market_quote_.positive;
    market["statusLabel"] = market_quote_.valid
        ? marketStatusLabel(market_quote_, market_open, true)
        : (!market_quote_.error_message.isEmpty()
               ? market_quote_.error_message
               : (wifi_connected ? String(u8"数据不可用") : String(u8"Wi-Fi 离线")));
    JsonArray markets = render_data.createNestedArray("markets");
    for (const MarketLayoutItem& item : settings_.market_layout) {
        if (!item.visible) {
            continue;
        }
        MarketRenderState& state =
            const_cast<ClockApp*>(this)->marketRenderStateFor(item);
        JsonObject market_item = markets.createNestedObject();
        market_item["id"] = item.instance_id;
        market_item["requestSymbol"] = state.symbol;
        market_item["code"] = state.quote.symbol;
        market_item["displayName"] = marketDisplayLabel(state.quote, true);
        market_item["valid"] = state.quote.valid;
        market_item["price"] = state.quote.valid ? state.quote.price : "--";
        market_item["change"] = state.quote.valid ? state.quote.change : "";
        market_item["changePercent"] =
            state.quote.valid ? state.quote.change_percent : "";
        market_item["positive"] = state.quote.positive;
        market_item["statusLabel"] = state.quote.valid
            ? marketStatusLabel(state.quote, market_open, true)
            : (!state.quote.error_message.isEmpty()
                   ? state.quote.error_message
                   : (wifi_connected ? String(u8"数据不可用")
                                     : String(u8"Wi-Fi 离线")));
    }

    JsonObject climate = render_data.createNestedObject("climate");
    climate["valid"] = last_environment_.valid;
    climate["humidity"] = formatDashboardHumidity(last_environment_);
    climate["temperature"] = formatDashboardTemperature(last_environment_);
    climate["rawHumidity"] = last_environment_.humidity;
    climate["rawTemperature"] = last_environment_.temperature;
    climate["comfortFace"] =
        comfortFace(last_environment_, settings_.comfort_settings);
}

void ClockApp::populateLayoutDocument(JsonObject document) const {
    const String active_layout_id = settings_.active_layout_id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : settings_.active_layout_id;
    document["activeLayoutId"] = active_layout_id;
    JsonArray layouts = document.createNestedArray("layouts");

    JsonObject classic_layout = layouts.createNestedObject();
    classic_layout["id"] = logic::kClassicLayoutId;
    classic_layout["name"] = logic::kClassicLayoutName;
    classic_layout["kind"] = "classic";
    JsonObject classic_canvas = classic_layout.createNestedObject("canvas");
    classic_canvas["width"] = logic::kLayoutScreenWidth;
    classic_canvas["height"] = logic::kLayoutScreenHeight;
    populateClassicLayoutComponents(
        classic_layout.createNestedArray("components"));

    for (const SavedDashboardLayout& preset : settings_.layout_presets) {
        JsonObject dashboard_layout = layouts.createNestedObject();
        dashboard_layout["id"] = preset.id;
        dashboard_layout["name"] = preset.name.isEmpty() ? preset.id : preset.name;
        dashboard_layout["kind"] = "dashboard";
        JsonObject dashboard_canvas =
            dashboard_layout.createNestedObject("canvas");
        dashboard_canvas["width"] = logic::kLayoutScreenWidth;
        dashboard_canvas["height"] = logic::kLayoutScreenHeight;
        populateDashboardLayoutComponents(
            dashboard_layout.createNestedArray("components"),
            preset.dashboard_layout, preset.market_layout);
    }
}

void ClockApp::populateClassicLayoutComponents(JsonArray components) const {
    for (const logic::ClassicLayoutComponentDefinition& item :
         logic::kClassicLayoutComponentDefinitions) {
        JsonObject component = components.createNestedObject();
        component["id"] = item.instance_id;
        component["type"] = item.type;
        component["x"] = item.x;
        component["y"] = item.y;
        component["w"] = item.w;
        component["h"] = item.h;
        component["visible"] = true;
        component["lockedSize"] = true;
        populateComponentProps(component.createNestedObject("props"),
                               item.source_id, item.variant);
    }
}

void ClockApp::populateDashboardLayoutComponents(
    JsonArray components,
    const logic::DashboardLayout& dashboard_layout,
    const std::vector<MarketLayoutItem>& market_layout) const {
    for (const logic::DashboardLayoutItem& item : dashboard_layout) {
        if (item.id == logic::DashboardComponentId::Summary) {
            continue;
        }
        JsonObject component = components.createNestedObject();
        component["id"] = item.instance_id;
        component["type"] = item.type;
        component["x"] = item.x;
        component["y"] = item.y;
        component["w"] = item.w;
        component["h"] = item.h;
        component["visible"] = item.visible;
        component["lockedSize"] = true;
        populateComponentProps(component.createNestedObject("props"),
                               item.id, item.variant);
    }
    for (const MarketLayoutItem& item : market_layout) {
        const logic::DashboardLayoutItem& market_default =
            logic::MarketComponentDefaultItem();
        JsonObject component = components.createNestedObject();
        component["id"] = item.instance_id;
        component["type"] = market_default.type;
        component["x"] = item.x;
        component["y"] = item.y;
        component["w"] = market_default.w;
        component["h"] = market_default.h;
        component["visible"] = item.visible;
        component["lockedSize"] = true;
        populateComponentProps(component.createNestedObject("props"),
                               market_default.id, market_default.variant,
                               item.symbol);
    }
}

void ClockApp::populateComponentProps(JsonObject props,
                                      logic::DashboardComponentId id,
                                      const char* variant,
                                      const String& symbol) const {
    layoutdoc::PopulateComponentProps(
        props, layoutdoc::SharedSettingsFromAppSettings(settings_), id, variant,
        symbol);
}

void ClockApp::populateLayoutComponents(JsonArray components) const {
    populateDashboardLayoutComponents(components, settings_.dashboard_layout,
                                      settings_.market_layout);
}

void ClockApp::sendConfigDoc(const JsonDocument& doc,
                             ConfigTransport transport) const {
    String line = "@cfg:";
    serializeJson(doc, line);
    line += "\n";
    sendConfigLine(line, transport);
}

void ClockApp::sendConfigLine(const String& line,
                              ConfigTransport transport) const {
    if (transport == ConfigTransport::Serial) {
        Serial.print(line);
        return;
    }

    if (!ble_config_client_connected_ || !ble_config_tx_characteristic_) {
        return;
    }

    for (uint32_t offset = 0; offset < line.length();
         offset += kBleNotifyChunkSize) {
        const String chunk = line.substring(
            offset,
            std::min<uint32_t>(offset + kBleNotifyChunkSize, line.length()));
        std::vector<uint8_t> chunk_bytes(chunk.begin(), chunk.end());
        ble_config_tx_characteristic_->setValue(chunk_bytes.data(),
                                                chunk_bytes.size());
        ble_config_tx_characteristic_->notify();
        delay(4);
    }
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
    if (document.isNull()) {
        error_message = "Missing layout document";
        return false;
    }

    const String active_layout_id =
        String(document["activeLayoutId"] | logic::kDefaultLayoutId);
    JsonArrayConst layouts = document["layouts"].as<JsonArrayConst>();
    if (layouts.isNull()) {
        error_message = "Missing layouts";
        return false;
    }

    bool active_dashboard_found = active_layout_id == logic::kClassicLayoutId;
    uint8_t saved_dashboard_count = 0;
    std::vector<SavedDashboardLayout> next_presets;
    const std::vector<SavedDashboardLayout> previous_presets =
        settings_.layout_presets;
    const String previous_active_layout_id = settings_.active_layout_id;
    const logic::DashboardLayout previous_dashboard_layout =
        settings_.dashboard_layout;
    const std::vector<MarketLayoutItem> previous_market_layout =
        settings_.market_layout;

    for (JsonVariantConst value : layouts) {
        JsonObjectConst layout = value.as<JsonObjectConst>();
        const String layout_id = String(layout["id"] | "");
        const String layout_kind = String(layout["kind"] | "dashboard");
        if (layout_id.isEmpty() || layout_id == logic::kClassicLayoutId ||
            layout_kind == "classic") {
            continue;
        }
        if (saved_dashboard_count >= 5 && layout_id != active_layout_id) {
            continue;
        }
        if (saved_dashboard_count >= 5 && layout_id == active_layout_id &&
            !next_presets.empty()) {
            next_presets.pop_back();
            --saved_dashboard_count;
        }
        settings_.layout_presets = next_presets;
        settings_.active_layout_id = layout_id;
        if (!applyDashboardLayout(layout["components"].as<JsonArrayConst>(),
                                  error_message,
                                  layout_id == active_layout_id)) {
            settings_.layout_presets = previous_presets;
            settings_.active_layout_id = previous_active_layout_id;
            settings_.dashboard_layout = previous_dashboard_layout;
            settings_.market_layout = previous_market_layout;
            return false;
        }
        upsertActiveDashboardPreset(String(layout["name"] | layout_id));
        next_presets = settings_.layout_presets;
        ++saved_dashboard_count;
        if (layout_id == active_layout_id) {
            active_dashboard_found = true;
        }
    }
    settings_.layout_presets = next_presets;

    if (active_layout_id == logic::kClassicLayoutId) {
        settings_.active_layout_id = logic::kClassicLayoutId;
        if (!settings_.layout_presets.empty()) {
            store_.saveLayoutPresets(settings_.layout_presets);
        }
        store_.saveActiveLayoutId(settings_.active_layout_id);
        return true;
    }

    if (!active_dashboard_found) {
        error_message = "Active layout not found";
        settings_.layout_presets = previous_presets;
        settings_.active_layout_id = previous_active_layout_id;
        settings_.dashboard_layout = previous_dashboard_layout;
        settings_.market_layout = previous_market_layout;
        return false;
    }
    settings_.active_layout_id = active_layout_id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : active_layout_id;
    const int active_index = activeDashboardPresetIndex();
    if (active_index >= 0) {
        applyDashboardPreset(settings_.layout_presets[active_index]);
    }
    store_.saveActiveLayoutId(settings_.active_layout_id);
    store_.saveLayoutPresets(settings_.layout_presets);
    return true;
}

bool ClockApp::applyDashboardLayout(JsonArrayConst components,
                                    String& error_message,
                                    bool apply_shared_settings) {
    if (components.isNull()) {
        error_message = "Missing dashboard layout components";
        return false;
    }

    layoutdoc::LayoutApplyDraft draft =
        layoutdoc::CreateLayoutApplyDraft(settings_);
    const layoutdoc::LayoutSharedSettings current_settings =
        layoutdoc::SharedSettingsFromAppSettings(settings_);
    for (JsonVariantConst value : components) {
        if (!layoutdoc::DecodeDashboardComponent(
                value.as<JsonObjectConst>(), current_settings, draft,
                apply_shared_settings, error_message)) {
            return false;
        }
    }

    layoutdoc::FinalizeLayoutApplyDraft(draft);
    commitLayoutApplyDraft(draft);
    return true;
}

void ClockApp::commitLayoutApplyDraft(
    const layoutdoc::LayoutApplyDraft& draft) {
    settings_.dashboard_layout = draft.dashboard_layout;
    store_.saveDashboardLayout(settings_.dashboard_layout);
    settings_.market_layout = draft.market_layout;
    store_.saveMarketLayout(settings_.market_layout);
    syncMarketRenderStates();
    upsertActiveDashboardPreset();
    if (!draft.market_symbol.isEmpty() &&
        draft.market_symbol != settings_.market_symbol) {
        settings_.market_symbol = draft.market_symbol;
        const String market_code =
            marketCodeFromRequestSymbol(settings_.market_symbol);
        const std::string known_name =
            logic::KnownMarketDisplayName(std::string(market_code.c_str()));
        settings_.market_name = known_name.empty()
            ? market_code
            : String(known_name.c_str());
        seedMarketQuoteFromSettings();
        last_market_summary_rendered_ = "";
        last_market_fetch_ms_ = 0;
        store_.saveMarket(settings_.market_symbol, settings_.market_name);
    }
    if (draft.comfort_settings_changed) {
        settings_.comfort_settings =
            logic::NormalizeComfortSettings(draft.comfort_settings);
        last_comfort_face_rendered_ = "";
        last_dashboard_comfort_face_rendered_ = "";
        store_.saveComfortSettings(settings_.comfort_settings);
    }
    if (draft.date_settings_changed &&
        (draft.date_format != settings_.date_format ||
         draft.weekday_format != settings_.weekday_format ||
         draft.date_layout != settings_.date_layout ||
         draft.date_text_size != settings_.date_text_size ||
         draft.show_holiday != settings_.show_holiday)) {
        settings_.date_format = draft.date_format;
        settings_.weekday_format = draft.weekday_format;
        settings_.date_layout = draft.date_layout;
        settings_.date_text_size = draft.date_text_size;
        settings_.show_holiday = draft.show_holiday;
        last_date_text_rendered_ = "";
        last_holiday_display_rendered_ = "";
        last_weekday_rendered_ = 255;
        store_.saveDateDisplay(settings_.date_format, settings_.weekday_format,
                               settings_.date_layout, settings_.date_text_size,
                               settings_.show_holiday);
    }
}
