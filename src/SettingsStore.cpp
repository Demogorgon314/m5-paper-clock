#include "SettingsStore.h"

bool SettingsStore::begin() {
    if (started_) {
        return true;
    }
    started_ = preferences_.begin(kNamespace, false);
    return started_;
}

AppSettings SettingsStore::load() const {
    AppSettings settings;
    if (!started_) {
        return settings;
    }
    settings.ssid = preferences_.getString("ssid", "");
    settings.password = preferences_.getString("password", "");
    settings.timezone = preferences_.getChar("timezone", settings.timezone);
    settings.time_synced = preferences_.getBool("time_synced", false);
    settings.clock_style = preferences_.getUChar("clock_style", settings.clock_style);
    settings.market_symbol =
        preferences_.getString("market_symbol", settings.market_symbol);
    settings.market_name =
        preferences_.getString("market_name", settings.market_name);
    settings.ble_pairing_token = preferences_.getString("ble_token", "");
    settings.comfort_settings.min_temperature =
        preferences_.getFloat("comfort_t_min",
                              settings.comfort_settings.min_temperature);
    settings.comfort_settings.max_temperature =
        preferences_.getFloat("comfort_t_max",
                              settings.comfort_settings.max_temperature);
    settings.comfort_settings.min_humidity =
        preferences_.getFloat("comfort_h_min",
                              settings.comfort_settings.min_humidity);
    settings.comfort_settings.max_humidity =
        preferences_.getFloat("comfort_h_max",
                              settings.comfort_settings.max_humidity);
    if (settings.market_symbol.isEmpty()) {
        settings.market_symbol = "sh000001";
    }
    if (settings.market_name.isEmpty()) {
        settings.market_name = "上证指数";
    }
    settings.comfort_settings =
        logic::NormalizeComfortSettings(settings.comfort_settings);
    return settings;
}

void SettingsStore::save(const AppSettings& settings) {
    if (!started_) {
        return;
    }
    preferences_.putString("ssid", settings.ssid);
    preferences_.putString("password", settings.password);
    preferences_.putChar("timezone", settings.timezone);
    preferences_.putBool("time_synced", settings.time_synced);
    preferences_.putUChar("clock_style", settings.clock_style);
    preferences_.putString("market_symbol", settings.market_symbol);
    preferences_.putString("market_name", settings.market_name);
    preferences_.putString("ble_token", settings.ble_pairing_token);
    preferences_.putFloat("comfort_t_min",
                          settings.comfort_settings.min_temperature);
    preferences_.putFloat("comfort_t_max",
                          settings.comfort_settings.max_temperature);
    preferences_.putFloat("comfort_h_min",
                          settings.comfort_settings.min_humidity);
    preferences_.putFloat("comfort_h_max",
                          settings.comfort_settings.max_humidity);
}

void SettingsStore::saveWifi(const String& ssid, const String& password) {
    if (!started_) {
        return;
    }
    preferences_.putString("ssid", ssid);
    preferences_.putString("password", password);
}

void SettingsStore::saveTimezone(int8_t timezone) {
    if (!started_) {
        return;
    }
    preferences_.putChar("timezone", timezone);
}

void SettingsStore::saveTimeSynced(bool time_synced) {
    if (!started_) {
        return;
    }
    preferences_.putBool("time_synced", time_synced);
}

void SettingsStore::saveClockStyle(uint8_t clock_style) {
    if (!started_) {
        return;
    }
    preferences_.putUChar("clock_style", clock_style);
}

void SettingsStore::saveMarket(const String& market_symbol,
                               const String& market_name) {
    if (!started_) {
        return;
    }
    preferences_.putString("market_symbol", market_symbol);
    preferences_.putString("market_name", market_name);
}

void SettingsStore::saveComfortSettings(
    const logic::ComfortSettings& comfort_settings) {
    if (!started_) {
        return;
    }
    const logic::ComfortSettings normalized =
        logic::NormalizeComfortSettings(comfort_settings);
    preferences_.putFloat("comfort_t_min", normalized.min_temperature);
    preferences_.putFloat("comfort_t_max", normalized.max_temperature);
    preferences_.putFloat("comfort_h_min", normalized.min_humidity);
    preferences_.putFloat("comfort_h_max", normalized.max_humidity);
}

void SettingsStore::saveBlePairingToken(const String& token) {
    if (!started_) {
        return;
    }
    preferences_.putString("ble_token", token);
}
