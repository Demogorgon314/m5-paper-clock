#include "render/DashboardMarketRenderer.h"

#include "DisplayFormatting.h"

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kBorder = 11;
constexpr uint8_t kText = 15;
constexpr uint8_t kMutedText = 8;
constexpr int16_t kDashboardSummaryTitleX = 16;
constexpr int16_t kDashboardSummaryTitleY = 9;
constexpr int16_t kDashboardSummaryPriceRight = kDashboardMarketStatusW - 14;
constexpr int16_t kDashboardSummaryPriceY = 10;
constexpr int16_t kDashboardSummaryBottomY = 58;
constexpr int16_t kDashboardSummaryArrowGap = 12;
constexpr int16_t kDashboardSummaryValueGap = 14;
constexpr int16_t kDashboardSummaryTitleAreaX = 8;
constexpr int16_t kDashboardSummaryTitleAreaY = 4;
constexpr int16_t kDashboardSummaryTitleAreaW = 180;
constexpr int16_t kDashboardSummaryTitleAreaH = 34;
constexpr int16_t kDashboardSummaryPriceAreaX = 124;
constexpr int16_t kDashboardSummaryPriceAreaY = 4;
constexpr int16_t kDashboardSummaryPriceAreaW =
    kDashboardMarketStatusW - kDashboardSummaryPriceAreaX - 8;
constexpr int16_t kDashboardSummaryPriceAreaH = 52;
constexpr int16_t kDashboardSummaryBottomAreaX = 8;
constexpr int16_t kDashboardSummaryBottomAreaY = 52;
constexpr int16_t kDashboardSummaryBottomAreaW = kDashboardMarketStatusW - 16;
constexpr int16_t kDashboardSummaryBottomAreaH = 26;

void drawFauxBoldString(M5EPD_Canvas& canvas, const String& text, int16_t x,
                        int16_t y, int16_t offset_x = 1) {
    canvas.drawString(text, x, y);
    if (offset_x != 0 && !text.isEmpty()) {
        canvas.drawString(text, x + offset_x, y);
    }
}

void includeFullDirtyRect(DirtyRect& dirty) {
    dirty.x = 0;
    dirty.y = 0;
    dirty.w = kDashboardMarketStatusW;
    dirty.h = kDashboardMarketStatusH;
    dirty.valid = true;
}

}  // namespace

String DashboardMarketSignature(const MarketQuote& quote,
                                bool wifi_connected,
                                bool market_open) {
    if (quote.valid) {
        const String quote_time = displayfmt::MarketQuoteTimeLabel(quote);
        return String("valid|") + quote.symbol + "|" + quote.display_name + "|" +
               quote.price + "|" + quote.change + "|" + quote.change_percent +
               "|" + (quote.positive ? "1" : "0") + "|" +
               (market_open ? "1" : "0") + "|" + quote_time;
    }

    const String status_detail =
        !quote.error_message.isEmpty()
            ? quote.error_message
            : (wifi_connected ? "Data unavailable" : "Wi-Fi offline");
    return String("invalid|") + quote.request_symbol + "|" + quote.symbol + "|" +
           quote.display_name + "|" + status_detail;
}

