// Included by ClockApp.Config.inc. Owns config protocol commands for OTA.
void ClockApp::handleRemoteOtaConfigCommand(
    const JsonDocument& request_doc,
    DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    const JsonObjectConst data = request_doc["data"].as<JsonObjectConst>();
    const String url = String(data["url"] | "");
    const String sha256 = String(data["sha256"] | "");
    status_message_ = "OTA update downloading";
    status_error_ = false;

    String error_message;
    const bool ok = performOtaUpdate(url, sha256, transport, error_message);
    response_doc["ok"] = ok;
    if (ok) {
        status_message_ = "OTA update installed";
        status_error_ = false;
        pending_serial_reboot_ = true;
        pending_serial_reboot_at_ms_ = millis() + 750;
        response_doc["data"]["message"] = "OTA installed, reboot scheduled";
    } else {
        status_message_ = error_message.isEmpty() ? "OTA update failed"
                                                  : error_message;
        status_error_ = true;
        response_doc["error"] = status_message_;
    }
    JsonObject data_out = response_doc["data"].is<JsonObject>()
                              ? response_doc["data"].as<JsonObject>()
                              : response_doc.createNestedObject("data");
    populateSerialStatus(data_out.createNestedObject("status"));
    sendConfigDoc(response_doc, transport);
}

void ClockApp::handleLocalOtaBeginConfigCommand(
    const JsonDocument& request_doc,
    DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    if (transport != ConfigTransport::Serial) {
        response_doc["ok"] = false;
        response_doc["error"] = "Local OTA upload requires USB";
        sendConfigDoc(response_doc, transport);
        return;
    }

    const JsonObjectConst data = request_doc["data"].as<JsonObjectConst>();
    const size_t size = static_cast<size_t>(data["size"] | 0);
    const String sha256 = String(data["sha256"] | "");
    const bool binary_mode = String(data["mode"] | "") == "binary";

    String error_message;
    const bool ok =
        beginLocalOtaUpdate(size, sha256, binary_mode, error_message);
    response_doc["ok"] = ok;
    if (ok) {
        const LocalOtaStatus status = local_ota_.status();
        response_doc["data"]["message"] = "Local OTA upload ready";
        response_doc["data"]["size"] = status.size;
        response_doc["data"]["written"] = status.written;
        response_doc["data"]["mode"] = status.binary_mode ? "binary" : "json";
    } else {
        response_doc["error"] = error_message;
    }
    sendConfigDoc(response_doc, transport);
}

void ClockApp::handleLocalOtaChunkConfigCommand(
    const JsonDocument& request_doc,
    DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    if (transport != ConfigTransport::Serial) {
        response_doc["ok"] = false;
        response_doc["error"] = "Local OTA upload requires USB";
        sendConfigDoc(response_doc, transport);
        return;
    }

    const JsonObjectConst data = request_doc["data"].as<JsonObjectConst>();
    const size_t offset = static_cast<size_t>(data["offset"] | 0);
    const String base64_data = String(data["data"] | "");

    String error_message;
    size_t written = local_ota_.status().written;
    const bool ok =
        writeLocalOtaChunk(offset, base64_data, written, error_message);
    response_doc["ok"] = ok;
    if (ok) {
        const LocalOtaStatus status = local_ota_.status();
        response_doc["data"]["written"] = written;
        response_doc["data"]["size"] = status.size;
    } else {
        const LocalOtaStatus status = local_ota_.status();
        response_doc["error"] = error_message;
        response_doc["data"]["active"] = status.active;
        response_doc["data"]["written"] = status.written;
        response_doc["data"]["size"] = status.size;
    }
    sendConfigDoc(response_doc, transport);
}

void ClockApp::handleLocalOtaStatusConfigCommand(
    DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    if (transport != ConfigTransport::Serial) {
        response_doc["ok"] = false;
        response_doc["error"] = "Local OTA upload requires USB";
        sendConfigDoc(response_doc, transport);
        return;
    }

    response_doc["ok"] = true;
    populateLocalOtaStatus(response_doc.createNestedObject("data"));
    sendConfigDoc(response_doc, transport);
}

void ClockApp::handleLocalOtaEndConfigCommand(
    DynamicJsonDocument& response_doc,
    ConfigTransport transport) {
    if (transport != ConfigTransport::Serial) {
        response_doc["ok"] = false;
        response_doc["error"] = "Local OTA upload requires USB";
        sendConfigDoc(response_doc, transport);
        return;
    }

    String error_message;
    const bool ok = finishLocalOtaUpdate(error_message);
    response_doc["ok"] = ok;
    if (ok) {
        status_message_ = "Local OTA installed";
        status_error_ = false;
        pending_serial_reboot_ = true;
        pending_serial_reboot_at_ms_ = millis() + 750;
        response_doc["data"]["message"] =
            "Local OTA installed, reboot scheduled";
    } else {
        status_message_ = error_message.isEmpty() ? "Local OTA failed"
                                                  : error_message;
        status_error_ = true;
        response_doc["error"] = status_message_;
    }
    JsonObject data_out = response_doc["data"].is<JsonObject>()
                              ? response_doc["data"].as<JsonObject>()
                              : response_doc.createNestedObject("data");
    populateSerialStatus(data_out.createNestedObject("status"));
    sendConfigDoc(response_doc, transport);
}

void ClockApp::handleLocalOtaAbortConfigCommand(
    DynamicJsonDocument& response_doc) {
    abortLocalOtaUpdate();
    response_doc["ok"] = true;
    response_doc["data"]["message"] = "Local OTA upload aborted";
}
