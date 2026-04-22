#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct AppSettings {
    String ssid;
    String password;
    int8_t timezone = 8;
    bool time_synced = false;
    uint8_t clock_style = 0;
    String market_symbol = "sh000001";
    String market_name = "上证指数";
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

private:
    static constexpr const char* kNamespace = "paper-clock";
    mutable Preferences preferences_;
    bool started_ = false;
};
