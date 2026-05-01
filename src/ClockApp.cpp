#include "ClockApp.h"

#include <algorithm>
#include <array>
#include <SPIFFS.h>
#include <esp_heap_caps.h>
#include <esp_system.h>

#include "logic/ComfortLogic.h"
#include "logic/ClimateDisplayLogic.h"
#include "logic/ConfigCommandLogic.h"
#include "logic/ComponentUpdateGroups.h"
#include "logic/DateDisplayLogic.h"
#include "logic/HolidayLogic.h"
#include "logic/MarketLogic.h"
#include "logic/UiLogic.h"
#include "DisplayFormatting.h"
#include "OtaUtils.h"
#include "PageButtonFactory.h"
#include "render/PageRenderer.h"
#include "render/RenderPrimitives.h"

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

#ifndef FIRMWARE_GIT_SHA
#define FIRMWARE_GIT_SHA "unknown"
#endif

#ifndef FIRMWARE_BUILD_TIME
#define FIRMWARE_BUILD_TIME "unknown"
#endif

#ifndef M5_CLOCK_ENABLE_PERF_LOGS
#define M5_CLOCK_ENABLE_PERF_LOGS 0
#endif

namespace {

constexpr bool kPerfLogs = M5_CLOCK_ENABLE_PERF_LOGS != 0;

template <typename... Args>
void logPerf(const char* format, Args... args) {
    if (kPerfLogs) {
        Serial.printf(format, args...);
    }
}

constexpr uint8_t kWhite = 0;
constexpr uint32_t kClockSensorIntervalMs = 15000;
constexpr uint32_t kDashboardMarketIntervalMs = 60000;
constexpr uint32_t kCenterButtonLongPressMs = 1500;
constexpr uint32_t kBackgroundReconnectStartDelayMs = 1800;
constexpr uint32_t kBackgroundReconnectTimeoutMs = 4000;
constexpr uint32_t kBackgroundTimeSyncTimeoutMs = 3000;
constexpr uint32_t kSerialConfigActiveMs = 60000;
constexpr uint32_t kBlePairingCodeTtlMs = 60000;
constexpr uint8_t kBlePairingMaxAttempts = 5;
constexpr size_t kBleNotifyChunkSize = 20;
constexpr const char* kBleDeviceName = "M5Paper Clock";
constexpr const char* kBleServiceUuid =
    "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
constexpr const char* kBleRxCharacteristicUuid =
    "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
constexpr const char* kBleTxCharacteristicUuid =
    "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
constexpr int16_t kDateX = 24;
constexpr int16_t kDateY = 20;
constexpr int16_t kBatteryX = 768;
constexpr int16_t kBatteryY = 20;
using render::pushAndRefreshCanvas;
using render::uiTextPixelSize;
using render::updateAlignedArea;

bool ensureCanvasSize(M5EPD_Canvas& canvas, int16_t width, int16_t height) {
    if (canvas.width() == width && canvas.height() == height) {
        return true;
    }
    canvas.deleteCanvas();
    return canvas.createCanvas(width, height) != nullptr;
}

using displayfmt::CalculateWeekday;
using displayfmt::ClassicHumidityField;
using displayfmt::ClassicTemperatureField;
using displayfmt::ComfortFace;
using displayfmt::ConfiguredDate;
using displayfmt::DashboardHumidity;
using displayfmt::DashboardTemperature;
using displayfmt::DateOnly;
using displayfmt::DateTime;
using displayfmt::HolidayDisplaySignature;
using displayfmt::HolidayDisplayText;
using displayfmt::MarketCodeFromRequestSymbol;
using displayfmt::MarketSearchMatchesQuery;
using displayfmt::NormalizeDateFormat;
using displayfmt::NormalizeDateLayout;
using displayfmt::NormalizeDateTextSize;
using displayfmt::NormalizeWeekdayFormat;
using displayfmt::TimeDigits;
using displayfmt::WeekdayLabel;
using displayfmt::WifiBitmapLevel;
using displayfmt::WifiSignalLevelFromRssi;

}  // namespace

