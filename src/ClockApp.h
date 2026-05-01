#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <M5EPD.h>

#include <array>
#include <initializer_list>
#include <string>
#include <vector>

#include "BackgroundConnectivityController.h"
#include "BleConfigService.h"
#include "ConnectivityService.h"
#include "ConfigLineBuffer.h"
#include "ConfigStatusDocument.h"
#include "LayoutDocumentCodec.h"
#include "LayoutPreviewDocument.h"
#include "LayoutPresetService.h"
#include "LayoutSettingsApplier.h"
#include "LocalOtaSession.h"
#include "MarketService.h"
#include "RemoteOtaUpdate.h"
#include "RenderState.h"
#include "SegmentRenderer.h"
#include "SensorService.h"
#include "SettingsStore.h"
#include "UiTypes.h"

class ClockApp : private BleConfigServiceEvents,
                 private BackgroundConnectivityEvents {
public:
    void begin();
    void loop();

private:
    using PageId = logic::AppPage;
    enum class ConfigTransport : uint8_t { Serial = 0, Bluetooth = 1 };
    struct RemoteOtaCallbackContext {
        ClockApp* app = nullptr;
        ConfigTransport transport = ConfigTransport::Serial;
    };
    static constexpr int16_t kScreenWidth = 960;
    static constexpr int16_t kScreenHeight = 540;
    static constexpr int16_t kWifiPageSize = 6;
    using ComponentUpdateHandler = void (ClockApp::*)(bool, bool);

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
    void resetClockRenderState();
    void updateLayoutComponent(logic::DashboardComponentId id,
                               bool full_refresh, bool allow_fetch = true);
    void updateLayoutComponents(const logic::DashboardComponentId* ids,
                                size_t count, bool full_refresh,
                                bool allow_fetch = true);
    ComponentUpdateHandler componentUpdateHandler(
        logic::DashboardComponentId id) const;
    void updateDateComponent(bool full_refresh, bool allow_fetch);
    void updateBatteryComponent(bool full_refresh, bool allow_fetch);
    void updateCalendarComponent(bool full_refresh, bool allow_fetch);
    void updateTimeComponent(bool full_refresh, bool allow_fetch);
    void updateMarketComponent(bool full_refresh, bool allow_fetch);
    void updateClimateComponent(bool full_refresh, bool allow_fetch);
    void updateClassicTimeComponent(bool full_refresh, bool allow_fetch);
    void updateClassicInfoComponent(bool full_refresh, bool allow_fetch);
    void updateDashboardLayoutComponents(bool full_refresh);
    void updateDashboardMinuteComponents();
    void updateDashboardDateComponents();
    void updateDashboardSensorComponents();
    void updateDashboardMarketComponents();
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
    struct MarketRenderState {
        String instance_id;
        String symbol;
        MarketQuote quote;
        uint32_t last_fetch_ms = 0;
        render::DashboardMarketRenderState render_state;
    };
    MarketRenderState& marketRenderStateFor(const MarketLayoutItem& item);
    void syncMarketRenderStates();
    void seedMarketQuoteFromSettings(MarketRenderState& state,
                                     const String& symbol,
                                     const String& display_name = String());
    bool refreshMarketQuote(MarketRenderState& state, bool force);
    bool refreshMarketQuote(bool force);
    void updateDashboardMarketInstance(const MarketLayoutItem& item,
                                       MarketRenderState& state,
                                       bool full_refresh,
                                       bool allow_fetch);
    void updatePasswordFieldCanvas(m5epd_update_mode_t mode);
    void updatePasswordStatusCanvas(m5epd_update_mode_t mode);
    void updateSettingsStatusCanvas(m5epd_update_mode_t mode);

    void updateButtonCanvas(const UiButton& button, m5epd_update_mode_t mode,
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
    void processConfigLine(const String& line, ConfigTransport transport);
    void onBleConfigConnected(uint16_t conn_id) override;
    void onBleConfigDisconnected() override;
    void onBackgroundConnected() override;
    void onBackgroundTimeSynced() override;
    bool authorizeBleRequest(const JsonDocument& request_doc);
    bool blePairingCodeActive() const;
    void beginBlePairing(DynamicJsonDocument& response_doc);
    void verifyBlePairing(const JsonDocument& request_doc,
                          DynamicJsonDocument& response_doc);
    void clearBlePairingPrompt();
    String generateBlePairingCode() const;
    String generateBlePairingToken() const;
    ConfigConnectionIcon activeConfigConnectionIcon() const;
    void handleRemoteOtaConfigCommand(const JsonDocument& request_doc,
                                      DynamicJsonDocument& response_doc,
                                      ConfigTransport transport);
    void handleLocalOtaBeginConfigCommand(const JsonDocument& request_doc,
                                          DynamicJsonDocument& response_doc,
                                          ConfigTransport transport);
    void handleLocalOtaChunkConfigCommand(const JsonDocument& request_doc,
                                          DynamicJsonDocument& response_doc,
                                          ConfigTransport transport);
    void handleLocalOtaStatusConfigCommand(DynamicJsonDocument& response_doc,
                                           ConfigTransport transport);
    void handleLocalOtaEndConfigCommand(DynamicJsonDocument& response_doc,
                                        ConfigTransport transport);
    void handleLocalOtaAbortConfigCommand(DynamicJsonDocument& response_doc);
    void handleScanWifiConfigCommand(DynamicJsonDocument& response_doc,
                                     ConfigTransport transport);
    void handleSearchMarketConfigCommand(const JsonDocument& request_doc,
                                         DynamicJsonDocument& response_doc,
                                         ConfigTransport transport);
    void handleSetMarketConfigCommand(const JsonDocument& request_doc,
                                      DynamicJsonDocument& response_doc,
                                      ConfigTransport transport);
    void handleApplySettingsConfigCommand(const JsonDocument& request_doc,
                                          DynamicJsonDocument& response_doc,
                                          ConfigTransport transport);
    void handleButtonPress(int button_id);
    int buttonIdAt(int16_t x, int16_t y) const;
    void switchPage(PageId page, bool force_full_refresh = true);
    void refreshCurrentPage();
    void cycleClockLayout(int delta);
    bool usesDashboardLayout() const;
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
    static void handleRemoteOtaProgress(void* context, const char* stage,
                                        size_t written, size_t size);
    static void handleRemoteOtaCheckpoint(void* context, const char* phase);
    static void handleRemoteOtaYield(void* context);
    void connectSelectedNetwork();
    void populateSerialStatus(JsonObject data) const;
    void populateLayoutPreviewState(JsonObject data) const;
    void sendConfigDoc(const JsonDocument& doc, ConfigTransport transport) const;
    void sendConfigLine(const String& line, ConfigTransport transport) const;
    const char* currentPageName() const;
    const char* activeLayoutKind() const;
    const logic::DashboardLayoutItem& dashboardLayoutItem(
        logic::DashboardComponentId id) const;
    bool dashboardComponentVisible(logic::DashboardComponentId id) const;
    int16_t dashboardComponentX(logic::DashboardComponentId id) const;
    int16_t dashboardComponentY(logic::DashboardComponentId id) const;
    int activeDashboardPresetIndex() const;
    void applyDashboardPreset(const SavedDashboardLayout& preset);
    void upsertActiveDashboardPreset(const String& name = String());
    bool applyLayoutDocument(JsonObjectConst document, String& error_message);
    bool applyDashboardLayout(JsonArrayConst components, String& error_message,
                              bool apply_shared_settings = true);
    void commitLayoutApplyDraft(const layoutdoc::LayoutApplyDraft& draft);
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
    BackgroundConnectivityController background_connectivity_ {connectivity_};

    RemoteOtaUpdate remote_ota_;
    LocalOtaSession local_ota_;
    size_t local_ota_last_reported_ = 0;
    SegmentRenderer renderer_;

    AppSettings settings_;
    EnvironmentReading last_environment_;
    MarketQuote market_quote_;
    std::vector<MarketRenderState> market_render_states_;
    rtc_time_t last_time_ {};
    rtc_date_t last_date_ {};
    RenderResources render_;
    mutable RenderCache render_cache_;

    std::vector<UiButton> buttons_;
    std::vector<WiFiNetwork> wifi_networks_;

    PageId current_page_ = PageId::Settings;
    int wifi_page_index_ = 0;
    int pressed_button_id_ = -1;
    ConfigLineBuffer serial_config_rx_buffer_ {8192};
    BleConfigService ble_config_;
    bool ble_config_session_authorized_ = false;
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
    uint32_t last_serial_config_at_ms_ = 0;

};
