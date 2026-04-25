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
    settings.active_layout_id =
        preferences_.getString("active_layout_id", settings.active_layout_id);
    settings.market_symbol =
        preferences_.getString("market_symbol", settings.market_symbol);
    settings.market_name =
        preferences_.getString("market_name", settings.market_name);
    settings.date_format =
        preferences_.getString("date_format", settings.date_format);
    settings.weekday_format =
        preferences_.getString("weekday_fmt", settings.weekday_format);
    settings.date_layout =
        preferences_.getString("date_layout", settings.date_layout);
    settings.show_holiday =
        preferences_.getBool("show_holiday", settings.show_holiday);
    settings.ble_pairing_token = preferences_.getString("ble_token", "");
    settings.partial_clean_interval = static_cast<uint8_t>(
        logic::ClampPartialCleanInterval(preferences_.getUChar(
            "partial_gc16", settings.partial_clean_interval)));
    settings.dashboard_layout = logic::DefaultDashboardLayout();
    for (logic::DashboardLayoutItem& item : settings.dashboard_layout) {
        const String key_prefix = String("dash_") + item.instance_id;
        const String legacy_key_prefix = String("dash_") + item.type;
        item.x = preferences_.getShort(
            (key_prefix + "_x").c_str(),
            preferences_.getShort((legacy_key_prefix + "_x").c_str(), item.x));
        item.y = preferences_.getShort(
            (key_prefix + "_y").c_str(),
            preferences_.getShort((legacy_key_prefix + "_y").c_str(), item.y));
        item.visible =
            preferences_.getBool((key_prefix + "_v").c_str(),
                                 preferences_.getBool(
                                     (legacy_key_prefix + "_v").c_str(),
                                     item.visible));
    }
    settings.dashboard_layout =
        logic::NormalizeDashboardLayout(settings.dashboard_layout);
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
    preferences_.putString("active_layout_id", settings.active_layout_id);
    preferences_.putString("market_symbol", settings.market_symbol);
    preferences_.putString("market_name", settings.market_name);
    preferences_.putString("date_format", settings.date_format);
    preferences_.putString("weekday_fmt", settings.weekday_format);
    preferences_.putString("date_layout", settings.date_layout);
    preferences_.putBool("show_holiday", settings.show_holiday);
    preferences_.putString("ble_token", settings.ble_pairing_token);
    preferences_.putUChar(
        "partial_gc16",
        static_cast<uint8_t>(
            logic::ClampPartialCleanInterval(settings.partial_clean_interval)));
    saveDashboardLayout(settings.dashboard_layout);
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

void SettingsStore::saveActiveLayoutId(const String& active_layout_id) {
    if (!started_) {
        return;
    }
    preferences_.putString("active_layout_id", active_layout_id);
}

void SettingsStore::saveMarket(const String& market_symbol,
                               const String& market_name) {
    if (!started_) {
        return;
    }
    preferences_.putString("market_symbol", market_symbol);
    preferences_.putString("market_name", market_name);
}

void SettingsStore::saveDateDisplay(const String& date_format,
                                    const String& weekday_format,
                                    const String& date_layout,
                                    bool show_holiday) {
    if (!started_) {
        return;
    }
    preferences_.putString("date_format", date_format);
    preferences_.putString("weekday_fmt", weekday_format);
    preferences_.putString("date_layout", date_layout);
    preferences_.putBool("show_holiday", show_holiday);
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

void SettingsStore::savePartialCleanInterval(uint8_t partial_clean_interval) {
    if (!started_) {
        return;
    }
    preferences_.putUChar(
        "partial_gc16",
        static_cast<uint8_t>(
            logic::ClampPartialCleanInterval(partial_clean_interval)));
}

void SettingsStore::saveDashboardLayout(
    const logic::DashboardLayout& dashboard_layout) {
    if (!started_) {
        return;
    }
    const logic::DashboardLayout normalized =
        logic::NormalizeDashboardLayout(dashboard_layout);
    for (const logic::DashboardLayoutItem& item : normalized) {
        const String key_prefix = String("dash_") + item.instance_id;
        preferences_.putShort((key_prefix + "_x").c_str(), item.x);
        preferences_.putShort((key_prefix + "_y").c_str(), item.y);
        preferences_.putBool((key_prefix + "_v").c_str(), item.visible);
    }
}

void SettingsStore::saveBlePairingToken(const String& token) {
    if (!started_) {
        return;
    }
    preferences_.putString("ble_token", token);
}
