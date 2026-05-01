// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
#include "ClockApp.ConfigTransport.inc"

#include "ClockApp.ConfigBlePairing.inc"

#include "ClockApp.ConfigOtaCommands.inc"

#include "ClockApp.ConfigNetworkCommands.inc"

#include "ClockApp.ConfigSettingsCommands.inc"

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
    const logic::ConfigCommand command_kind =
        logic::ParseConfigCommand(command.c_str());
    if (transport == ConfigTransport::Bluetooth) {
        if (!ble_config_session_authorized_) {
            authorizeBleRequest(request_doc);
        }
        if (logic::ConfigCommandRequiresBleAuth(command_kind) &&
            !ble_config_session_authorized_) {
            response_doc["ok"] = false;
            response_doc["error"] = "Bluetooth pairing required";
            sendConfigDoc(response_doc, transport);
            delay(20);
            ble_config_.disconnect();
            return;
        }
    }

    if (logic::ConfigCommandRequiresBluetooth(command_kind) &&
        transport != ConfigTransport::Bluetooth) {
        response_doc["ok"] = false;
        response_doc["error"] = "Bluetooth transport required";
        sendConfigDoc(response_doc, transport);
        return;
    }

    switch (command_kind) {
    case logic::ConfigCommand::PairBegin:
        beginBlePairing(response_doc);
        sendConfigDoc(response_doc, transport);
        return;

    case logic::ConfigCommand::PairVerify:
        verifyBlePairing(request_doc, response_doc);
        sendConfigDoc(response_doc, transport);
        return;

    case logic::ConfigCommand::GetStatus: {
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    case logic::ConfigCommand::LayoutPreviewState: {
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateLayoutPreviewState(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    case logic::ConfigCommand::ScanWifi:
        handleScanWifiConfigCommand(response_doc, transport);
        return;

    case logic::ConfigCommand::ApplySettings:
        handleApplySettingsConfigCommand(request_doc, response_doc, transport);
        return;

    case logic::ConfigCommand::SearchMarket:
        handleSearchMarketConfigCommand(request_doc, response_doc, transport);
        return;

    case logic::ConfigCommand::SetMarket:
        handleSetMarketConfigCommand(request_doc, response_doc, transport);
        return;

    case logic::ConfigCommand::ApplyLayout: {
        const JsonObject data = request_doc["data"].as<JsonObject>();
        String error_message;
        bool ok = true;
        if (data["reset"] | false) {
            layoutpreset::ResetToDefaultDashboard(settings_);
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

    case logic::ConfigCommand::SyncTime: {
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

    case logic::ConfigCommand::OtaUpdate:
        handleRemoteOtaConfigCommand(request_doc, response_doc, transport);
        return;

    case logic::ConfigCommand::LocalOtaBegin:
        handleLocalOtaBeginConfigCommand(request_doc, response_doc, transport);
        return;

    case logic::ConfigCommand::LocalOtaChunk:
        handleLocalOtaChunkConfigCommand(request_doc, response_doc, transport);
        return;

    case logic::ConfigCommand::LocalOtaStatus:
        handleLocalOtaStatusConfigCommand(response_doc, transport);
        return;

    case logic::ConfigCommand::LocalOtaEnd:
        handleLocalOtaEndConfigCommand(response_doc, transport);
        return;

    case logic::ConfigCommand::LocalOtaAbort:
        handleLocalOtaAbortConfigCommand(response_doc);
        sendConfigDoc(response_doc, transport);
        return;

    case logic::ConfigCommand::RefreshScreen: {
        refreshCurrentPage();
        response_doc["ok"] = true;
        JsonObject data = response_doc.createNestedObject("data");
        populateSerialStatus(data);
        sendConfigDoc(response_doc, transport);
        return;
    }

    case logic::ConfigCommand::Reboot:
        pending_serial_reboot_ = true;
        pending_serial_reboot_at_ms_ = millis() + 250;
        response_doc["ok"] = true;
        response_doc["data"]["message"] = "Reboot scheduled";
        sendConfigDoc(response_doc, transport);
        return;

    case logic::ConfigCommand::Unknown:
    default:
        response_doc["ok"] = false;
        response_doc["error"] = "Unknown command";
        sendConfigDoc(response_doc, transport);
        return;
    }
}
