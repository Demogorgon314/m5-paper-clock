#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <array>
#include <vector>

#include "AppSettingsTypes.h"
#include "logic/ComfortLogic.h"
#include "logic/LayoutLogic.h"

namespace layoutdoc {

struct LayoutSharedSettings {
    String market_symbol;
    String date_format;
    String weekday_format;
    String date_layout;
    uint8_t date_text_size = 7;
    bool show_holiday = true;
    logic::ComfortSettings comfort_settings;
};

struct LayoutApplyDraft {
    logic::DashboardLayout dashboard_layout;
    std::vector<MarketLayoutItem> market_layout;
    String market_symbol;
    String date_format;
    String weekday_format;
    String date_layout;
    uint8_t date_text_size = 7;
    bool show_holiday = true;
    bool date_settings_changed = false;
    logic::ComfortSettings comfort_settings;
    bool comfort_settings_changed = false;
    std::array<bool, logic::kDashboardComponentCount> seen {};
};

struct LayoutDocumentPlan {
    String active_layout_id = logic::kDefaultLayoutId;
    bool active_is_classic = false;
    bool has_active_dashboard_draft = false;
    LayoutApplyDraft active_dashboard_draft;
    std::vector<SavedDashboardLayout> presets;
};

LayoutSharedSettings SharedSettingsFromAppSettings(const AppSettings& settings);
LayoutApplyDraft CreateLayoutApplyDraft(const AppSettings& settings);

void PopulateComponentProps(JsonObject props,
                            const LayoutSharedSettings& settings,
                            logic::DashboardComponentId id,
                            const char* variant,
                            const String& symbol = String());
void PopulateLayoutDocument(JsonObject document, const AppSettings& settings);
void PopulateClassicLayoutComponents(JsonArray components,
                                     const AppSettings& settings);
void PopulateDashboardLayoutComponents(
    JsonArray components,
    const logic::DashboardLayout& dashboard_layout,
    const std::vector<MarketLayoutItem>& market_layout,
    const AppSettings& settings);

bool DecodeDashboardComponent(JsonObjectConst component,
                              const LayoutSharedSettings& current_settings,
                              LayoutApplyDraft& draft,
                              bool apply_shared_settings,
                              String& error_message);
bool BuildLayoutApplyDraft(JsonArrayConst components,
                           const AppSettings& settings,
                           bool apply_shared_settings,
                           LayoutApplyDraft& draft,
                           String& error_message);
bool BuildLayoutDocumentPlan(JsonObjectConst document,
                             const AppSettings& settings,
                             LayoutDocumentPlan& plan,
                             String& error_message);

void FinalizeLayoutApplyDraft(LayoutApplyDraft& draft);

}  // namespace layoutdoc
