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