void ClockApp::begin() {
    Serial.println("[boot] begin: initializeHardware");
    initializeHardware();
    BackgroundConnectivityController::Config background_config;
    background_config.reconnect_start_delay_ms =
        kBackgroundReconnectStartDelayMs;
    background_config.reconnect_timeout_ms = kBackgroundReconnectTimeoutMs;
    background_config.time_sync_timeout_ms = kBackgroundTimeSyncTimeoutMs;
    background_connectivity_.configure(background_config);
    Serial.println("[boot] begin: beginBleConfig");
    beginBleConfig();
    Serial.println("[boot] begin: createCanvases");
    createCanvases();
    Serial.println("[boot] begin: initializeTypography");
    initializeTypography();
    Serial.println("[boot] begin: store.begin");
    store_.begin();
    Serial.println("[boot] begin: store.load");
    settings_ = store_.load();
    settings_.timezone = logic::ClampTimeZone(settings_.timezone);
    if (settings_.market_symbol.isEmpty()) {
        settings_.market_symbol = "sh000001";
    }
    if (settings_.market_name.isEmpty()) {
        settings_.market_name = "上证指数";
    }
    settings_.date_format = NormalizeDateFormat(settings_.date_format);
    settings_.weekday_format = NormalizeWeekdayFormat(settings_.weekday_format);
    settings_.date_layout = NormalizeDateLayout(settings_.date_layout);
    settings_.date_text_size = NormalizeDateTextSize(settings_.date_text_size);
    settings_.comfort_settings =
        logic::NormalizeComfortSettings(settings_.comfort_settings);
    if (settings_.active_layout_id.isEmpty()) {
        settings_.active_layout_id = logic::kDefaultLayoutId;
    }
    if (settings_.layout_presets.empty()) {
        SavedDashboardLayout preset;
        preset.id = logic::kDefaultLayoutId;
        preset.name = logic::kDefaultLayoutName;
        preset.dashboard_layout = settings_.dashboard_layout;
        preset.market_layout = settings_.market_layout;
        settings_.layout_presets.push_back(preset);
    }
    seedMarketQuoteFromSettings();
    M5.RTC.getTime(&last_time_);
    M5.RTC.getDate(&last_date_);
    Serial.println("[boot] begin: renderPage");
    renderPage(UPDATE_MODE_GC16, true);
    Serial.println("[boot] begin: ready");
}

void ClockApp::initializeTypography() {
    Serial.println("[font] mounting SPIFFS");
    littlefs_ready_ = SPIFFS.begin(false);
    if (!littlefs_ready_) {
        Serial.println("[font] SPIFFS mount failed; using built-in font");
        cjk_font_ready_ = false;
        date_cjk_font_ready_ = false;
        dashboard_calendar_cjk_font_ready_ = false;
        dashboard_summary_cjk_font_ready_ = false;
        SPIFFS.end();
        return;
    }

    Serial.printf("[font] SPIFFS total=%u used=%u exists=%d\n",
                  SPIFFS.totalBytes(), SPIFFS.usedBytes(),
                  SPIFFS.exists("/cjk.ttf") ? 1 : 0);
    date_cjk_font_ready_ = loadCjkFont(render_.date_cjk_canvas, {2, 3, 7});
    dashboard_calendar_cjk_font_ready_ =
        loadCjkFont(render_.dashboard_calendar_canvas, {2, 3, 7});
    dashboard_summary_cjk_font_ready_ =
        loadCjkFont(render_.dashboard_summary_cjk_canvas, {2, 3, 7});
    cjk_font_ready_ = false;

    Serial.printf("[font] date=%s calendar=%s market=%s\n",
                  date_cjk_font_ready_ ? "ready" : "fallback",
                  dashboard_calendar_cjk_font_ready_ ? "ready" : "fallback",
                  dashboard_summary_cjk_font_ready_ ? "ready" : "fallback");
}

