#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "logic/ComfortLogic.h"
#include "logic/LayoutLogic.h"
#include "logic/UiLogic.h"

struct AppSettings {
    String ssid;
    String password;
    int8_t timezone = 8;
    bool time_synced = false;
    uint8_t clock_style = 1;
    String market_symbol = "sh000001";
    String market_name = "上证指数";
    String ble_pairing_token;
    logic::ComfortSettings comfort_settings;
    uint8_t partial_clean_interval = logic::kDefaultPartialCleanInterval;
    logic::DashboardLayout dashboard_layout = logic::DefaultDashboardLayout();
};

class SettingsStore {
public:
    bool begin();
    AppSettings load() const;
    void save(const AppSettings& settings);
    void saveWifi(const String& ssid, const String& password);
    void saveTimezone(int8_t timezone);
    void saveTimeSynced(bool time_synced);
    void saveClockStyle(uint8_t clock_style);
    void saveMarket(const String& market_symbol, const String& market_name);
    void saveComfortSettings(const logic::ComfortSettings& comfort_settings);
    void savePartialCleanInterval(uint8_t partial_clean_interval);
    void saveDashboardLayout(const logic::DashboardLayout& dashboard_layout);
    void saveBlePairingToken(const String& token);

private:
    static constexpr const char* kNamespace = "paper-clock";
    mutable Preferences preferences_;
    bool started_ = false;
};
