#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include <array>
#include <vector>

#include "ConnectivityService.h"
#include "SegmentRenderer.h"
#include "SensorService.h"
#include "SettingsStore.h"

class ClockApp {
public:
    void begin();
    void loop();

private:
    enum class PageId { Settings, WifiScan, Password, Clock };

    enum ButtonId {
        kButtonWifi = 1,
        kButtonSyncTime,
        kButtonEnterClock,
        kButtonTimezoneMinus,
        kButtonTimezonePlus,
        kButtonBack,
        kButtonRescan,
        kButtonPrevPage,
        kButtonNextPage,
        kButtonBackspace,
        kButtonSpace,
        kButtonClear,
        kButtonPasswordVisibility,
        kButtonConnect,
        kButtonKeyboardMode,
        kButtonShift,
        kButtonNetworkBase = 100,
        kButtonKeyboardBase = 200,
    };

    struct Rect {
        int16_t x = 0;
        int16_t y = 0;
        int16_t w = 0;
        int16_t h = 0;

        Rect() = default;
        Rect(int16_t x_value, int16_t y_value, int16_t width_value,
             int16_t height_value)
            : x(x_value), y(y_value), w(width_value), h(height_value) {
        }

        bool contains(int16_t px, int16_t py) const {
            return px >= x && px < x + w && py >= y && py < y + h;
        }
    };

    struct Button {
        int id = 0;
        Rect bounds;
        String label;
        bool visible = true;
        bool enabled = true;
        bool primary = false;

        Button() = default;
        Button(int button_id, Rect button_bounds, const String& button_label,
               bool is_visible, bool is_enabled, bool is_primary)
            : id(button_id),
              bounds(button_bounds),
              label(button_label),
              visible(is_visible),
              enabled(is_enabled),
              primary(is_primary) {
        }
    };

    struct WiFiNetwork {
        String ssid;
        int32_t rssi = -100;
    };

    static constexpr int16_t kScreenWidth = 960;
    static constexpr int16_t kScreenHeight = 540;
    static constexpr int16_t kWifiPageSize = 6;

    void initializeHardware();
    void createCanvases();
    void renderPage(m5epd_update_mode_t mode, bool force = false);
    void renderSettingsPage(m5epd_update_mode_t mode);
    void renderWifiScanPage(m5epd_update_mode_t mode);
    void renderPasswordPage(m5epd_update_mode_t mode);
    void renderClockPage(bool full_refresh);
    void updateClockPage();
    void updateTimeCanvas(bool full_refresh);
    void updateInfoCanvas(bool full_refresh);
    void updateDateCanvas(bool full_refresh);
    void updateBatteryCanvas(bool full_refresh);
    void updatePasswordFieldCanvas(m5epd_update_mode_t mode);
    void updatePasswordStatusCanvas(m5epd_update_mode_t mode);
    void updateSettingsStatusCanvas(m5epd_update_mode_t mode);

    void drawHeader(const String& title, bool show_back);
    void drawButton(const Button& button, bool pressed = false);
    void drawCard(const Rect& rect, uint8_t fill = 0, uint8_t border = 11);
    void drawButton(M5EPD_Canvas& canvas, const Button& button, bool pressed = false);
    void drawSettingsStatusCard(M5EPD_Canvas& canvas, int16_t origin_x,
                                int16_t origin_y);
    void drawStatusLine(const String& label, const String& value, int16_t x,
                        int16_t y);
    void drawWifiRow(const Button& button, const WiFiNetwork& network);
    void drawPasswordField(const Rect& rect);
    void updateButtonCanvas(const Button& button, m5epd_update_mode_t mode,
                            bool pressed = false);

    void rebuildButtons();
    void rebuildSettingsButtons();
    void rebuildWifiButtons();
    void rebuildPasswordButtons();
    void rebuildClockButtons();

    void handleHardwareButtons();
    void handleTouch();
    void handleButtonPress(int button_id);
    int buttonIdAt(int16_t x, int16_t y) const;
    void switchPage(PageId page, bool force_full_refresh = true);
    void refreshCurrentPage();

    void autoConnectIfNeeded();
    void scanWiFi();
    bool trySyncTime(bool allow_connect);
    void connectSelectedNetwork();

    String timezoneLabel() const;
    String wifiStatusLabel() const;
    String syncStatusLabel() const;
    String formatRtcTimestamp() const;
    uint8_t batteryPercentage() const;
    String maskedPassword() const;
    String keyboardCharacterLabel(size_t index) const;
    std::vector<String> activeKeyboardLayout() const;
    void appendPasswordChar(const String& text);

    SettingsStore store_;
    ConnectivityService connectivity_;
    SensorService sensor_;
    SegmentRenderer renderer_;

    AppSettings settings_;
    EnvironmentReading last_environment_;
    rtc_time_t last_time_ {};
    rtc_date_t last_date_ {};

    M5EPD_Canvas page_canvas_ {&M5.EPD};
    M5EPD_Canvas time_canvas_ {&M5.EPD};
    M5EPD_Canvas info_canvas_ {&M5.EPD};
    M5EPD_Canvas date_canvas_ {&M5.EPD};
    M5EPD_Canvas battery_canvas_ {&M5.EPD};
    M5EPD_Canvas password_field_canvas_ {&M5.EPD};
    M5EPD_Canvas password_status_canvas_ {&M5.EPD};
    std::array<M5EPD_Canvas, 4> time_digit_canvases_ {};
    std::array<M5EPD_Canvas, 3> humidity_digit_canvases_ {};
    std::array<M5EPD_Canvas, 3> temperature_digit_canvases_ {};

    std::vector<Button> buttons_;
    std::vector<WiFiNetwork> wifi_networks_;

    PageId current_page_ = PageId::Settings;
    int wifi_page_index_ = 0;
    int pressed_button_id_ = -1;

    String selected_ssid_;
    String password_input_;
    String status_message_ = "Ready";
    bool status_error_ = false;

    bool keyboard_upper_ = false;
    bool keyboard_symbols_ = false;
    bool password_visible_ = false;
    bool auto_connect_attempted_ = false;
    bool first_settings_render_ = true;
    bool touch_down_ = false;
    bool center_button_long_press_handled_ = false;

    uint32_t last_sensor_read_ms_ = 0;
    uint32_t last_clock_tick_ms_ = 0;
    uint32_t partial_refresh_count_ = 0;
    uint32_t enter_clock_at_ms_ = 0;

    String last_time_text_rendered_;
    String last_humidity_text_rendered_;
    String last_temperature_text_rendered_;
    String last_date_text_rendered_;
    uint8_t last_weekday_rendered_ = 255;
    uint8_t last_battery_percentage_ = 255;
};
