#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <M5EPD.h>
#include <mbedtls/sha256.h>

#include <array>
#include <initializer_list>
#include <string>
#include <vector>

#include "ConnectivityService.h"
#include "MarketService.h"
#include "SegmentRenderer.h"
#include "SensorService.h"
#include "SettingsStore.h"

class BLECharacteristic;
class BLEServer;
class BleConfigRxCallbacks;
class BleConfigServerCallbacks;

class ClockApp {
public:
    void begin();
    void loop();

private:
    friend class BleConfigRxCallbacks;
    friend class BleConfigServerCallbacks;

    enum class PageId { Settings, WifiScan, Password, Clock };
    enum class ClockStyle : uint8_t { Classic = 0, Dashboard = 1 };
    enum class ConfigTransport : uint8_t { Serial = 0, Bluetooth = 1 };
    enum class ConfigConnectionIcon : uint8_t {
        None = 0,
        Serial = 1,
        Bluetooth = 2,
    };
    enum class BackgroundConnectivityTask : uint8_t {
        Idle = 0,
        ReconnectScheduled,
        Reconnecting,
        SyncingTime,
    };

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
    void initializeTypography();
    bool loadCjkFont(M5EPD_Canvas& canvas,
                     std::initializer_list<uint8_t> legacy_sizes);
    bool ensureCjkCanvas(M5EPD_Canvas& canvas, int16_t width, int16_t height,
                         std::initializer_list<uint8_t> legacy_sizes);
    void renderPage(m5epd_update_mode_t mode, bool force = false);
    void renderSettingsPage(m5epd_update_mode_t mode);
    void renderWifiScanPage(m5epd_update_mode_t mode);
    void renderPasswordPage(m5epd_update_mode_t mode);
    void renderClockPage(bool full_refresh);
    void renderClassicClockPage(bool full_refresh);
    void renderDashboardClockPage(bool full_refresh);
    void updateClockPage();
    void updateTimeCanvas(bool full_refresh);
    void updateInfoCanvas(bool full_refresh);
    void updateDateCanvas(bool full_refresh);
    void updateBatteryCanvas(bool full_refresh);
    void updateDashboardCalendarCanvas(bool full_refresh);
    void updateDashboardTimeCanvas(bool full_refresh);
    void updateDashboardSummaryCanvas(bool full_refresh,
                                      bool allow_fetch = true);
    void updateDashboardClimateCanvas(bool full_refresh);
    bool refreshMarketQuote(bool force);
    void updatePasswordFieldCanvas(m5epd_update_mode_t mode);
    void updatePasswordStatusCanvas(m5epd_update_mode_t mode);
    void updateSettingsStatusCanvas(m5epd_update_mode_t mode);

    void drawHeader(const String& title, bool show_back);
    void drawButton(const Button& button, bool pressed = false);
    void drawCard(const Rect& rect, uint8_t fill = 0, uint8_t border = 11);
    void drawButton(M5EPD_Canvas& canvas, const Button& button, bool pressed = false,
                    bool use_cjk_font = true);
    void drawSettingsStatusCard(M5EPD_Canvas& canvas, int16_t origin_x,
                                int16_t origin_y, bool use_cjk_font = true);
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
    void handleSerialConfig();
    void handleBleConfig();
    void beginBleConfig();
    void enqueueBleConfigChunk(const std::string& chunk);
    void processConfigLine(const String& line, ConfigTransport transport);
    bool authorizeBleRequest(const JsonDocument& request_doc);
    bool bleCommandRequiresAuth(const String& command) const;
    bool blePairingCodeActive() const;
    void beginBlePairing(DynamicJsonDocument& response_doc);
    void verifyBlePairing(const JsonDocument& request_doc,
                          DynamicJsonDocument& response_doc);
    void clearBlePairingPrompt();
    String generateBlePairingCode() const;
    String generateBlePairingToken() const;
    ConfigConnectionIcon activeConfigConnectionIcon() const;
    void handleButtonPress(int button_id);
    int buttonIdAt(int16_t x, int16_t y) const;
    void switchPage(PageId page, bool force_full_refresh = true);
    void refreshCurrentPage();
    void cycleClockStyle(int delta);
    bool usesDashboardClockStyle() const;
    void startBackgroundReconnect();
    void handleBackgroundConnectivity();
    void cancelBackgroundConnectivity();

    void autoConnectIfNeeded();
    void scanWiFi();
    std::vector<WiFiNetwork> scanWiFiNetworks(bool update_status);
    bool trySyncTime(bool allow_connect);
    void logOtaHeap(const char* phase) const;
    void prepareForOtaUpdate();
    void restoreAfterFailedOtaUpdate();
    void prepareBleConfigForOta();
    void releaseTypographyForOta();
    bool performOtaUpdate(const String& url, const String& expected_sha256,
                          ConfigTransport transport, String& error_message);
    bool beginLocalOtaUpdate(size_t size, const String& expected_sha256,
                             bool binary_mode, String& error_message);
    bool writeLocalOtaChunk(size_t offset, const String& base64_data,
                            size_t& written, String& error_message);
    bool writeLocalOtaBytes(uint8_t* data, size_t length,
                            size_t& written, String& error_message);
    bool finishLocalOtaUpdate(String& error_message);
    void abortLocalOtaUpdate();
    void populateLocalOtaStatus(JsonObject data) const;
    void processLocalOtaBinaryStream();
    void sendOtaProgress(ConfigTransport transport, const char* stage,
                         size_t written, size_t size) const;
    void sendLocalOtaProgress(ConfigTransport transport) const;
    void connectSelectedNetwork();
    void populateSerialStatus(JsonObject data) const;
    void sendConfigDoc(const JsonDocument& doc, ConfigTransport transport) const;
    void sendConfigLine(const String& line, ConfigTransport transport) const;
    const char* currentPageName() const;
    const char* clockStyleName() const;
    String currentIpAddress() const;
    String currentMarketCode() const;
    String currentMarketDisplayName() const;
    void seedMarketQuoteFromSettings();

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
    MarketService market_;
    SensorService sensor_;