bool ClockApp::loadCjkFont(M5EPD_Canvas& canvas,
                           std::initializer_list<uint8_t> legacy_sizes) {
    if (!littlefs_ready_) {
        Serial.println("[font] load skipped: SPIFFS not ready");
        return false;
    }

    const esp_err_t rc = canvas.loadFont("/cjk.ttf", SPIFFS);
    if (rc != ESP_OK) {
        Serial.printf("[font] loadFont failed rc=%d\n", static_cast<int>(rc));
        return false;
    }

    std::array<bool, 8> created {};
    for (uint8_t legacy_size : legacy_sizes) {
        if (legacy_size >= created.size() || created[legacy_size]) {
            continue;
        }
        created[legacy_size] = true;
        const uint16_t cache_size = legacy_size <= 2 ? 48 : 24;
        const esp_err_t render_rc =
            canvas.createRender(uiTextPixelSize(legacy_size), cache_size);
        if (render_rc != ESP_OK) {
            Serial.printf("[font] createRender failed size=%u rc=%d\n",
                          static_cast<unsigned>(uiTextPixelSize(legacy_size)),
                          static_cast<int>(render_rc));
            return false;
        }
    }
    canvas.useFreetypeFont(false);
    return true;
}

bool ClockApp::ensureCjkCanvas(M5EPD_Canvas& canvas, int16_t width,
                               int16_t height,
                               std::initializer_list<uint8_t> legacy_sizes) {
    if (canvas.width() == width && canvas.height() == height) {
        return true;
    }

    canvas.deleteCanvas();
    if (canvas.createCanvas(width, height) == nullptr) {
        Serial.printf("[font] createCanvas failed %dx%d\n", width, height);
        return false;
    }
    canvas.useFreetypeFont(false);
    return loadCjkFont(canvas, legacy_sizes);
}

void ClockApp::loop() {
    M5.update();
    handleSerialConfig();
    handleBleConfig();
    handleHardwareButtons();
    handleTouch();
    handleBackgroundConnectivity();

    if (pending_serial_reboot_ &&
        millis() >= pending_serial_reboot_at_ms_) {
        Serial.flush();
        delay(50);
        ESP.restart();
    }

    if (current_page_ == PageId::Settings && !local_ota_.active() &&
        activeConfigConnectionIcon() == ConfigConnectionIcon::None) {
        autoConnectIfNeeded();
    }

    if (ble_pairing_prompt_visible_ && !blePairingCodeActive()) {
        clearBlePairingPrompt();
    }

    if (current_page_ == PageId::Clock) {
        if (pending_dashboard_date_cjk_render_) {
            pending_dashboard_date_cjk_render_ = false;
            updateDateCanvas(false);
            return;
        }
        if (pending_dashboard_calendar_cjk_render_) {
            pending_dashboard_calendar_cjk_render_ = false;
            updateDashboardCalendarCanvas(false);
            return;
        }
        if (pending_market_refresh_) {
            pending_market_refresh_ = false;
            if (connectivity_.isConnected()) {
                refreshMarketQuote(true);
                updateDashboardSummaryCanvas(false, false);
            }
        }
        if (battery_status_dirty_) {
            updateBatteryCanvas(false);
        }
        updateClockPage();
    }
}

void ClockApp::initializeHardware() {
    Serial.setRxBufferSize(16384);
    Serial.begin(1500000);
    delay(50);

    pinMode(M5EPD_MAIN_PWR_PIN, OUTPUT);
    pinMode(M5EPD_EXT_PWR_EN_PIN, OUTPUT);
    pinMode(M5EPD_EPD_PWR_EN_PIN, OUTPUT);
    pinMode(M5EPD_KEY_RIGHT_PIN, INPUT);
    pinMode(M5EPD_KEY_PUSH_PIN, INPUT);
    pinMode(M5EPD_KEY_LEFT_PIN, INPUT);

    M5.enableMainPower();
    M5.enableEXTPower();
    M5.enableEPDPower();
    delay(1000);

    M5.EPD.begin(M5EPD_SCK_PIN, M5EPD_MOSI_PIN, M5EPD_MISO_PIN, M5EPD_CS_PIN,
                 M5EPD_BUSY_PIN);
    M5.EPD.Clear(true);
    M5.EPD.SetRotation(M5EPD_Driver::ROTATE_0);
    M5.TP.SetRotation(GT911::ROTATE_0);
    M5.TP.begin(21, 22, 36);
    M5.BatteryADCBegin();
    sensor_.begin();
    M5.RTC.begin();
}

