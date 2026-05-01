#include "render/PageRenderer.h"

#include "logic/UiLogic.h"
#include "render/RenderPrimitives.h"

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kBorder = 11;
constexpr uint8_t kText = 15;
constexpr uint8_t kMutedText = 8;
constexpr uint8_t kPrimaryFill = 2;
constexpr uint8_t kPressedFill = 4;
constexpr uint8_t kErrorText = 12;
constexpr uint8_t kAccentText = 13;
constexpr uint16_t kHeaderHeight = 60;

void drawHeader(M5EPD_Canvas& canvas, const String& title,
                bool use_cjk_font) {
    canvas.drawFastHLine(0, kHeaderHeight - 1, kPageScreenWidth, kBorder);
    canvas.setTextDatum(CC_DATUM);
    setCanvasTextSize(canvas, use_cjk_font, 4);
    canvas.setTextColor(kText);
    canvas.drawString(title, kPageScreenWidth / 2, 26);
}

void drawCard(M5EPD_Canvas& canvas, const Rect& rect, uint8_t fill = kWhite,
              uint8_t border = kBorder) {
    canvas.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, fill);
    canvas.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, border);
}

void drawWifiRow(M5EPD_Canvas& canvas, const UiButton& button,
                 const WiFiNetwork& network, bool use_cjk_font) {
    RenderButton(canvas, button, false, use_cjk_font);

    String label = network.ssid;
    if (label.length() > 28) {
        label = label.substring(0, 28) + "...";
    }

    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString(label, button.bounds.x + 18,
                      button.bounds.y + button.bounds.h / 2);

    canvas.setTextDatum(CR_DATUM);
    canvas.setTextColor(kMutedText);
    canvas.drawString(String(network.rssi) + " dBm",
                      button.bounds.x + button.bounds.w - 18,
                      button.bounds.y + button.bounds.h / 2);
}

void drawPasswordField(M5EPD_Canvas& canvas, const Rect& rect,
                       const String& masked_password, bool use_cjk_font) {
    canvas.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 6, kWhite);
    canvas.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 6, kBorder);
    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString(masked_password, rect.x + 16, rect.y + rect.h / 2);
}

}  // namespace

void RenderSettingsPage(M5EPD_Canvas& canvas, const String& ssid,
                        const String& timezone_label,
                        const SettingsStatusText& status,
                        const std::vector<UiButton>& buttons,
                        bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextColor(kText);
    drawHeader(canvas, "Clock Setup", use_cjk_font);
    RenderSettingsStatusCard(canvas, kSettingsStatusX, kSettingsStatusY,
                             status, use_cjk_font);

    const Rect wifi_card {24, 238, 430, 122};
    const Rect tz_card {482, 238, 454, 122};
    drawCard(canvas, wifi_card);
    drawCard(canvas, tz_card);

    canvas.setTextDatum(TL_DATUM);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.setTextColor(kMutedText);
    canvas.drawString("Saved Wi-Fi", wifi_card.x + 18, wifi_card.y + 16);
    canvas.setTextColor(kText);
    canvas.drawString(ssid.isEmpty() ? "Not configured" : ssid,
                      wifi_card.x + 18, wifi_card.y + 52);

    canvas.setTextColor(kMutedText);
    canvas.drawString("Timezone", tz_card.x + 18, tz_card.y + 16);
    canvas.setTextColor(kText);
    canvas.drawString(timezone_label, tz_card.x + 18, tz_card.y + 52);

    for (const UiButton& button : buttons) {
        RenderButton(canvas, button, false, use_cjk_font);
    }

    canvas.setTextDatum(TR_DATUM);
    canvas.setTextColor(kMutedText);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString("M5Paper horizontal ink clock", kPageScreenWidth - 28,
                      kPageScreenHeight - 22);
}

