#include "ConnectivityService.h"

#include <M5EPD.h>
#include <WiFi.h>

#include <algorithm>

namespace {
constexpr const char* kNtpServer = "time.cloudflare.com";

bool isUsableIp(const IPAddress& ip) {
    return ip != IPAddress(0, 0, 0, 0);
}
}

void ConnectivityService::applyRtcTime(const tm& time_info) const {
    rtc_time_t rtc_time;
    rtc_time.hour = time_info.tm_hour;
    rtc_time.min = time_info.tm_min;
    rtc_time.sec = time_info.tm_sec;
    M5.RTC.setTime(&rtc_time);

    rtc_date_t rtc_date;
    rtc_date.week = time_info.tm_wday;
    rtc_date.mon = time_info.tm_mon + 1;
    rtc_date.day = time_info.tm_mday;
    rtc_date.year = time_info.tm_year + 1900;
    M5.RTC.setDate(&rtc_date);
}

bool ConnectivityService::connect(const String& ssid, const String& password,
                                  uint32_t timeout_ms) {
    if (ssid.isEmpty()) {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout_ms) {
        delay(50);
        M5.update();
    }
    const bool connected = WiFi.status() == WL_CONNECTED;
    if (connected) {
        ensureDnsConfigured();
        logNetworkDiagnostics("connected");
    }
    return connected;
}

bool ConnectivityService::ensureConnected(const String& ssid,
                                          const String& password,
                                          uint32_t timeout_ms) {
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        ensureDnsConfigured();
        return true;
    }
    return connect(ssid, password, timeout_ms);
}

bool ConnectivityService::syncTimeToRtc(int8_t timezone, uint32_t timeout_ms) {
    configTime(static_cast<long>(timezone) * 3600, 0, kNtpServer);

    struct tm time_info;
    if (!getLocalTime(&time_info, timeout_ms)) {
        return false;
    }
    applyRtcTime(time_info);
    return true;
}

ConnectivityService::AsyncConnectState ConnectivityService::beginConnectAsync(
    const String& ssid, const String& password, uint32_t timeout_ms) {
    cancelConnectAsync();
    if (ssid.isEmpty()) {
        async_connect_state_ = AsyncConnectState::Failed;
        return async_connect_state_;
    }

    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        async_connect_state_ = AsyncConnectState::Succeeded;
        return async_connect_state_;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());

    async_connect_ssid_ = ssid;
    async_connect_started_ms_ = millis();
    async_connect_timeout_ms_ = timeout_ms;
    async_connect_state_ = AsyncConnectState::InProgress;
    return async_connect_state_;
}

ConnectivityService::AsyncConnectState ConnectivityService::pollConnectAsync() {
    if (async_connect_state_ != AsyncConnectState::InProgress) {
        return async_connect_state_;
    }

    if (WiFi.status() == WL_CONNECTED &&
        (async_connect_ssid_.isEmpty() || WiFi.SSID() == async_connect_ssid_)) {
        ensureDnsConfigured();
        logNetworkDiagnostics("async connected");
        async_connect_state_ = AsyncConnectState::Succeeded;
        async_connect_ssid_.clear();
        return async_connect_state_;
    }

    if (millis() - async_connect_started_ms_ >= async_connect_timeout_ms_) {
        async_connect_state_ = AsyncConnectState::Failed;
        async_connect_ssid_.clear();
    }
    return async_connect_state_;
}

void ConnectivityService::cancelConnectAsync() {
    async_connect_state_ = AsyncConnectState::Idle;
    async_connect_ssid_.clear();
    async_connect_started_ms_ = 0;
    async_connect_timeout_ms_ = 0;
}

