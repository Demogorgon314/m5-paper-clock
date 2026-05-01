#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "AppSettingsTypes.h"

struct ConfigStatusSnapshot {
    const char* firmware_version = "dev";
    const char* firmware_build_sha = "unknown";
    const char* firmware_build_time = "unknown";
    const char* page = "settings";
    const char* layout_kind = "dashboard";
    String current_ssid;
    String ip_address;
    String market_code;
    String market_display_name;
    String rtc;
    String status_message;
    bool wifi_connected = false;
    uint8_t battery_percent = 0;
    bool bluetooth_authorized = false;
    bool bluetooth_pairing_active = false;
    bool status_error = false;
};

namespace configstatus {

void PopulateStatusDocument(JsonObject data,
                            const AppSettings& settings,
                            const ConfigStatusSnapshot& snapshot);

}  // namespace configstatus
