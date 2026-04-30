// Included by ClockApp.Config.inc. Owns USB/BLE config transport plumbing.
void ClockApp::handleSerialConfig() {
    while (Serial.available() > 0) {
        if (local_ota_binary_mode_) {
            processLocalOtaBinaryStream();
            continue;
        }

        const char ch = static_cast<char>(Serial.read());
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            String line = serial_config_rx_buffer_;
            serial_config_rx_buffer_.clear();
            line.trim();
            if (!line.isEmpty()) {
                processConfigLine(line, ConfigTransport::Serial);
            }
            continue;
        }

        if (serial_config_rx_buffer_.length() < 8192) {
            serial_config_rx_buffer_ += ch;
        }
    }
}

void ClockApp::beginBleConfig() {
    if (ble_config_ready_) {
        return;
    }

    BLEDevice::init(kBleDeviceName);
    BLEServer* server = BLEDevice::createServer();
    ble_config_server_ = server;
    server->setCallbacks(new BleConfigServerCallbacks(this));

    BLEService* service = server->createService(kBleServiceUuid);
    ble_config_tx_characteristic_ = service->createCharacteristic(
        kBleTxCharacteristicUuid,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    ble_config_tx_characteristic_->addDescriptor(new BLE2902());

    BLECharacteristic* rx_characteristic = service->createCharacteristic(
        kBleRxCharacteristicUuid,
        BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);
    rx_characteristic->setCallbacks(new BleConfigRxCallbacks(this));

    service->start();
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(kBleServiceUuid);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    ble_config_ready_ = true;
    Serial.println("[ble] config service ready");
}

void ClockApp::enqueueBleConfigChunk(const std::string& chunk) {
    if (chunk.empty()) {
        return;
    }

    portENTER_CRITICAL(&ble_config_rx_mux_);
    if (ble_config_pending_chunks_.length() + chunk.size() > 2048) {
        ble_config_pending_chunks_.clear();
        ble_config_rx_buffer_.clear();
        portEXIT_CRITICAL(&ble_config_rx_mux_);
        Serial.println("[ble] config RX overflow");
        return;
    }

    for (char ch : chunk) {
        ble_config_pending_chunks_ += ch;
    }
    portEXIT_CRITICAL(&ble_config_rx_mux_);
}

void ClockApp::handleBleConfig() {
    if (ble_config_pending_chunks_.isEmpty()) {
        return;
    }

    portENTER_CRITICAL(&ble_config_rx_mux_);
    const String pending = ble_config_pending_chunks_;
    ble_config_pending_chunks_.clear();
    portEXIT_CRITICAL(&ble_config_rx_mux_);

    for (uint32_t index = 0; index < pending.length(); ++index) {
        const char ch = pending.charAt(index);
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            String line = ble_config_rx_buffer_;
            ble_config_rx_buffer_.clear();
            line.trim();
            if (!line.isEmpty()) {
                processConfigLine(line, ConfigTransport::Bluetooth);
            }
            continue;
        }

        if (ble_config_rx_buffer_.length() < 1024) {
            ble_config_rx_buffer_ += ch;
        }
    }
}

ClockApp::ConfigConnectionIcon ClockApp::activeConfigConnectionIcon() const {
    if (ble_config_client_connected_) {
        return ConfigConnectionIcon::Bluetooth;
    }

    if (last_serial_config_at_ms_ != 0 &&
        millis() - last_serial_config_at_ms_ <= kSerialConfigActiveMs) {
        return ConfigConnectionIcon::Serial;
    }

    return ConfigConnectionIcon::None;
}
