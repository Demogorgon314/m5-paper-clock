#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <vector>

#include "logic/ComfortLogic.h"
#include "logic/LayoutLogic.h"
#include "logic/UiLogic.h"

struct MarketLayoutItem {
    String instance_id = "market-1";
    String symbol = "sh000001";
    int16_t x = 72;
    int16_t y = 392;
    bool visible = true;
};

struct SavedDashboardLayout {
    String id = logic::kDefaultLayoutId;
    String name = logic::kDefaultLayoutName;
    logic::DashboardLayout dashboard_layout = logic::DefaultDashboardLayout();
    std::vector<MarketLayoutItem> market_layout = {MarketLayoutItem {}};
};

struct MqttSettings {
    bool enabled = false;
    String host = "homeassistant.local";
    uint16_t port = 1883;
    String username;
    String password;
    String discovery_prefix = "homeassistant";
    String base_topic;
};

struct AppSettings {
    String ssid;
    String password;
    int8_t timezone = 8;
    bool time_synced = false;
    String active_layout_id = logic::kDefaultLayoutId;
    String market_symbol = "sh000001";
    String market_name = "上证指数";
    String date_format = "yyyy-mm-dd";
    String weekday_format = "short";
    String date_layout = "inline";
    uint8_t date_text_size = 7;
    bool show_holiday = true;
    String ble_pairing_token;
    logic::ComfortSettings comfort_settings;
    uint8_t partial_clean_interval = logic::kDefaultPartialCleanInterval;
    logic::DashboardLayout dashboard_layout = logic::DefaultDashboardLayout();
    std::vector<MarketLayoutItem> market_layout = {MarketLayoutItem {}};
    std::vector<SavedDashboardLayout> layout_presets;
    MqttSettings mqtt;
};

class SettingsStore {
public:
    bool begin();
    AppSettings load() const;
    void save(const AppSettings& settings);
    void saveWifi(const String& ssid, const String& password);
    void saveTimezone(int8_t timezone);
    void saveTimeSynced(bool time_synced);
    void saveActiveLayoutId(const String& active_layout_id);
    void saveMarket(const String& market_symbol, const String& market_name);
    void saveDateDisplay(const String& date_format,
                         const String& weekday_format,
                         const String& date_layout,
                         uint8_t date_text_size,
                         bool show_holiday);
    void saveComfortSettings(const logic::ComfortSettings& comfort_settings);
    void savePartialCleanInterval(uint8_t partial_clean_interval);
    void saveDashboardLayout(const logic::DashboardLayout& dashboard_layout);
    void saveMarketLayout(const std::vector<MarketLayoutItem>& market_layout);
    void saveLayoutPresets(const std::vector<SavedDashboardLayout>& presets);
    void saveMqtt(const MqttSettings& mqtt_settings);
    void saveBlePairingToken(const String& token);

private:
    static constexpr const char* kNamespace = "paper-clock";
    mutable Preferences preferences_;
    bool started_ = false;
};