    bool local_ota_active_ = false;
    bool local_ota_binary_mode_ = false;
    size_t local_ota_expected_size_ = 0;
    size_t local_ota_written_ = 0;
    size_t local_ota_last_reported_ = 0;
    String local_ota_expected_sha256_;
    mbedtls_sha256_context local_ota_sha_context_;
    SegmentRenderer renderer_;

    AppSettings settings_;
    EnvironmentReading last_environment_;
    MarketQuote market_quote_;
    rtc_time_t last_time_ {};
    rtc_date_t last_date_ {};

    M5EPD_Canvas page_canvas_ {&M5.EPD};
    M5EPD_Canvas time_canvas_ {&M5.EPD};
    M5EPD_Canvas minute_canvas_ {&M5.EPD};
    M5EPD_Canvas info_canvas_ {&M5.EPD};
    M5EPD_Canvas humidity_canvas_ {&M5.EPD};
    M5EPD_Canvas temperature_canvas_ {&M5.EPD};
    M5EPD_Canvas comfort_canvas_ {&M5.EPD};
    M5EPD_Canvas date_canvas_ {&M5.EPD};
    M5EPD_Canvas date_cjk_canvas_ {&M5.EPD};
    M5EPD_Canvas battery_canvas_ {&M5.EPD};
    M5EPD_Canvas dashboard_calendar_canvas_ {&M5.EPD};
    M5EPD_Canvas dashboard_time_canvas_ {&M5.EPD};
    M5EPD_Canvas dashboard_summary_canvas_ {&M5.EPD};
    M5EPD_Canvas dashboard_summary_cjk_canvas_ {&M5.EPD};
    M5EPD_Canvas dashboard_climate_canvas_ {&M5.EPD};
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
    String serial_config_rx_buffer_;
    String ble_config_rx_buffer_;
    String ble_config_pending_chunks_;
    portMUX_TYPE ble_config_rx_mux_ = portMUX_INITIALIZER_UNLOCKED;
    BLECharacteristic* ble_config_tx_characteristic_ = nullptr;
    bool ble_config_ready_ = false;
    bool ble_config_client_connected_ = false;
    bool ble_config_session_authorized_ = false;
    BLEServer* ble_config_server_ = nullptr;
    uint16_t ble_config_conn_id_ = 0;
    bool ble_pairing_prompt_visible_ = false;
    String ble_pairing_code_;
    uint8_t ble_pairing_attempts_ = 0;
    uint32_t ble_pairing_expires_at_ms_ = 0;

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
    ClockStyle clock_style_ = ClockStyle::Dashboard;
    BackgroundConnectivityTask background_connectivity_task_ =
        BackgroundConnectivityTask::Idle;
    bool littlefs_ready_ = false;
    bool cjk_font_ready_ = false;
    bool date_cjk_font_ready_ = false;
    bool dashboard_calendar_cjk_font_ready_ = false;
    bool dashboard_summary_cjk_font_ready_ = false;
    bool fast_dashboard_entry_render_ = false;
    bool pending_dashboard_date_cjk_render_ = false;
    bool pending_dashboard_calendar_cjk_render_ = false;

    uint32_t last_sensor_read_ms_ = 0;
    uint32_t last_market_fetch_ms_ = 0;
    uint32_t last_clock_tick_ms_ = 0;
    uint32_t partial_refresh_count_ = 0;
    bool pending_market_refresh_ = false;
    bool pending_serial_reboot_ = false;
    bool battery_status_dirty_ = false;
    uint32_t pending_serial_reboot_at_ms_ = 0;
    uint32_t background_connectivity_due_ms_ = 0;
    uint32_t last_serial_config_at_ms_ = 0;

    String last_time_text_rendered_;
    String last_humidity_text_rendered_;
    String last_temperature_text_rendered_;
    String last_comfort_face_rendered_;
    String last_market_summary_rendered_;
    String last_date_text_rendered_;
    String last_holiday_display_rendered_;
    uint8_t last_weekday_rendered_ = 255;
    uint8_t last_battery_percentage_ = 255;
    bool last_wifi_connected_ = false;
    uint8_t last_wifi_signal_level_ = 255;
    ConfigConnectionIcon last_config_connection_icon_ =
        ConfigConnectionIcon::None;
    std::array<uint8_t, 4> time_digit_partial_counts_ {};
    std::array<uint8_t, 3> humidity_digit_partial_counts_ {};
    std::array<uint8_t, 3> temperature_digit_partial_counts_ {};
    uint8_t battery_partial_count_ = 0;
};
