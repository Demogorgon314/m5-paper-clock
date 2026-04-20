#pragma once

#include <Arduino.h>
#include <M5EPD.h>
#include <WiFi.h>

class ConnectivityService {
public:
    bool connect(const String& ssid, const String& password,
                 uint32_t timeout_ms = 10000);
    bool ensureConnected(const String& ssid, const String& password,
                         uint32_t timeout_ms = 10000);
    bool syncTimeToRtc(int8_t timezone, uint32_t timeout_ms = 10000);
    bool isConnected() const;
    String currentSsid() const;
};
