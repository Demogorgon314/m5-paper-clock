#include "LayoutPreviewDocument.h"

#include "DisplayFormatting.h"
#include "LayoutDocumentCodec.h"
#include "logic/MarketLogic.h"

namespace layoutpreview {

namespace {

String marketDisplayLabel(const MarketQuote& quote) {
    return displayfmt::MarketDisplayLabel(quote, true);
}

String marketStatusLabel(const MarketQuote& quote,
                         bool market_open,
                         bool wifi_connected) {
    if (quote.valid) {
        return displayfmt::MarketStatusLabel(quote, market_open, true);
    }
    if (!quote.error_message.isEmpty()) {
        return quote.error_message;
    }
    return wifi_connected ? String(u8"数据不可用") : String(u8"Wi-Fi 离线");
}

void populateMarket(JsonObject data,
                    const String& request_symbol,
                    const String& code,
                    const MarketQuote& quote,
                    bool market_open,
                    bool wifi_connected) {
    data["requestSymbol"] = request_symbol;
    data["code"] = code;
    data["displayName"] = marketDisplayLabel(quote);
    data["valid"] = quote.valid;
    data["price"] = quote.valid ? quote.price : "--";
    data["change"] = quote.valid ? quote.change : "";
    data["changePercent"] = quote.valid ? quote.change_percent : "";
    data["positive"] = quote.positive;
    data["statusLabel"] =
        marketStatusLabel(quote, market_open, wifi_connected);
}

const char* configConnectionIconName(ConfigConnectionIcon icon) {
    switch (icon) {
        case ConfigConnectionIcon::Serial:
            return "serial";
        case ConfigConnectionIcon::Bluetooth:
            return "bluetooth";
        case ConfigConnectionIcon::None:
        default:
            return "none";
    }
}

}  // namespace

void PopulatePreviewDocument(JsonObject data,
                             const AppSettings& settings,
                             const PreviewSnapshot& snapshot) {
    data["protocolVersion"] = 1;
    data["generatedAtMs"] = snapshot.generated_at_ms;
    data["activeLayoutId"] = settings.active_layout_id;
    data["layoutKind"] = snapshot.layout_kind;
    layoutdoc::PopulateLayoutDocument(data.createNestedObject("layoutDocument"),
                                      settings);

    JsonObject render_data = data.createNestedObject("renderData");

    JsonObject time = render_data.createNestedObject("time");
    time["hour"] = snapshot.time.hour;
    time["minute"] = snapshot.time.min;
    time["text"] = displayfmt::TimeDigits(snapshot.time);
    time["display"] = displayfmt::TwoDigits(snapshot.time.hour) + ":" +
                      displayfmt::TwoDigits(snapshot.time.min);

    JsonObject date = render_data.createNestedObject("date");
    date["year"] = snapshot.date.year;
    date["month"] = snapshot.date.mon;
    date["day"] = snapshot.date.day;
    date["iso"] = displayfmt::DateOnly(snapshot.date);
    const uint8_t weekday = displayfmt::CalculateWeekday(snapshot.date);
    date["weekdayIndex"] = weekday;
    date["dateFormat"] = settings.date_format;
    date["weekdayFormat"] = settings.weekday_format;
    date["layoutStyle"] = settings.date_layout;
    date["textSize"] = settings.date_text_size;
    date["showHoliday"] = settings.show_holiday;
    date["displayDate"] =
        displayfmt::ConfiguredDate(snapshot.date, settings.date_format);
    date["weekdayLabel"] = displayfmt::WeekdayLabel(weekday);
    date["displayWeekday"] =
        displayfmt::WeekdayLabel(weekday, settings.weekday_format);
    date["monthLabel"] = displayfmt::TwoDigits(snapshot.date.mon);
    date["daysInMonth"] =
        displayfmt::DaysInMonth(snapshot.date.year, snapshot.date.mon);
    rtc_date_t first_day = snapshot.date;
    first_day.day = 1;
    date["firstWeekday"] = displayfmt::CalculateWeekday(first_day);
    date["holidayText"] =
        displayfmt::HolidayText(snapshot.date, settings.show_holiday);

    JsonObject status = render_data.createNestedObject("status");
    status["batteryPercent"] = snapshot.battery_percent;
    status["wifiConnected"] = snapshot.wifi_connected;
    status["wifiSignalLevel"] =
        snapshot.wifi_connected
            ? displayfmt::WifiBitmapLevel(
                  displayfmt::WifiSignalLevelFromRssi(snapshot.wifi_rssi))
            : 1;
    status["configConnectionIcon"] =
        configConnectionIconName(snapshot.config_connection_icon);

    const bool market_open = logic::IsCnAShareMarketOpen(
        snapshot.date.year, snapshot.date.mon, snapshot.date.day,
        snapshot.time.hour, snapshot.time.min);
    JsonObject market = render_data.createNestedObject("market");
    populateMarket(
        market, settings.market_symbol,
        displayfmt::MarketCodeFromRequestSymbol(settings.market_symbol),
        snapshot.market_quote, market_open, snapshot.wifi_connected);

    JsonArray markets = render_data.createNestedArray("markets");
    for (const MarketPreviewItem& item : snapshot.markets) {
        JsonObject market_item = markets.createNestedObject();
        market_item["id"] = item.id;
        populateMarket(market_item, item.request_symbol, item.quote.symbol,
                       item.quote, market_open, snapshot.wifi_connected);
    }

    JsonObject climate = render_data.createNestedObject("climate");
    climate["valid"] = snapshot.environment.valid;
    climate["humidity"] = displayfmt::DashboardHumidity(snapshot.environment);
    climate["temperature"] =
        displayfmt::DashboardTemperature(snapshot.environment);
    climate["rawHumidity"] = snapshot.environment.humidity;
    climate["rawTemperature"] = snapshot.environment.temperature;
    climate["comfortFace"] =
        displayfmt::ComfortFace(snapshot.environment, settings.comfort_settings);
}

}  // namespace layoutpreview
