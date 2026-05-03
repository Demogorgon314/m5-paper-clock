#pragma once

#include <Arduino.h>
#include <vector>

struct WiFiNetwork {
    String ssid;
    int32_t rssi = -100;
};

class ConnectivityService {
public:
    enum class AsyncConnectState : uint8_t {
        Idle = 0,
        InProgress,
        Succeeded,
        Failed,
    };

    enum class AsyncTimeSyncState : uint8_t {
        Idle = 0,
        InProgress,
        Succeeded,
        Failed,
    };

    bool connect(const String& ssid, const String& password,
                 uint32_t timeout_ms = 10000);
    bool ensureConnected(const String& ssid, const String& password,
                         uint32_t timeout_ms = 10000);
    bool syncTimeToRtc(int8_t timezone, uint32_t timeout_ms = 10000);
    AsyncConnectState beginConnectAsync(const String& ssid, const String& password,
                                        uint32_t timeout_ms = 3000);
    AsyncConnectState pollConnectAsync();
    void cancelConnectAsync();
    AsyncTimeSyncState beginTimeSyncAsync(int8_t timezone,
                                          uint32_t timeout_ms = 3000);
    AsyncTimeSyncState pollTimeSyncAsync();
    void cancelTimeSyncAsync();
    bool isConnected() const;
    String currentSsid() const;
    String currentIpAddress() const;
    int32_t currentRssi() const;
    void ensureDnsConfigured() const;
    void logNetworkDiagnostics(const char* phase) const;
    std::vector<WiFiNetwork> scanNetworks() const;

private:
    void applyRtcTime(const tm& time_info) const;

    AsyncConnectState async_connect_state_ = AsyncConnectState::Idle;
    AsyncTimeSyncState async_time_sync_state_ = AsyncTimeSyncState::Idle;
    String async_connect_ssid_;
    uint32_t async_connect_started_ms_ = 0;
    uint32_t async_connect_timeout_ms_ = 0;
    uint32_t async_time_sync_started_ms_ = 0;
    uint32_t async_time_sync_timeout_ms_ = 0;
};
