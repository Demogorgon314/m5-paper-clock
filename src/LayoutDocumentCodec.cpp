#include "LayoutDocumentCodec.h"

#include "logic/DateDisplayLogic.h"

namespace layoutdoc {

LayoutSharedSettings SharedSettingsFromAppSettings(const AppSettings& settings) {
    LayoutSharedSettings shared;
    shared.market_symbol = settings.market_symbol;
    shared.date_format = settings.date_format;
    shared.weekday_format = settings.weekday_format;
    shared.date_layout = settings.date_layout;
    shared.date_text_size = settings.date_text_size;
    shared.show_holiday = settings.show_holiday;
    shared.comfort_settings = settings.comfort_settings;
    return shared;
}

LayoutApplyDraft CreateLayoutApplyDraft(const AppSettings& settings) {
    LayoutApplyDraft draft;
    draft.dashboard_layout = settings.dashboard_layout;
    draft.market_symbol = settings.market_symbol;
    draft.date_format = settings.date_format;
    draft.weekday_format = settings.weekday_format;
    draft.date_layout = settings.date_layout;
    draft.date_text_size = settings.date_text_size;
    draft.show_holiday = settings.show_holiday;
    draft.comfort_settings = settings.comfort_settings;
    return draft;
}

void PopulateComponentProps(JsonObject props,
                            const LayoutSharedSettings& settings,
                            logic::DashboardComponentId id,
                            const char* variant,
                            const String& symbol) {
    props["variant"] = variant;
    if (id == logic::DashboardComponentId::Date) {
        props["dateFormat"] = settings.date_format;
        props["weekdayFormat"] = settings.weekday_format;
        props["showHoliday"] = settings.show_holiday;
        props["layoutStyle"] = settings.date_layout;
        props["textSize"] = settings.date_text_size;
        return;
    }
    if (id == logic::DashboardComponentId::Climate ||
        id == logic::DashboardComponentId::ClassicInfo) {
        props["comfortTemperatureMin"] =
            settings.comfort_settings.min_temperature;
        props["comfortTemperatureMax"] =
            settings.comfort_settings.max_temperature;
        props["comfortHumidityMin"] = settings.comfort_settings.min_humidity;
        props["comfortHumidityMax"] = settings.comfort_settings.max_humidity;
        return;
    }
    if (id == logic::DashboardComponentId::Summary && !symbol.isEmpty()) {
        props["symbol"] = symbol;
    }
}

void PopulateLayoutDocument(JsonObject document, const AppSettings& settings) {
    const String active_layout_id = settings.active_layout_id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : settings.active_layout_id;
    document["activeLayoutId"] = active_layout_id;
    JsonArray layouts = document.createNestedArray("layouts");

    JsonObject classic_layout = layouts.createNestedObject();
    classic_layout["id"] = logic::kClassicLayoutId;
    classic_layout["name"] = logic::kClassicLayoutName;
    classic_layout["kind"] = "classic";
    JsonObject classic_canvas = classic_layout.createNestedObject("canvas");
    classic_canvas["width"] = logic::kLayoutScreenWidth;
    classic_canvas["height"] = logic::kLayoutScreenHeight;
    PopulateClassicLayoutComponents(classic_layout.createNestedArray("components"),
                                    settings);

    for (const SavedDashboardLayout& preset : settings.layout_presets) {
        JsonObject dashboard_layout = layouts.createNestedObject();
        dashboard_layout["id"] = preset.id;
        dashboard_layout["name"] =
            preset.name.isEmpty() ? preset.id : preset.name;
        dashboard_layout["kind"] = "dashboard";
        JsonObject dashboard_canvas =
            dashboard_layout.createNestedObject("canvas");
        dashboard_canvas["width"] = logic::kLayoutScreenWidth;
        dashboard_canvas["height"] = logic::kLayoutScreenHeight;
        PopulateDashboardLayoutComponents(
            dashboard_layout.createNestedArray("components"),
            preset.dashboard_layout, preset.market_layout, settings);
    }
}

void PopulateClassicLayoutComponents(JsonArray components,
                                     const AppSettings& settings) {
    const LayoutSharedSettings shared_settings =
        SharedSettingsFromAppSettings(settings);
    for (const logic::ClassicLayoutComponentDefinition& item :
         logic::kClassicLayoutComponentDefinitions) {
        JsonObject component = components.createNestedObject();
        component["id"] = item.instance_id;
        component["type"] = item.type;
        component["x"] = item.x;
        component["y"] = item.y;
        component["w"] = item.w;
        component["h"] = item.h;
        component["visible"] = true;
        component["lockedSize"] = true;
        PopulateComponentProps(component.createNestedObject("props"),
                               shared_settings, item.source_id, item.variant);
    }
}

void PopulateDashboardLayoutComponents(
    JsonArray components,
    const logic::DashboardLayout& dashboard_layout,
    const std::vector<MarketLayoutItem>& market_layout,
    const AppSettings& settings) {
    const LayoutSharedSettings shared_settings =
        SharedSettingsFromAppSettings(settings);
    for (const logic::DashboardLayoutItem& item : dashboard_layout) {
        if (item.id == logic::DashboardComponentId::Summary) {
            continue;
        }
        JsonObject component = components.createNestedObject();
        component["id"] = item.instance_id;
        component["type"] = item.type;
        component["x"] = item.x;
        component["y"] = item.y;
        component["w"] = item.w;
        component["h"] = item.h;
        component["visible"] = item.visible;
        component["lockedSize"] = true;
        PopulateComponentProps(component.createNestedObject("props"),
                               shared_settings, item.id, item.variant);
    }
    for (const MarketLayoutItem& item : market_layout) {
        const logic::DashboardLayoutItem& market_default =
            logic::MarketComponentDefaultItem();
        JsonObject component = components.createNestedObject();
        component["id"] = item.instance_id;
        component["type"] = market_default.type;
        component["x"] = item.x;
        component["y"] = item.y;
        component["w"] = market_default.w;
        component["h"] = market_default.h;
        component["visible"] = item.visible;
        component["lockedSize"] = true;
        PopulateComponentProps(component.createNestedObject("props"),
                               shared_settings, market_default.id,
                               market_default.variant, item.symbol);
    }
}

bool DecodeDashboardComponent(JsonObjectConst component,
                              const LayoutSharedSettings& current_settings,
                              LayoutApplyDraft& draft,
                              bool apply_shared_settings,
                              String& error_message) {
    const char* id_key = component["id"] | "";
    const char* type_key = component["type"] | "";
    const bool is_market_component =
        String(type_key) == "market" || String(id_key).startsWith("market-");
    if (is_market_component) {
        const logic::DashboardLayoutItem& market_default =
            logic::MarketComponentDefaultItem();
        MarketLayoutItem item;
        item.instance_id = String(id_key).isEmpty()
                               ? "market-" + String(draft.market_layout.size() + 1)
                               : String(id_key);
        JsonObjectConst props = component["props"].as<JsonObjectConst>();
        item.symbol = current_settings.market_symbol;
        if (!props.isNull()) {
            const String prop_symbol = String(props["symbol"] | "");
            if (!prop_symbol.isEmpty()) {
                item.symbol = prop_symbol;
            }
        }
        item.x = component["x"] | 72;
        item.y = component["y"] | 392;
        item.visible = component["visible"] | true;
        if (item.x < 0) {
            item.x = 0;
        } else if (item.x > logic::kLayoutScreenWidth - market_default.w) {
            item.x = logic::kLayoutScreenWidth - market_default.w;
        }
        if (item.y < 0) {
            item.y = 0;
        } else if (item.y > logic::kLayoutScreenHeight - market_default.h) {
            item.y = logic::kLayoutScreenHeight - market_default.h;
        }
        if (item.symbol.isEmpty()) {
            item.symbol = "sh000001";
        }
        draft.market_layout.push_back(item);
        if (draft.market_layout.size() == 1) {
            draft.market_symbol = item.symbol;
        }
        draft.seen[logic::DashboardComponentIndex(
            logic::DashboardComponentId::Summary)] = true;
        return true;
    }

    bool found = false;
    logic::DashboardComponentId id =
        logic::DashboardComponentIdFromKey(id_key, found);
    if (!found && type_key[0] != '\0') {
        id = logic::DashboardComponentIdFromKey(type_key, found);
    }
    if (!found) {
        error_message = "Unknown dashboard component";
        return false;
    }

    logic::DashboardLayoutItem& item =
        draft.dashboard_layout[logic::DashboardComponentIndex(id)];
    draft.seen[logic::DashboardComponentIndex(id)] = true;
    const logic::DashboardLayoutItem& default_item =
        logic::DashboardLayoutDefaultItem(id);
    item.x = component["x"] | item.x;
    item.y = component["y"] | item.y;
    item.w = default_item.w;
    item.h = default_item.h;
    item.visible = component["visible"] | item.visible;
    item = logic::ClampDashboardLayoutItem(item);

    JsonObjectConst props = component["props"].as<JsonObjectConst>();
    if (id == logic::DashboardComponentId::Date && apply_shared_settings &&
        !props.isNull()) {
        if (props.containsKey("dateFormat")) {
            const String value = String(props["dateFormat"] | "");
            draft.date_format = logic::NormalizeDateFormat(value.c_str());
            draft.date_settings_changed = true;
        }
        if (props.containsKey("weekdayFormat")) {
            const String value = String(props["weekdayFormat"] | "");
            draft.weekday_format = logic::NormalizeWeekdayFormat(value.c_str());
            draft.date_settings_changed = true;
        }
        if (props.containsKey("layoutStyle")) {
            const String value = String(props["layoutStyle"] | "");
            draft.date_layout = logic::NormalizeDateLayout(value.c_str());
            draft.date_settings_changed = true;
        }
        if (props.containsKey("textSize")) {
            draft.date_text_size = logic::NormalizeDateTextSize(
                String(props["textSize"] | "").toInt());
            draft.date_settings_changed = true;
        }
        if (props.containsKey("showHoliday")) {
            draft.show_holiday = props["showHoliday"].as<bool>();
            draft.date_settings_changed = true;
        }
    } else if (id == logic::DashboardComponentId::Summary && !props.isNull()) {
        const String prop_symbol = String(props["symbol"] | "");
        if (!prop_symbol.isEmpty()) {
            draft.market_symbol = prop_symbol;
        }
    } else if (apply_shared_settings &&
               (id == logic::DashboardComponentId::Climate ||
                id == logic::DashboardComponentId::ClassicInfo) &&
               !props.isNull()) {
        if (props.containsKey("comfortTemperatureMin")) {
            draft.comfort_settings.min_temperature =
                props["comfortTemperatureMin"].as<float>();
            draft.comfort_settings_changed = true;
        }
        if (props.containsKey("comfortTemperatureMax")) {
            draft.comfort_settings.max_temperature =
                props["comfortTemperatureMax"].as<float>();
            draft.comfort_settings_changed = true;
        }
        if (props.containsKey("comfortHumidityMin")) {
            draft.comfort_settings.min_humidity =
                props["comfortHumidityMin"].as<float>();
            draft.comfort_settings_changed = true;
        }
        if (props.containsKey("comfortHumidityMax")) {
            draft.comfort_settings.max_humidity =
                props["comfortHumidityMax"].as<float>();
            draft.comfort_settings_changed = true;
        }
    }
    return true;
}

bool BuildLayoutApplyDraft(JsonArrayConst components,
                           const AppSettings& settings,
                           bool apply_shared_settings,
                           LayoutApplyDraft& draft,
                           String& error_message) {
    if (components.isNull()) {
        error_message = "Missing dashboard layout components";
        return false;
    }

    draft = CreateLayoutApplyDraft(settings);
    const LayoutSharedSettings current_settings =
        SharedSettingsFromAppSettings(settings);
    for (JsonVariantConst value : components) {
        if (!DecodeDashboardComponent(value.as<JsonObjectConst>(),
                                      current_settings, draft,
                                      apply_shared_settings, error_message)) {
            return false;
        }
    }

    FinalizeLayoutApplyDraft(draft);
    return true;
}

bool BuildLayoutDocumentPlan(JsonObjectConst document,
                             const AppSettings& settings,
                             LayoutDocumentPlan& plan,
                             String& error_message) {
    if (document.isNull()) {
        error_message = "Missing layout document";
        return false;
    }

    const String active_layout_id =
        String(document["activeLayoutId"] | logic::kDefaultLayoutId);
    JsonArrayConst layouts = document["layouts"].as<JsonArrayConst>();
    if (layouts.isNull()) {
        error_message = "Missing layouts";
        return false;
    }

    plan = LayoutDocumentPlan {};
    plan.active_layout_id = active_layout_id.isEmpty()
        ? String(logic::kDefaultLayoutId)
        : active_layout_id;
    plan.active_is_classic = plan.active_layout_id == logic::kClassicLayoutId;

    bool active_dashboard_found = plan.active_is_classic;
    uint8_t saved_dashboard_count = 0;
    for (JsonVariantConst value : layouts) {
        JsonObjectConst layout = value.as<JsonObjectConst>();
        const String layout_id = String(layout["id"] | "");
        const String layout_kind = String(layout["kind"] | "dashboard");
        if (layout_id.isEmpty() || layout_id == logic::kClassicLayoutId ||
            layout_kind == "classic") {
            continue;
        }
        if (saved_dashboard_count >= 5 && layout_id != plan.active_layout_id) {
            continue;
        }
        if (saved_dashboard_count >= 5 && layout_id == plan.active_layout_id &&
            !plan.presets.empty()) {
            plan.presets.pop_back();
            --saved_dashboard_count;
        }

        LayoutApplyDraft draft;
        const bool is_active_layout = layout_id == plan.active_layout_id;
        if (!BuildLayoutApplyDraft(layout["components"].as<JsonArrayConst>(),
                                   settings, is_active_layout, draft,
                                   error_message)) {
            return false;
        }

        SavedDashboardLayout preset;
        preset.id = layout_id;
        preset.name = String(layout["name"] | layout_id);
        preset.dashboard_layout = draft.dashboard_layout;
        preset.market_layout = draft.market_layout;
        plan.presets.push_back(preset);
        ++saved_dashboard_count;

        if (is_active_layout) {
            active_dashboard_found = true;
            plan.has_active_dashboard_draft = true;
            plan.active_dashboard_draft = draft;
        }
    }

    if (!active_dashboard_found) {
        error_message = "Active layout not found";
        return false;
    }
    return true;
}

void FinalizeLayoutApplyDraft(LayoutApplyDraft& draft) {
    for (size_t index = 0; index < draft.dashboard_layout.size(); ++index) {
        if (!draft.seen[index]) {
            draft.dashboard_layout[index].visible = false;
        }
    }
    logic::DashboardLayoutItem& summary_item =
        draft.dashboard_layout[logic::DashboardComponentIndex(
            logic::DashboardComponentId::Summary)];
    bool any_market_visible = false;
    for (const MarketLayoutItem& item : draft.market_layout) {
        any_market_visible = any_market_visible || item.visible;
    }
    summary_item.visible = any_market_visible;
    if (!draft.market_layout.empty()) {
        summary_item.x = draft.market_layout.front().x;
        summary_item.y = draft.market_layout.front().y;
    }
}

}  // namespace layoutdoc
