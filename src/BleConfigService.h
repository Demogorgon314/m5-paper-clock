#pragma once

#include <Arduino.h>

#include <string>

#include "ConfigLineBuffer.h"

class BLECharacteristic;
class BLEServer;

struct BleConfigServiceConfig {
    const char* device_name = "";
    const char* service_uuid = "";
    const char* rx_characteristic_uuid = "";
    const char* tx_characteristic_uuid = "";
    size_t pending_max_length = 2048;
    size_t notify_chunk_size = 20;
};

class BleConfigServiceEvents {
public:
    virtual ~BleConfigServiceEvents() = default;
    virtual void onBleConfigConnected(uint16_t conn_id) = 0;
    virtual void onBleConfigDisconnected() = 0;
};

class BleConfigService {
public:
    explicit BleConfigService(size_t line_max_length = 1024)
        : line_buffer_(line_max_length) {
    }

    void begin(const BleConfigServiceConfig& config,
               BleConfigServiceEvents* events);
    bool pollLine(String& line);
    void enqueueChunk(const std::string& chunk);
    void sendLine(const String& line) const;
    void disconnect();
    void clearRx();

    bool ready() const { return ready_; }
    bool connected() const { return connected_; }
    uint16_t connectionId() const { return conn_id_; }

    void handleConnected(uint16_t conn_id);
    void handleDisconnected();

private:
    BleConfigServiceConfig config_;
    BleConfigServiceEvents* events_ = nullptr;
    ConfigLineBuffer line_buffer_;
    String pending_chunks_;
    portMUX_TYPE rx_mux_ = portMUX_INITIALIZER_UNLOCKED;
    BLECharacteristic* tx_characteristic_ = nullptr;
    BLEServer* server_ = nullptr;
    bool ready_ = false;
    bool connected_ = false;
    uint16_t conn_id_ = 0;
};