void ClockApp::createCanvases() {
    render_.page_canvas.createCanvas(kScreenWidth, kScreenHeight);
    render_.page_canvas.useFreetypeFont(false);
    render_.time_canvas.createCanvas(render::kClassicTimeCanvasW,
                                     render::kClassicTimeCanvasH);
    render_.time_canvas.useFreetypeFont(false);
    render_.minute_canvas.createCanvas(render::kClassicMinuteCanvasW,
                                       render::kClassicMinuteCanvasH);
    render_.minute_canvas.useFreetypeFont(false);
    render_.info_canvas.createCanvas(render::kClassicInfoCanvasW,
                                     render::kClassicInfoCanvasH);
    render_.info_canvas.useFreetypeFont(false);
    render_.humidity_canvas.createCanvas(render::kClassicHumidityCanvasW,
                                         render::kClassicHumidityCanvasH);
    render_.humidity_canvas.useFreetypeFont(false);
    render_.temperature_canvas.createCanvas(render::kClassicTemperatureCanvasW,
                                            render::kClassicTemperatureCanvasH);
    render_.temperature_canvas.useFreetypeFont(false);
    render_.comfort_canvas.createCanvas(render::kClassicComfortCanvasW,
                                        render::kClassicComfortCanvasH);
    render_.comfort_canvas.useFreetypeFont(false);
    render_.date_canvas.createCanvas(render::kDateStatusW,
                                     render::kDateStatusH);
    render_.date_canvas.useFreetypeFont(false);
    render_.date_cjk_canvas.createCanvas(render::kDateStatusW,
                                         render::kDateStatusH);
    render_.date_cjk_canvas.useFreetypeFont(false);
    render_.battery_canvas.createCanvas(render::kBatteryStatusW,
                                        render::kBatteryStatusH);
    render_.battery_canvas.useFreetypeFont(false);
    render_.dashboard_calendar_canvas.createCanvas(
        render::kDashboardCalendarStatusW, render::kDashboardCalendarStatusH);
    render_.dashboard_calendar_canvas.useFreetypeFont(false);
    render_.dashboard_time_canvas.createCanvas(render::kDashboardTimeStatusW,
                                               render::kDashboardTimeStatusH);
    render_.dashboard_time_canvas.useFreetypeFont(false);
    render_.dashboard_minute_canvas.createCanvas(
        render::kDashboardMinuteStatusW, render::kDashboardMinuteStatusH);
    render_.dashboard_minute_canvas.useFreetypeFont(false);
    render_.dashboard_summary_canvas.createCanvas(
        render::kDashboardMarketStatusW, render::kDashboardMarketStatusH);
    render_.dashboard_summary_canvas.useFreetypeFont(false);
    render_.dashboard_summary_cjk_canvas.createCanvas(
        render::kDashboardMarketStatusW, render::kDashboardMarketStatusH);
    render_.dashboard_summary_cjk_canvas.useFreetypeFont(false);
    render_.dashboard_climate_canvas.createCanvas(
        render::kDashboardClimateStatusW, render::kDashboardClimateStatusH);
    render_.dashboard_climate_canvas.useFreetypeFont(false);
    render_.password_field_canvas.createCanvas(render::kPasswordFieldW,
                                               render::kPasswordFieldH);
    render_.password_field_canvas.useFreetypeFont(false);
    render_.password_status_canvas.createCanvas(render::kPasswordStatusW,
                                                render::kPasswordStatusH);
    render_.password_status_canvas.useFreetypeFont(false);
}

void ClockApp::renderPage(m5epd_update_mode_t mode, bool force) {
    rebuildButtons();
    switch (current_page_) {
        case PageId::Settings:
            renderSettingsPage(mode);
            break;
        case PageId::WifiScan:
            renderWifiScanPage(mode);
            break;
        case PageId::Password:
            renderPasswordPage(mode);
            break;
        case PageId::Clock:
            renderClockPage(force || mode == UPDATE_MODE_GC16);
            break;
    }
}

void ClockApp::renderSettingsPage(m5epd_update_mode_t mode) {
    const render::SettingsStatusText status {
        wifiStatusLabel(), formatRtcTimestamp(), syncStatusLabel(),
        status_error_
    };
    render::RenderSettingsPage(render_.page_canvas, settings_.ssid,
                               timezoneLabel(), status, buttons_,
                               cjk_font_ready_);
    render_.page_canvas.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(mode);
}

void ClockApp::renderWifiScanPage(m5epd_update_mode_t mode) {
    render::RenderWifiScanPage(render_.page_canvas, wifi_networks_,
                               wifi_page_index_, kWifiPageSize, buttons_,
                               cjk_font_ready_);
    render_.page_canvas.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(mode);
}