void RenderWifiScanPage(M5EPD_Canvas& canvas,
                        const std::vector<WiFiNetwork>& networks,
                        int page_index, int page_size,
                        const std::vector<UiButton>& buttons,
                        bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextColor(kText);
    drawHeader(canvas, "Choose Wi-Fi", use_cjk_font);

    canvas.setTextDatum(TL_DATUM);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.setTextColor(kMutedText);
    canvas.drawString("Tap a network and then enter its password.", 28, 82);

    const int visible_count =
        logic::VisibleItemCount(static_cast<int>(networks.size()), page_index,
                                page_size);
    for (int index = 0; index < visible_count; ++index) {
        const int network_index = logic::PageStart(page_index, page_size) + index;
        drawWifiRow(canvas, buttons[index], networks[network_index],
                    use_cjk_font);
    }

    for (size_t i = static_cast<size_t>(visible_count); i < buttons.size();
         ++i) {
        RenderButton(canvas, buttons[i], false, use_cjk_font);
    }

    canvas.setTextDatum(TR_DATUM);
    canvas.setTextColor(kMutedText);
    const int page_count =
        logic::PageCount(static_cast<int>(networks.size()), page_size);
    canvas.drawString("Page " + String(page_index + 1) + "/" +
                          String(page_count),
                      kPageScreenWidth - 28, 82);
}

void RenderPasswordPage(M5EPD_Canvas& canvas, const String& selected_ssid,
                        const String& masked_password,
                        const String& status_message, bool status_error,
                        const std::vector<UiButton>& buttons,
                        bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextColor(kText);
    drawHeader(canvas, "Enter Password", use_cjk_font);

    canvas.setTextDatum(TL_DATUM);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.setTextColor(kMutedText);
    canvas.drawString("SSID", 28, 82);
    canvas.setTextColor(kText);
    canvas.drawString(selected_ssid, 28, 112);

    drawPasswordField(canvas,
                      Rect(kPasswordFieldX, kPasswordFieldY, kPasswordFieldW,
                           kPasswordFieldH),
                      masked_password, use_cjk_font);

    for (const UiButton& button : buttons) {
        RenderButton(canvas, button, false, use_cjk_font);
    }

    if (!status_message.isEmpty()) {
        canvas.setTextColor(status_error ? kErrorText : kAccentText);
        canvas.drawString(status_message, kPasswordStatusX,
                          kPasswordStatusY + (kPasswordStatusH / 2));
    }
}

void RenderPasswordField(M5EPD_Canvas& canvas, const String& masked_password,
                         bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.drawRoundRect(0, 0, kPasswordFieldW, kPasswordFieldH, 6, kBorder);
    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString(masked_password, 16, kPasswordFieldH / 2);
}

void RenderPasswordStatus(M5EPD_Canvas& canvas, const String& status_message,
                          bool status_error, bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(CL_DATUM);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.setTextColor(status_error ? kErrorText : kAccentText);
    canvas.drawString(status_message, 0, kPasswordStatusH / 2);
}

void RenderSettingsStatusCard(M5EPD_Canvas& canvas, int16_t origin_x,
                              int16_t origin_y,
                              const SettingsStatusText& status,
                              bool use_cjk_font) {
    canvas.fillRoundRect(origin_x, origin_y, kSettingsStatusW, kSettingsStatusH,
                         6, kWhite);
    canvas.drawRoundRect(origin_x, origin_y, kSettingsStatusW, kSettingsStatusH,
                         6, kBorder);
    canvas.setTextDatum(TL_DATUM);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.setTextColor(kText);
    canvas.drawString("Network", origin_x + 18, origin_y + 18);
    canvas.drawString("RTC", origin_x + 18, origin_y + 52);
    canvas.drawString("Sync", origin_x + 18, origin_y + 86);
    canvas.setTextColor(kAccentText);
    canvas.drawString(status.network, origin_x + 160, origin_y + 18);
    canvas.drawString(status.rtc, origin_x + 160, origin_y + 52);
    canvas.setTextColor(status.sync_error ? kErrorText : kAccentText);
    canvas.drawString(status.sync, origin_x + 160, origin_y + 86);
}

void RenderButton(M5EPD_Canvas& canvas, const UiButton& button, bool pressed,
                  bool use_cjk_font) {
    if (!button.visible) {
        return;
    }

    const uint8_t fill = pressed ? kPressedFill
                         : (button.primary ? kPrimaryFill : kWhite);
    const uint8_t border = button.enabled ? kBorder : 12;
    canvas.fillRoundRect(button.bounds.x, button.bounds.y, button.bounds.w,
                         button.bounds.h, 6, fill);
    canvas.drawRoundRect(button.bounds.x, button.bounds.y, button.bounds.w,
                         button.bounds.h, 6, border);
    canvas.setTextDatum(CC_DATUM);
    canvas.setTextColor(button.enabled ? kText : kMutedText);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString(button.label, button.bounds.x + button.bounds.w / 2,
                      button.bounds.y + button.bounds.h / 2);
}

}  // namespace render
