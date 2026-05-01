#include "BleConfigService.h"

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <algorithm>
#include <vector>

class BleConfigServerCallbacks : public BLEServerCallbacks {
public:
    explicit BleConfigServerCallbacks(BleConfigService* service)
        : service_(service) {
    }

    void onConnect(BLEServer* server) override {
        if (!service_) {
            return;
        }
        const uint16_t conn_id = server ? server->getConnId() : 0;
        service_->handleConnected(conn_id);
    }

    void onConnect(BLEServer*, esp_ble_gatts_cb_param_t* param) override {
        if (!service_) {
            return;
        }
        const uint16_t conn_id = param ? param->connect.conn_id : 0;
        service_->handleConnected(conn_id);
    }

    void onDisconnect(BLEServer*) override {
        if (service_) {
            service_->handleDisconnected();
        }
    }

private:
    BleConfigService* service_;
};

class BleConfigRxCallbacks : public BLECharacteristicCallbacks {
public:
    explicit BleConfigRxCallbacks(BleConfigService* service)
        : service_(service) {
    }

    void onWrite(BLECharacteristic* characteristic) override {
        if (!service_ || !characteristic) {
            return;
        }
        service_->enqueueChunk(characteristic->getValue());
    }

private:
    BleConfigService* service_;
};

void BleConfigService::begin(const BleConfigServiceConfig& config,
                             BleConfigServiceEvents* events) {
    if (ready_) {
        return;
    }

    config_ = config;
    events_ = events;

    BLEDevice::init(config_.device_name);
    server_ = BLEDevice::createServer();
    server_->setCallbacks(new BleConfigServerCallbacks(this));

    BLEService* service = server_->createService(config_.service_uuid);
    tx_characteristic_ = service->createCharacteristic(
        config_.tx_characteristic_uuid,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    tx_characteristic_->addDescriptor(new BLE2902());

    BLECharacteristic* rx_characteristic = service->createCharacteristic(
        config_.rx_characteristic_uuid,
        BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);
    rx_characteristic->setCallbacks(new BleConfigRxCallbacks(this));

    service->start();
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(config_.service_uuid);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    ready_ = true;
    Serial.println("[ble] config service ready");
}

bool BleConfigService::pollLine(String& line) {
    if (pending_chunks_.isEmpty()) {
        return false;
    }

    portENTER_CRITICAL(&rx_mux_);
    const String pending = pending_chunks_;
    pending_chunks_.clear();
    portEXIT_CRITICAL(&rx_mux_);

    for (uint32_t index = 0; index < pending.length(); ++index) {
        if (line_buffer_.append(pending.charAt(index), line) &&
            !line.isEmpty()) {
            return true;
        }
    }
    return false;
}

void BleConfigService::enqueueChunk(const std::string& chunk) {
    if (chunk.empty()) {
        return;
    }

    portENTER_CRITICAL(&rx_mux_);
    if (pending_chunks_.length() + chunk.size() > config_.pending_max_length) {
        pending_chunks_.clear();
        line_buffer_.clear();
        portEXIT_CRITICAL(&rx_mux_);
        Serial.println("[ble] config RX overflow");
        return;
    }

    for (char ch : chunk) {
        pending_chunks_ += ch;
    }
    portEXIT_CRITICAL(&rx_mux_);
}

void BleConfigService::sendLine(const String& line) const {
    if (!connected_ || !tx_characteristic_) {
        return;
    }

    const size_t chunk_size = config_.notify_chunk_size;
    for (size_t offset = 0; offset < line.length(); offset += chunk_size) {
        const size_t end = std::min(offset + chunk_size, line.length());
        const String chunk = line.substring(offset, end);
        std::vector<uint8_t> chunk_bytes(chunk.begin(), chunk.end());
        tx_characteristic_->setValue(chunk_bytes.data(), chunk_bytes.size());
        tx_characteristic_->notify();
        delay(5);
    }
}

void BleConfigService::disconnect() {
    if (server_ && conn_id_ != 0) {
        server_->disconnect(conn_id_);
    }
}

void BleConfigService::clearRx() {
    portENTER_CRITICAL(&rx_mux_);
    pending_chunks_.clear();
    line_buffer_.clear();
    portEXIT_CRITICAL(&rx_mux_);
}

void BleConfigService::handleConnected(uint16_t conn_id) {
    conn_id_ = conn_id;
    connected_ = true;
    if (events_) {
        events_->onBleConfigConnected(conn_id_);
    }
}

void BleConfigService::handleDisconnected() {
    connected_ = false;
    conn_id_ = 0;
    if (events_) {
        events_->onBleConfigDisconnected();
    }
    BLEDevice::startAdvertising();
}
