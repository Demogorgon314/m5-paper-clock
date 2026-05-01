#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include <vector>

#include "ConnectivityService.h"
#include "UiTypes.h"

namespace render {

constexpr int16_t kPageScreenWidth = 960;
constexpr int16_t kPageScreenHeight = 540;
constexpr int16_t kSettingsStatusX = 24;
constexpr int16_t kSettingsStatusY = 86;
constexpr int16_t kSettingsStatusW = 912;
constexpr int16_t kSettingsStatusH = 132;
constexpr int16_t kPasswordFieldX = 28;
constexpr int16_t kPasswordFieldY = 156;
constexpr int16_t kPasswordFieldW = 904;
constexpr int16_t kPasswordFieldH = 58;
constexpr int16_t kPasswordStatusX = 28;
constexpr int16_t kPasswordStatusY = 500;
constexpr int16_t kPasswordStatusW = 904;
constexpr int16_t kPasswordStatusH = 28;

struct SettingsStatusText {
    String network;
    String rtc;
    String sync;
    bool sync_error = false;

    SettingsStatusText() = default;
    SettingsStatusText(const String& network_label, const String& rtc_label,
                       const String& sync_label, bool has_sync_error)
        : network(network_label),
          rtc(rtc_label),
          sync(sync_label),
          sync_error(has_sync_error) {
    }
};

void RenderSettingsPage(M5EPD_Canvas& canvas, const String& ssid,
                        const String& timezone_label,
                        const SettingsStatusText& status,
                        const std::vector<UiButton>& buttons,
                        bool use_cjk_font);
void RenderWifiScanPage(M5EPD_Canvas& canvas,
                        const std::vector<WiFiNetwork>& networks,
                        int page_index, int page_size,
                        const std::vector<UiButton>& buttons,
                        bool use_cjk_font);
void RenderPasswordPage(M5EPD_Canvas& canvas, const String& selected_ssid,
                        const String& masked_password,
                        const String& status_message, bool status_error,
                        const std::vector<UiButton>& buttons,
                        bool use_cjk_font);
void RenderPasswordField(M5EPD_Canvas& canvas, const String& masked_password,
                         bool use_cjk_font);
void RenderPasswordStatus(M5EPD_Canvas& canvas, const String& status_message,
                          bool status_error, bool use_cjk_font);
void RenderSettingsStatusCard(M5EPD_Canvas& canvas, int16_t origin_x,
                              int16_t origin_y,
                              const SettingsStatusText& status,
                              bool use_cjk_font);
void RenderButton(M5EPD_Canvas& canvas, const UiButton& button,
                  bool pressed = false, bool use_cjk_font = true);

}  // namespace render
