#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <M5EPD.h>

#include <vector>

#include "AppSettingsTypes.h"
#include "ConfigTypes.h"
#include "MarketService.h"
#include "SensorService.h"

namespace layoutpreview {

struct MarketPreviewItem {
    String id;
    String request_symbol;
    MarketQuote quote;
};

struct PreviewSnapshot {
    uint32_t generated_at_ms = 0;
    const char* layout_kind = "dashboard";
    rtc_time_t time {};
    rtc_date_t date {};
    bool wifi_connected = false;
    int32_t wifi_rssi = -100;
    uint8_t battery_percent = 0;
    ConfigConnectionIcon config_connection_icon = ConfigConnectionIcon::None;
    MarketQuote market_quote;
    std::vector<MarketPreviewItem> markets;
    EnvironmentReading environment;
};

void PopulatePreviewDocument(JsonObject data,
                             const AppSettings& settings,
                             const PreviewSnapshot& snapshot);

}  // namespace layoutpreview
