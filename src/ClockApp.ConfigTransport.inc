// Included by ClockApp.Config.inc. Owns USB/BLE config transport plumbing.
void ClockApp::handleSerialConfig() {
    while (Serial.available() > 0) {
        if (local_ota_.binaryMode()) {
            processLocalOtaBinaryStream();
            continue;
        }

        const char ch = static_cast<char>(Serial.read());
        String line;
        if (serial_config_rx_buffer_.append(ch, line)) {
            if (!line.isEmpty()) {
                processConfigLine(line, ConfigTransport::Serial);
            }
        }
    }
}

void ClockApp::beginBleConfig() {
    BleConfigServiceConfig config;
    config.device_name = kBleDeviceName;
    config.service_uuid = kBleServiceUuid;
    config.rx_characteristic_uuid = kBleRxCharacteristicUuid;
    config.tx_characteristic_uuid = kBleTxCharacteristicUuid;
    config.notify_chunk_size = kBleNotifyChunkSize;
    ble_config_.begin(config, this);
}

void ClockApp::handleBleConfig() {
    String line;
    while (ble_config_.pollLine(line)) {
        processConfigLine(line, ConfigTransport::Bluetooth);
    }
}

ConfigConnectionIcon ClockApp::activeConfigConnectionIcon() const {
    if (ble_config_.connected()) {
        return ConfigConnectionIcon::Bluetooth;
    }

    if (last_serial_config_at_ms_ != 0 &&
        millis() - last_serial_config_at_ms_ <= kSerialConfigActiveMs) {
        return ConfigConnectionIcon::Serial;
    }

    return ConfigConnectionIcon::None;
}

void ClockApp::onBleConfigConnected(uint16_t conn_id) {
    ble_config_session_authorized_ = false;
    battery_status_dirty_ = true;
    Serial.printf("[ble] config client connected conn=%u\n", conn_id);
}

void ClockApp::onBleConfigDisconnected() {
    ble_config_session_authorized_ = false;
    clearBlePairingPrompt();
    battery_status_dirty_ = true;
    Serial.println("[ble] config client disconnected");
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

    ble_config_.sendLine(line);
}
