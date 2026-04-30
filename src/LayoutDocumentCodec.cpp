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
