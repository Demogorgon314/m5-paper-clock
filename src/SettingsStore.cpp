#include "SettingsStore.h"

namespace {
constexpr uint8_t kMaxSavedMarketLayoutItems = 6;
constexpr uint8_t kMaxSavedDashboardLayouts = 5;
constexpr const char* kActiveLayoutIdKey = "active_lid";
constexpr uint8_t kPreferencesKeyMaxLength = 15;

bool preferencesKeySafe(const String& key) {
    return key.length() <= kPreferencesKeyMaxLength;
}

String dashboardComponentStoragePrefix(
    const logic::DashboardLayoutItem& item) {
    return "d" + String(logic::DashboardComponentIndex(item.id));
}

MarketLayoutItem defaultMarketLayoutItem(uint8_t index,
                                         const String& fallback_symbol,
                                         const logic::DashboardLayoutItem& summary) {
    MarketLayoutItem item;
    item.instance_id = "market-" + String(index + 1);
    item.symbol = fallback_symbol.isEmpty() ? String("sh000001") : fallback_symbol;
    item.x = index == 0 ? summary.x : static_cast<int16_t>(summary.x + 348);
    item.y = summary.y;
    item.visible = summary.visible;
    return item;
}

std::vector<MarketLayoutItem> defaultDashboardMarketLayout(
    const String& fallback_symbol) {
    const logic::DashboardLayout defaults = logic::DefaultDashboardLayout();
    return {defaultMarketLayoutItem(
        0, fallback_symbol,
        defaults[logic::DashboardComponentIndex(
            logic::DashboardComponentId::Summary)])};
}

SavedDashboardLayout defaultSavedDashboardLayout(
    const String& fallback_symbol) {
    SavedDashboardLayout preset;
    preset.id = logic::kDefaultLayoutId;
    preset.name = logic::kDefaultLayoutName;
    preset.dashboard_layout = logic::DefaultDashboardLayout();
    preset.market_layout = defaultDashboardMarketLayout(fallback_symbol);
    return preset;
}

void normalizeBuiltInDefaultPreset(SavedDashboardLayout& preset,
                                   const String& fallback_symbol) {
    if (preset.id != logic::kDefaultLayoutId) {
        return;
    }
    preset.name = logic::kDefaultLayoutName;
    preset.dashboard_layout = logic::DefaultDashboardLayout();
    preset.market_layout = defaultDashboardMarketLayout(fallback_symbol);
}

bool dashboardPresetIdExists(const std::vector<SavedDashboardLayout>& presets,
                             const String& id) {
    for (const SavedDashboardLayout& preset : presets) {
        if (preset.id == id) {
            return true;
        }
    }
    return false;
}

const SavedDashboardLayout* findDashboardPreset(
    const std::vector<SavedDashboardLayout>& presets, const String& id) {
    for (const SavedDashboardLayout& preset : presets) {
        if (preset.id == id) {
            return &preset;
        }
    }
    return nullptr;
}
}  // namespace

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
        preferences_.getString(kActiveLayoutIdKey, settings.active_layout_id);
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
    settings.date_text_size =
        preferences_.getUChar("date_txt_sz", settings.date_text_size);
    settings.show_holiday =
        preferences_.getBool("show_holiday", settings.show_holiday);
    settings.ble_pairing_token = preferences_.getString("ble_token", "");
    settings.partial_clean_interval = static_cast<uint8_t>(
        logic::ClampPartialCleanInterval(preferences_.getUChar(
            "partial_gc16", settings.partial_clean_interval)));
    settings.mqtt.enabled = preferences_.getBool("mqtt_en", false);
    settings.mqtt.host = preferences_.getString("mqtt_host", settings.mqtt.host);
    settings.mqtt.port = preferences_.getUShort("mqtt_port", settings.mqtt.port);
    settings.mqtt.username = preferences_.getString("mqtt_user", "");
    settings.mqtt.password = preferences_.getString("mqtt_pass", "");
    settings.mqtt.discovery_prefix =
        preferences_.getString("mqtt_pref", settings.mqtt.discovery_prefix);
    settings.mqtt.base_topic = preferences_.getString("mqtt_base", "");
    settings.dashboard_layout = logic::DefaultDashboardLayout();
    for (logic::DashboardLayoutItem& item : settings.dashboard_layout) {
        const String key_prefix = dashboardComponentStoragePrefix(item);
        const String instance_key_prefix = String("dash_") + item.instance_id;
        const String legacy_key_prefix = String("dash_") + item.type;
        auto getShort = [this](const String& key,
                               int16_t fallback) -> int16_t {
            if (!preferencesKeySafe(key)) {
                return fallback;
            }
            return preferences_.getShort(key.c_str(), fallback);
        };
        auto getBool = [this](const String& key, bool fallback) -> bool {
            if (!preferencesKeySafe(key)) {
                return fallback;
            }
            return preferences_.getBool(key.c_str(), fallback);
        };
        item.x = getShort(
            key_prefix + "_x",
            getShort(instance_key_prefix + "_x",
                     getShort(legacy_key_prefix + "_x", item.x)));
        item.y = getShort(
            key_prefix + "_y",
            getShort(instance_key_prefix + "_y",
                     getShort(legacy_key_prefix + "_y", item.y)));
        item.visible = getBool(
            key_prefix + "_v",
            getBool(instance_key_prefix + "_v",
                    getBool(legacy_key_prefix + "_v", item.visible)));
    }
    settings.dashboard_layout =
        logic::NormalizeDashboardLayout(settings.dashboard_layout);
    settings.market_layout.clear();
    const logic::DashboardLayoutItem& summary =
        settings.dashboard_layout[logic::DashboardComponentIndex(
            logic::DashboardComponentId::Summary)];
    const uint8_t market_count =
        preferences_.getUChar("market_count", 1);
    const uint8_t clamped_market_count =
        market_count > kMaxSavedMarketLayoutItems ? kMaxSavedMarketLayoutItems
                                                  : market_count;
    for (uint8_t i = 0; i < clamped_market_count; ++i) {
        const String prefix = "market_" + String(i);
        MarketLayoutItem item = defaultMarketLayoutItem(
            i, i == 0 ? settings.market_symbol : String("sh000001"), summary);
        item.instance_id =
            preferences_.getString((prefix + "_id").c_str(), item.instance_id);
        item.symbol =
            preferences_.getString((prefix + "_sym").c_str(), item.symbol);
        item.x = preferences_.getShort((prefix + "_x").c_str(), item.x);
        item.y = preferences_.getShort((prefix + "_y").c_str(), item.y);
        item.visible =
            preferences_.getBool((prefix + "_v").c_str(), item.visible);
        if (item.symbol.isEmpty()) {
            item.symbol = "sh000001";
        }
        settings.market_layout.push_back(item);
    }
    if (settings.market_layout.empty() && summary.visible) {
        settings.market_layout.push_back(
            defaultMarketLayoutItem(0, settings.market_symbol, summary));
    }
    const SavedDashboardLayout legacy_default =
        defaultSavedDashboardLayout(settings.market_symbol);
    settings.layout_presets.clear();
    const uint8_t layout_count =
        preferences_.getUChar("lp_count", 0);
    const uint8_t clamped_layout_count =
        layout_count > kMaxSavedDashboardLayouts ? kMaxSavedDashboardLayouts
                                                 : layout_count;
    for (uint8_t layout_index = 0; layout_index < clamped_layout_count;
         ++layout_index) {
        const String prefix = "lp" + String(layout_index);
        SavedDashboardLayout preset;
        preset.id =
            preferences_.getString((prefix + "_id").c_str(), "");
        if (preset.id.isEmpty() || preset.id == logic::kClassicLayoutId ||
            dashboardPresetIdExists(settings.layout_presets, preset.id)) {
            continue;
        }
        preset.name =
            preferences_.getString((prefix + "_name").c_str(), preset.id);
        preset.dashboard_layout = logic::DefaultDashboardLayout();
        for (uint8_t component_index = 0;
             component_index < logic::kDashboardComponentCount;
             ++component_index) {
            logic::DashboardLayoutItem& item =
                preset.dashboard_layout[component_index];
            const String component_prefix =
                prefix + "_c" + String(component_index);
            item.x = preferences_.getShort((component_prefix + "x").c_str(),
                                           item.x);
            item.y = preferences_.getShort((component_prefix + "y").c_str(),
                                           item.y);
            item.visible = preferences_.getBool(
                (component_prefix + "v").c_str(), item.visible);
        }
        preset.dashboard_layout =
            logic::NormalizeDashboardLayout(preset.dashboard_layout);
        preset.market_layout.clear();
        const uint8_t preset_market_count = preferences_.getUChar(
            (prefix + "_mc").c_str(), 0);
        const uint8_t clamped_preset_market_count =
            preset_market_count > kMaxSavedMarketLayoutItems
                ? kMaxSavedMarketLayoutItems
                : preset_market_count;
        const logic::DashboardLayoutItem& preset_summary =
            preset.dashboard_layout[logic::DashboardComponentIndex(
                logic::DashboardComponentId::Summary)];
        for (uint8_t market_index = 0;
             market_index < clamped_preset_market_count;
             ++market_index) {
            const String market_prefix =
                prefix + "_m" + String(market_index);
            MarketLayoutItem item = defaultMarketLayoutItem(
                market_index,
                market_index == 0 ? settings.market_symbol : String("sh000001"),
                preset_summary);
            item.instance_id = preferences_.getString(
                (market_prefix + "id").c_str(), item.instance_id);
            item.symbol = preferences_.getString(
                (market_prefix + "s").c_str(), item.symbol);
            item.x = preferences_.getShort((market_prefix + "x").c_str(),
                                           item.x);
            item.y = preferences_.getShort((market_prefix + "y").c_str(),
                                           item.y);
            item.visible = preferences_.getBool(
                (market_prefix + "v").c_str(), item.visible);
            if (item.symbol.isEmpty()) {
                item.symbol = "sh000001";
            }
            preset.market_layout.push_back(item);
        }
        if (preset.market_layout.empty() && preset_summary.visible) {
            preset.market_layout.push_back(defaultMarketLayoutItem(
                0, settings.market_symbol, preset_summary));
        }
        normalizeBuiltInDefaultPreset(preset, settings.market_symbol);
        settings.layout_presets.push_back(preset);
    }
    if (settings.layout_presets.empty()) {
        settings.layout_presets.push_back(legacy_default);
    } else if (!dashboardPresetIdExists(settings.layout_presets,
                                        logic::kDefaultLayoutId)) {
        settings.layout_presets.insert(settings.layout_presets.begin(),
                                       legacy_default);
        if (settings.layout_presets.size() > kMaxSavedDashboardLayouts) {
            settings.layout_presets.resize(kMaxSavedDashboardLayouts);
        }
    }
    if (settings.active_layout_id != logic::kClassicLayoutId) {
        const SavedDashboardLayout* active_preset =
            findDashboardPreset(settings.layout_presets,
                                settings.active_layout_id);
        if (!active_preset) {
            active_preset = &settings.layout_presets.front();
            settings.active_layout_id = active_preset->id;
        }
        settings.dashboard_layout = active_preset->dashboard_layout;
        settings.market_layout = active_preset->market_layout;
    }
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
    preferences_.putString(kActiveLayoutIdKey, settings.active_layout_id);
    preferences_.putString("market_symbol", settings.market_symbol);
    preferences_.putString("market_name", settings.market_name);
    preferences_.putString("date_format", settings.date_format);
    preferences_.putString("weekday_fmt", settings.weekday_format);
    preferences_.putString("date_layout", settings.date_layout);
    preferences_.putUChar("date_txt_sz", settings.date_text_size);
    preferences_.putBool("show_holiday", settings.show_holiday);
    preferences_.putString("ble_token", settings.ble_pairing_token);
    preferences_.putUChar(
        "partial_gc16",
        static_cast<uint8_t>(
            logic::ClampPartialCleanInterval(settings.partial_clean_interval)));
    saveMqtt(settings.mqtt);
    saveDashboardLayout(settings.dashboard_layout);
    saveMarketLayout(settings.market_layout);
    saveLayoutPresets(settings.layout_presets);
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
    preferences_.putString(kActiveLayoutIdKey, active_layout_id);
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
                                    uint8_t date_text_size,
                                    bool show_holiday) {
    if (!started_) {
        return;
    }
    preferences_.putString("date_format", date_format);
    preferences_.putString("weekday_fmt", weekday_format);
    preferences_.putString("date_layout", date_layout);
    preferences_.putUChar("date_txt_sz", date_text_size);
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
        const String key_prefix = dashboardComponentStoragePrefix(item);
        preferences_.putShort((key_prefix + "_x").c_str(), item.x);
        preferences_.putShort((key_prefix + "_y").c_str(), item.y);
        preferences_.putBool((key_prefix + "_v").c_str(), item.visible);
    }
}

