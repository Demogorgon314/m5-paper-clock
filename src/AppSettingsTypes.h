#pragma once

#include <Arduino.h>
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
};
