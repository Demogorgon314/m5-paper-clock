// Included by ClockApp.Config.inc. Owns BLE config authorization and pairing.
bool ClockApp::authorizeBleRequest(const JsonDocument& request_doc) {
    if (settings_.ble_pairing_token.isEmpty()) {
        return false;
    }

    const char* auth_value = request_doc["auth"] | "";
    if (!auth_value || settings_.ble_pairing_token != String(auth_value)) {
        return false;
    }

    ble_config_session_authorized_ = true;
    return true;
}

bool ClockApp::bleCommandRequiresAuth(const String& command) const {
    return command != "pair_begin" && command != "pair_verify";
}

bool ClockApp::blePairingCodeActive() const {
    return ble_pairing_prompt_visible_ && !ble_pairing_code_.isEmpty() &&
           millis() < ble_pairing_expires_at_ms_;
}

String ClockApp::generateBlePairingCode() const {
    char code[5];
    snprintf(code, sizeof(code), "%04u",
             static_cast<unsigned>(esp_random() % 10000));
    return String(code);
}

String ClockApp::generateBlePairingToken() const {
    char token[33];
    const char* hex = "0123456789abcdef";
    for (uint8_t index = 0; index < 16; ++index) {
        const uint8_t value = static_cast<uint8_t>(esp_random() & 0xff);
        token[index * 2] = hex[value >> 4];
        token[index * 2 + 1] = hex[value & 0x0f];
    }
    token[32] = '\0';
    return String(token);
}

void ClockApp::beginBlePairing(DynamicJsonDocument& response_doc) {
    ble_pairing_code_ = generateBlePairingCode();
    ble_pairing_attempts_ = 0;
    ble_pairing_expires_at_ms_ = millis() + kBlePairingCodeTtlMs;
    ble_pairing_prompt_visible_ = true;
    status_message_ = "Bluetooth pairing code " + ble_pairing_code_;
    status_error_ = false;

    if (current_page_ == PageId::Clock) {
        last_holiday_display_rendered_ = "";
        updateDateCanvas(false);
    } else {
        refreshCurrentPage();
    }

    response_doc["ok"] = true;
    JsonObject data = response_doc.createNestedObject("data");
    data["ttlMs"] = kBlePairingCodeTtlMs;
    data["message"] = "Enter the 4-digit code shown on the device";
    populateSerialStatus(data.createNestedObject("status"));
}

void ClockApp::verifyBlePairing(const JsonDocument& request_doc,
                                DynamicJsonDocument& response_doc) {
    const JsonObjectConst data = request_doc["data"].as<JsonObjectConst>();
    const String code = String(data["code"] | "");
    if (!blePairingCodeActive()) {
        response_doc["ok"] = false;
        response_doc["error"] = "Bluetooth pairing code expired";
        return;
    }

    if (code != ble_pairing_code_) {
        ++ble_pairing_attempts_;
        if (ble_pairing_attempts_ >= kBlePairingMaxAttempts) {
            clearBlePairingPrompt();
            response_doc["error"] = "Bluetooth pairing locked";
        } else {
            response_doc["error"] = "Invalid Bluetooth pairing code";
        }
        response_doc["ok"] = false;
        return;
    }

    settings_.ble_pairing_token = generateBlePairingToken();
    store_.saveBlePairingToken(settings_.ble_pairing_token);
    ble_config_session_authorized_ = true;
    clearBlePairingPrompt();
    status_message_ = "Bluetooth paired";
    status_error_ = false;
    if (current_page_ != PageId::Clock) {
        refreshCurrentPage();
    }

    response_doc["ok"] = true;
    JsonObject data_out = response_doc.createNestedObject("data");
    data_out["token"] = settings_.ble_pairing_token;
    data_out["message"] = "Bluetooth paired";
    populateSerialStatus(data_out.createNestedObject("status"));
}

void ClockApp::clearBlePairingPrompt() {
    const bool was_visible = ble_pairing_prompt_visible_;
    ble_pairing_prompt_visible_ = false;
    ble_pairing_code_.clear();
    ble_pairing_attempts_ = 0;
    ble_pairing_expires_at_ms_ = 0;
    if (was_visible && current_page_ == PageId::Clock) {
        last_holiday_display_rendered_ = "";
        pending_dashboard_date_cjk_render_ = true;
    }
}