ConnectivityService::AsyncTimeSyncState ConnectivityService::beginTimeSyncAsync(
    int8_t timezone, uint32_t timeout_ms) {
    cancelTimeSyncAsync();
    if (!isConnected()) {
        async_time_sync_state_ = AsyncTimeSyncState::Failed;
        return async_time_sync_state_;
    }

    configTime(static_cast<long>(timezone) * 3600, 0, kNtpServer);
    async_time_sync_started_ms_ = millis();
    async_time_sync_timeout_ms_ = timeout_ms;
    async_time_sync_state_ = AsyncTimeSyncState::InProgress;
    return async_time_sync_state_;
}

ConnectivityService::AsyncTimeSyncState ConnectivityService::pollTimeSyncAsync() {
    if (async_time_sync_state_ != AsyncTimeSyncState::InProgress) {
        return async_time_sync_state_;
    }

    if (!isConnected()) {
        async_time_sync_state_ = AsyncTimeSyncState::Failed;
        return async_time_sync_state_;
    }

    struct tm time_info;
    if (getLocalTime(&time_info, 0)) {
        applyRtcTime(time_info);
        async_time_sync_state_ = AsyncTimeSyncState::Succeeded;
        return async_time_sync_state_;
    }

    if (millis() - async_time_sync_started_ms_ >= async_time_sync_timeout_ms_) {
        async_time_sync_state_ = AsyncTimeSyncState::Failed;
    }
    return async_time_sync_state_;
}

void ConnectivityService::cancelTimeSyncAsync() {
    async_time_sync_state_ = AsyncTimeSyncState::Idle;
    async_time_sync_started_ms_ = 0;
    async_time_sync_timeout_ms_ = 0;
}

bool ConnectivityService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String ConnectivityService::currentSsid() const {
    return isConnected() ? WiFi.SSID() : String("");
}

String ConnectivityService::currentIpAddress() const {
    return isConnected() ? WiFi.localIP().toString() : String("");
}

int32_t ConnectivityService::currentRssi() const {
    return isConnected() ? WiFi.RSSI() : -100;
}

void ConnectivityService::ensureDnsConfigured() const {
    if (!isConnected()) {
        return;
    }

    const IPAddress primary_dns = WiFi.dnsIP(0);
    if (isUsableIp(primary_dns)) {
        return;
    }

    const IPAddress gateway = WiFi.gatewayIP();
    const IPAddress fallback_dns(1, 1, 1, 1);
    const IPAddress dns1 = isUsableIp(gateway) ? gateway : fallback_dns;
    WiFi.config(WiFi.localIP(), gateway, WiFi.subnetMask(), dns1,
                fallback_dns);
}

void ConnectivityService::logNetworkDiagnostics(const char* phase) const {
    if (!isConnected()) {
        Serial.printf("[wifi] %s disconnected status=%d\n", phase,
                      static_cast<int>(WiFi.status()));
        return;
    }

    Serial.printf(
        "[wifi] %s ssid=%s ip=%s gateway=%s subnet=%s dns1=%s dns2=%s "
        "rssi=%d\n",
        phase, WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(),
        WiFi.gatewayIP().toString().c_str(),
        WiFi.subnetMask().toString().c_str(),
        WiFi.dnsIP(0).toString().c_str(), WiFi.dnsIP(1).toString().c_str(),
        WiFi.RSSI());
}

std::vector<WiFiNetwork> ConnectivityService::scanNetworks() const {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    std::vector<WiFiNetwork> networks;
    const int count = WiFi.scanNetworks();
    for (int index = 0; index < count; ++index) {
        WiFiNetwork network;
        network.ssid = WiFi.SSID(index);
        network.rssi = WiFi.RSSI(index);
        if (!network.ssid.isEmpty()) {
            networks.push_back(network);
        }
    }
    WiFi.scanDelete();

    std::sort(networks.begin(), networks.end(),
              [](const WiFiNetwork& left, const WiFiNetwork& right) {
                  return left.rssi > right.rssi;
              });
    networks.erase(
        std::unique(networks.begin(), networks.end(),
                    [](const WiFiNetwork& left, const WiFiNetwork& right) {
                        return left.ssid == right.ssid;
                    }),
        networks.end());
    return networks;
}