void SettingsStore::saveMarketLayout(
    const std::vector<MarketLayoutItem>& market_layout) {
    if (!started_) {
        return;
    }
    const uint8_t count =
        market_layout.size() > kMaxSavedMarketLayoutItems
            ? kMaxSavedMarketLayoutItems
            : static_cast<uint8_t>(market_layout.size());
    preferences_.putUChar("market_count", count);
    for (uint8_t i = 0; i < count; ++i) {
        const String prefix = "market_" + String(i);
        const MarketLayoutItem& item = market_layout[i];
        preferences_.putString((prefix + "_id").c_str(), item.instance_id);
        preferences_.putString((prefix + "_sym").c_str(), item.symbol);
        preferences_.putShort((prefix + "_x").c_str(), item.x);
        preferences_.putShort((prefix + "_y").c_str(), item.y);
        preferences_.putBool((prefix + "_v").c_str(), item.visible);
    }
}

void SettingsStore::saveLayoutPresets(
    const std::vector<SavedDashboardLayout>& presets) {
    if (!started_) {
        return;
    }
    const uint8_t count =
        presets.size() > kMaxSavedDashboardLayouts
            ? kMaxSavedDashboardLayouts
            : static_cast<uint8_t>(presets.size());
    preferences_.putUChar("lp_count", count);
    for (uint8_t layout_index = 0; layout_index < count; ++layout_index) {
        const String prefix = "lp" + String(layout_index);
        const SavedDashboardLayout& preset = presets[layout_index];
        preferences_.putString((prefix + "_id").c_str(), preset.id);
        preferences_.putString((prefix + "_name").c_str(), preset.name);
        const logic::DashboardLayout normalized =
            logic::NormalizeDashboardLayout(preset.dashboard_layout);
        for (uint8_t component_index = 0;
             component_index < logic::kDashboardComponentCount;
             ++component_index) {
            const String component_prefix =
                prefix + "_c" + String(component_index);
            const logic::DashboardLayoutItem& item =
                normalized[component_index];
            preferences_.putShort((component_prefix + "x").c_str(), item.x);
            preferences_.putShort((component_prefix + "y").c_str(), item.y);
            preferences_.putBool((component_prefix + "v").c_str(),
                                 item.visible);
        }
        const uint8_t market_count =
            preset.market_layout.size() > kMaxSavedMarketLayoutItems
                ? kMaxSavedMarketLayoutItems
                : static_cast<uint8_t>(preset.market_layout.size());
        preferences_.putUChar((prefix + "_mc").c_str(), market_count);
        for (uint8_t market_index = 0; market_index < market_count;
             ++market_index) {
            const String market_prefix =
                prefix + "_m" + String(market_index);
            const MarketLayoutItem& item = preset.market_layout[market_index];
            preferences_.putString((market_prefix + "id").c_str(),
                                   item.instance_id);
            preferences_.putString((market_prefix + "s").c_str(), item.symbol);
            preferences_.putShort((market_prefix + "x").c_str(), item.x);
            preferences_.putShort((market_prefix + "y").c_str(), item.y);
            preferences_.putBool((market_prefix + "v").c_str(), item.visible);
        }
    }
}

void SettingsStore::saveMqtt(const MqttSettings& mqtt_settings) {
    if (!started_) {
        return;
    }
    preferences_.putBool("mqtt_en", mqtt_settings.enabled);
    preferences_.putString("mqtt_host", mqtt_settings.host);
    preferences_.putUShort("mqtt_port", mqtt_settings.port);
    preferences_.putString("mqtt_user", mqtt_settings.username);
    preferences_.putString("mqtt_pass", mqtt_settings.password);
    preferences_.putString("mqtt_pref", mqtt_settings.discovery_prefix);
    preferences_.putString("mqtt_base", mqtt_settings.base_topic);
}

void SettingsStore::saveBlePairingToken(const String& token) {
    if (!started_) {
        return;
    }
    preferences_.putString("ble_token", token);
}
