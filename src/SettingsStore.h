#pragma once

#include <Preferences.h>
#include <vector>

#include "AppSettingsTypes.h"

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
    void saveBlePairingToken(const String& token);

private:
    static constexpr const char* kNamespace = "paper-clock";
    mutable Preferences preferences_;
    bool started_ = false;
};