void ClockApp::renderPasswordPage(m5epd_update_mode_t mode) {
    render::RenderPasswordPage(render_.page_canvas, selected_ssid_,
                               maskedPassword(), status_message_,
                               status_error_, buttons_, cjk_font_ready_);
    render_.page_canvas.pushCanvas(0, 0, UPDATE_MODE_NONE);
    M5.EPD.UpdateFull(mode);
}

void ClockApp::renderClockPage(bool full_refresh) {
    if (usesDashboardLayout()) {
        renderDashboardClockPage(full_refresh);
        return;
    }
    renderClassicClockPage(full_refresh);
}

void ClockApp::renderClassicClockPage(bool full_refresh) {
    const uint32_t render_started_ms = millis();
    logPerf("[perf] renderClassicClockPage start full=%d at=%lu\n",
                  full_refresh ? 1 : 0, render_started_ms);
    render_.page_canvas.fillCanvas(kWhite);
    render_.page_canvas.pushCanvas(0, 0, UPDATE_MODE_NONE);

    resetClockRenderState();

    uint32_t step_started_ms = millis();
    updateTimeCanvas(true);
    logPerf("[perf] classic updateTimeCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateBatteryCanvas(true);
    logPerf("[perf] classic updateBatteryCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateInfoCanvas(true);
    logPerf("[perf] classic updateInfoCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    updateDateCanvas(true);
    logPerf("[perf] classic updateDateCanvas took=%lu\n",
                  millis() - step_started_ms);
    step_started_ms = millis();
    const m5epd_update_mode_t mode =
        full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16;
    M5.EPD.UpdateFull(mode);
    logPerf("[perf] classic UpdateFull mode=%d took=%lu\n",
                  static_cast<int>(mode), millis() - step_started_ms);
    partial_refresh_count_ = 0;
    logPerf("[perf] renderClassicClockPage total=%lu\n",
                  millis() - render_started_ms);
}

void ClockApp::renderDashboardClockPage(bool full_refresh) {
    const uint32_t render_started_ms = millis();
    logPerf("[perf] renderDashboardClockPage start full=%d at=%lu\n",
                  full_refresh ? 1 : 0, render_started_ms);
    render_.page_canvas.fillCanvas(kWhite);
    render_.page_canvas.pushCanvas(0, 0, UPDATE_MODE_NONE);

    resetClockRenderState();
    const bool fast_entry = full_refresh && fast_dashboard_entry_render_;
    updateDashboardLayoutComponents(true);
    const m5epd_update_mode_t mode =
        full_refresh ? UPDATE_MODE_GC16 : UPDATE_MODE_GL16;
    M5.EPD.UpdateFull(mode);
    partial_refresh_count_ = 0;
    if (fast_entry) {
        pending_dashboard_date_cjk_render_ = true;
        pending_dashboard_calendar_cjk_render_ = true;
    }
    fast_dashboard_entry_render_ = false;
    logPerf("[perf] renderDashboardClockPage total=%lu\n",
                  millis() - render_started_ms);
}

void ClockApp::resetClockRenderState() {
    render_cache_.classic_time = render::ClassicTimeRenderState {};
    render_cache_.classic_info = render::ClassicInfoRenderState {};
    render_cache_.dashboard_time = render::DashboardTimeRenderState {};
    render_cache_.dashboard_climate = render::DashboardClimateRenderState {};
    for (MarketRenderState& state : market_render_states_) {
        state.render_state = render::DashboardMarketRenderState {};
    }
    render_cache_.date_status = render::DateStatusRenderState {};
    render_cache_.battery_status = render::BatteryStatusRenderState {};
    render_cache_.dashboard_calendar = render::DashboardCalendarRenderState {};
}

void ClockApp::updateLayoutComponent(logic::DashboardComponentId id,
                                     bool full_refresh, bool allow_fetch) {
    uint32_t step_started_ms = millis();
    const ComponentUpdateHandler handler = componentUpdateHandler(id);
    if (handler) {
        (this->*handler)(full_refresh, allow_fetch);
    }
    const char* label = logic::DashboardComponentPerfLabel(id);
    logPerf("[perf] dashboard update %s took=%lu\n", label,
                  millis() - step_started_ms);
}

void ClockApp::updateLayoutComponents(const logic::DashboardComponentId* ids,
                                      size_t count, bool full_refresh,
                                      bool allow_fetch) {
    for (size_t index = 0; index < count; ++index) {
        updateLayoutComponent(ids[index], full_refresh, allow_fetch);
    }
}

void ClockApp::updateDashboardLayoutComponents(bool full_refresh) {
    updateLayoutComponents(logic::kFullRefreshComponents.ids.data(),
                           logic::kFullRefreshComponents.count,
                           full_refresh,
                           false);
}

void ClockApp::updateDashboardMinuteComponents() {
    updateLayoutComponents(logic::kMinuteComponents.ids.data(),
                           logic::kMinuteComponents.count,
                           false);
}

void ClockApp::updateDashboardDateComponents() {
    updateLayoutComponents(logic::kDateComponents.ids.data(),
                           logic::kDateComponents.count,
                           false);
}

void ClockApp::updateDashboardSensorComponents() {
    if (!dashboardComponentVisible(logic::DashboardComponentId::Climate) &&
        !dashboardComponentVisible(logic::DashboardComponentId::ClassicInfo)) {
        last_sensor_read_ms_ = millis();
        return;
    }
    updateLayoutComponents(logic::kSensorComponents.ids.data(),
                           logic::kSensorComponents.count,
                           false);
}

void ClockApp::updateDashboardMarketComponents() {
    updateLayoutComponents(logic::kMarketComponents.ids.data(),
                           logic::kMarketComponents.count, false);
}

void ClockApp::updateClockPage() {
    rtc_time_t current_time;
    rtc_date_t current_date;
    M5.RTC.getTime(&current_time);
    M5.RTC.getDate(&current_date);

    const bool minute_changed = current_time.min != last_time_.min ||
                                current_time.hour != last_time_.hour;
    const bool date_changed = current_date.day != last_date_.day ||
                              current_date.mon != last_date_.mon ||
                              current_date.year != last_date_.year;
    const bool time_for_sensor =
        millis() - last_sensor_read_ms_ > kClockSensorIntervalMs;
    const bool time_for_market =
        millis() - last_market_fetch_ms_ > kDashboardMarketIntervalMs;
    const bool time_for_gc16 =
        minute_changed && current_time.min == 0 &&
        (current_time.hour != last_time_.hour || date_changed);

    if (time_for_gc16) {
        renderClockPage(true);
        last_time_ = current_time;
        last_date_ = current_date;
        return;
    }

    if (usesDashboardLayout()) {
        if (minute_changed) {
            updateDashboardMinuteComponents();
        }
        if (date_changed) {
            updateDashboardDateComponents();
        }
        if (time_for_market) {
            updateDashboardMarketComponents();
        }
        if (minute_changed || time_for_sensor) {
            updateBatteryCanvas(false);
        }
        if (time_for_sensor) {
            updateDashboardSensorComponents();
        }
    } else {
        if (minute_changed) {
            updateTimeCanvas(false);
            updateDateCanvas(false);
        }
        if (minute_changed || time_for_sensor) {
            updateBatteryCanvas(false);
        }
        if (time_for_sensor) {
            updateInfoCanvas(false);
        }
    }
    last_time_ = current_time;
    last_date_ = current_date;
}

#include "ClockApp.Components.inc"

void ClockApp::updatePasswordFieldCanvas(m5epd_update_mode_t mode) {
    render::RenderPasswordField(render_.password_field_canvas, maskedPassword(),
                                cjk_font_ready_);
    render_.password_field_canvas.pushCanvas(render::kPasswordFieldX,
                                             render::kPasswordFieldY,
                                             UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(render::kPasswordFieldX, render::kPasswordFieldY,
                      render::kPasswordFieldW, render::kPasswordFieldH, mode);
}

void ClockApp::updatePasswordStatusCanvas(m5epd_update_mode_t mode) {
    render::RenderPasswordStatus(render_.password_status_canvas,
                                 status_message_, status_error_,
                                 cjk_font_ready_);
    render_.password_status_canvas.pushCanvas(render::kPasswordStatusX,
                                              render::kPasswordStatusY,
                                              UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(render::kPasswordStatusX, render::kPasswordStatusY,
                      render::kPasswordStatusW, render::kPasswordStatusH, mode);
}

void ClockApp::updateSettingsStatusCanvas(m5epd_update_mode_t mode) {
    M5EPD_Canvas settings_status_canvas(&M5.EPD);
    settings_status_canvas.createCanvas(render::kSettingsStatusW,
                                        render::kSettingsStatusH);
    const bool local_cjk_font_ready = false;
    const render::SettingsStatusText status {
        wifiStatusLabel(), formatRtcTimestamp(), syncStatusLabel(),
        status_error_
    };
    render::RenderSettingsStatusCard(settings_status_canvas, 0, 0, status,
                                     local_cjk_font_ready);
    settings_status_canvas.pushCanvas(render::kSettingsStatusX,
                                      render::kSettingsStatusY,
                                      UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(render::kSettingsStatusX, render::kSettingsStatusY,
                      render::kSettingsStatusW, render::kSettingsStatusH, mode);
}

void ClockApp::updateButtonCanvas(const UiButton& button, m5epd_update_mode_t mode,
                                  bool pressed) {
    M5EPD_Canvas button_canvas(&M5.EPD);
    button_canvas.createCanvas(button.bounds.w, button.bounds.h);
    const bool local_cjk_font_ready = false;
    button_canvas.fillCanvas(kWhite);
    UiButton local_button = button;
    local_button.bounds.x = 0;
    local_button.bounds.y = 0;
    render::RenderButton(button_canvas, local_button, pressed,
                         local_cjk_font_ready);
    button_canvas.pushCanvas(button.bounds.x, button.bounds.y, UPDATE_MODE_NONE);
    M5.EPD.UpdateArea(button.bounds.x, button.bounds.y, button.bounds.w,
                      button.bounds.h, mode);
}

void ClockApp::rebuildButtons() {
    buttons_.clear();
    switch (current_page_) {
        case PageId::Settings:
            rebuildSettingsButtons();
            break;
        case PageId::WifiScan:
            rebuildWifiButtons();
            break;
        case PageId::Password:
            rebuildPasswordButtons();
            break;
        case PageId::Clock:
            rebuildClockButtons();
            break;
    }
}

void ClockApp::rebuildSettingsButtons() {
    pagebuttons::BuildSettingsButtons(buttons_);
}

void ClockApp::rebuildWifiButtons() {
    pagebuttons::BuildWifiButtons(buttons_,
                                  static_cast<int>(wifi_networks_.size()),
                                  wifi_page_index_, kWifiPageSize);
}

void ClockApp::rebuildPasswordButtons() {
    pagebuttons::BuildPasswordButtons(buttons_, activeKeyboardLayout(),
                                      password_visible_, keyboard_symbols_);
}

void ClockApp::rebuildClockButtons() {
}

#include "ClockApp.Config.inc"

#include "ClockApp.Input.inc"

#include "ClockApp.Runtime.inc"

#include "ClockApp.LayoutPresets.inc"

#include "ClockApp.Ota.inc"

void ClockApp::connectSelectedNetwork() {
    cancelBackgroundConnectivity();
    if (selected_ssid_.isEmpty()) {
        status_message_ = "Choose a Wi-Fi network";
        status_error_ = true;
        updatePasswordStatusCanvas(UPDATE_MODE_GL16);
        return;
    }

    status_message_ = "Connecting to " + selected_ssid_ + "...";
    status_error_ = false;
    updatePasswordStatusCanvas(UPDATE_MODE_GL16);

    if (!connectivity_.connect(selected_ssid_, password_input_)) {
        status_message_ = "Connection failed";
        status_error_ = true;
        updatePasswordStatusCanvas(UPDATE_MODE_GL16);
        return;
    }

    settings_.ssid = selected_ssid_;
    settings_.password = password_input_;
    store_.saveWifi(settings_.ssid, settings_.password);
    auto_connect_attempted_ = true;
    status_message_ = "Connected";
    status_error_ = false;
    updatePasswordStatusCanvas(UPDATE_MODE_GL16);

    if (trySyncTime(false)) {
        switchPage(PageId::Clock);
        return;
    }
    switchPage(PageId::Settings);
}

#include "ClockApp.Layout.inc"

#include "ClockApp.MarketState.inc"

#include "ClockApp.Status.inc"
