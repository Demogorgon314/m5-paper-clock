#include "LayoutSettingsApplier.h"

#include "logic/MarketLogic.h"

namespace layoutsettings {

namespace {

String marketDisplayNameForSymbol(const String& request_symbol) {
    const std::string market_code =
        logic::MarketCodeFromRequestSymbol(std::string(request_symbol.c_str()));
    const std::string known_name = logic::KnownMarketDisplayName(market_code);
    return known_name.empty() ? String(market_code.c_str())
                              : String(known_name.c_str());
}

bool dateSettingsDiffer(const AppSettings& settings,
                        const layoutdoc::LayoutApplyDraft& draft) {
    return draft.date_format != settings.date_format ||
           draft.weekday_format != settings.weekday_format ||
           draft.date_layout != settings.date_layout ||
           draft.date_text_size != settings.date_text_size ||
           draft.show_holiday != settings.show_holiday;
}

}  // namespace

ApplyDraftResult ApplyLayoutDraft(AppSettings& settings,
                                  const layoutdoc::LayoutApplyDraft& draft) {
    ApplyDraftResult result;
    settings.dashboard_layout = draft.dashboard_layout;
    settings.market_layout = draft.market_layout;

    if (!draft.market_symbol.isEmpty() &&
        draft.market_symbol != settings.market_symbol) {
        settings.market_symbol = draft.market_symbol;
        settings.market_name = marketDisplayNameForSymbol(settings.market_symbol);
        result.market_changed = true;
    }

    if (draft.comfort_settings_changed) {
        settings.comfort_settings =
            logic::NormalizeComfortSettings(draft.comfort_settings);
        result.comfort_changed = true;
    }

    if (draft.date_settings_changed && dateSettingsDiffer(settings, draft)) {
        settings.date_format = draft.date_format;
        settings.weekday_format = draft.weekday_format;
        settings.date_layout = draft.date_layout;
        settings.date_text_size = draft.date_text_size;
        settings.show_holiday = draft.show_holiday;
        result.date_changed = true;
    }

    return result;
}

}  // namespace layoutsettings