DashboardMarketRenderResult RenderDashboardMarket(
    M5EPD_Canvas& canvas,
    const DashboardMarketRenderInput& input,
    DashboardMarketRenderState& state) {
    DashboardMarketRenderResult result;
    const String summary_signature =
        DashboardMarketSignature(input.quote, input.wifi_connected,
                                 input.market_open);
    if (!input.full_refresh && summary_signature == state.last_signature) {
        return result;
    }

    canvas.fillCanvas(kWhite);
    canvas.drawRoundRect(0, 0, kDashboardMarketStatusW,
                         kDashboardMarketStatusH, 6, kBorder);
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, input.use_cjk_font, 2);
    const String market_title =
        displayfmt::MarketDisplayLabel(input.quote, input.use_cjk_font);
    const bool summary_valid = input.quote.valid;
    String summary_price;
    String summary_bottom;
    String change_value;
    String percent_value;
    String status_label;
    String status_detail;
    if (summary_valid) {
        summary_price = input.quote.price;
        const String change_prefix = input.quote.positive ? "+" : "";
        change_value = change_prefix + input.quote.change;
        percent_value = change_prefix + input.quote.change_percent + "%";
        status_label =
            displayfmt::MarketStatusLabel(input.quote, input.market_open,
                                          input.use_cjk_font);
        summary_bottom = change_value + "|" + percent_value + "|" +
                         status_label + "|" +
                         (input.quote.positive ? "1" : "0");
    } else {
        status_detail =
            !input.quote.error_message.isEmpty()
                ? input.quote.error_message
                : (input.wifi_connected ? "Data unavailable" : "Wi-Fi offline");
        summary_bottom = status_detail;
    }

    if (input.use_cjk_font) {
        drawFauxBoldString(canvas, market_title, kDashboardSummaryTitleX,
                           kDashboardSummaryTitleY);
    } else {
        canvas.drawString(market_title, kDashboardSummaryTitleX,
                          kDashboardSummaryTitleY);
    }

    if (summary_valid) {
        canvas.setTextDatum(TR_DATUM);
        setCanvasTextSize(canvas, false, 4);
        canvas.setTextColor(kText);
        canvas.drawString(summary_price, kDashboardSummaryPriceRight,
                          kDashboardSummaryPriceY);

        setCanvasTextSize(canvas, false, 2);
        canvas.setTextColor(kText);
        canvas.setTextDatum(TR_DATUM);
        const int16_t percent_right = kDashboardSummaryPriceRight;
        const int16_t percent_width = canvas.textWidth(percent_value);
        const int16_t change_right =
            percent_right - percent_width - kDashboardSummaryValueGap;
        const int16_t change_width = canvas.textWidth(change_value);
        const int16_t change_left = change_right - change_width;
        const int16_t arrow_center_x = change_left - kDashboardSummaryArrowGap;
        const int16_t arrow_center_y = kDashboardSummaryBottomY + 5;
        if (input.quote.positive) {
            canvas.fillTriangle(arrow_center_x - 8, arrow_center_y + 6,
                                arrow_center_x, arrow_center_y - 6,
                                arrow_center_x + 8, arrow_center_y + 6,
                                kText);
        } else {
            canvas.fillTriangle(arrow_center_x - 8, arrow_center_y - 6,
                                arrow_center_x, arrow_center_y + 6,
                                arrow_center_x + 8, arrow_center_y - 6,
                                kText);
        }
        canvas.setTextDatum(TL_DATUM);
        canvas.setTextColor(kMutedText);
        setCanvasTextSize(canvas, input.use_cjk_font, 2);
        canvas.drawString(status_label, 16, 58);
        canvas.setTextColor(kText);
        setCanvasTextSize(canvas, false, 2);
        canvas.setTextDatum(TR_DATUM);
        canvas.drawString(change_value, change_right, kDashboardSummaryBottomY);
        canvas.drawString(percent_value, percent_right,
                          kDashboardSummaryBottomY);
    } else {
        canvas.setTextDatum(TL_DATUM);
        setCanvasTextSize(canvas, false, 3);
        canvas.setTextColor(kText);
        canvas.drawString("No quote", 16, 34);
        setCanvasTextSize(canvas, false, 2);
        canvas.setTextColor(kMutedText);
        canvas.drawString(status_detail, 16, 62);
    }

    if (input.full_refresh) {
        state.partial_count = 0;
    } else {
        if (summary_valid != state.last_valid) {
            includeFullDirtyRect(result.dirty);
        } else {
            if (market_title != state.last_title) {
                DirtyRect rect;
                rect.x = kDashboardSummaryTitleAreaX;
                rect.y = kDashboardSummaryTitleAreaY;
                rect.w = kDashboardSummaryTitleAreaW;
                rect.h = kDashboardSummaryTitleAreaH;
                rect.valid = true;
                includeDirtyRect(result.dirty, rect);
            }
            if (summary_price != state.last_price) {
                DirtyRect rect;
                rect.x = kDashboardSummaryPriceAreaX;
                rect.y = kDashboardSummaryPriceAreaY;
                rect.w = kDashboardSummaryPriceAreaW;
                rect.h = kDashboardSummaryPriceAreaH;
                rect.valid = true;
                includeDirtyRect(result.dirty, rect);
            }
            if (summary_bottom != state.last_bottom) {
                DirtyRect rect;
                rect.x = kDashboardSummaryBottomAreaX;
                rect.y = kDashboardSummaryBottomAreaY;
                rect.w = kDashboardSummaryBottomAreaW;
                rect.h = kDashboardSummaryBottomAreaH;
                rect.valid = true;
                includeDirtyRect(result.dirty, rect);
            }
        }
        result.update_mode =
            nextPartialMode(state.partial_count, input.partial_clean_interval);
        result.partial_update = result.dirty.valid;
    }

    state.last_signature =
        input.fast_ascii_only ? String("") : summary_signature;
    state.last_title = input.fast_ascii_only ? String("") : market_title;
    state.last_price = input.fast_ascii_only ? String("") : summary_price;
    state.last_bottom = input.fast_ascii_only ? String("") : summary_bottom;
    state.last_valid = !input.fast_ascii_only && summary_valid;
    result.rendered = true;
    return result;
}

}  // namespace render
