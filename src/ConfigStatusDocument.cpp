#include "ConfigStatusDocument.h"

#include "LayoutDocumentCodec.h"

namespace configstatus {

void PopulateStatusDocument(JsonObject data,
                            const AppSettings& settings,
                            const ConfigStatusSnapshot& snapshot) {
    data["deviceName"] = "M5Paper Ink Clock";
    data["protocolVersion"] = 1;
    data["firmwareVersion"] = snapshot.firmware_version;
    data["firmwareBuildSha"] = snapshot.firmware_build_sha;
    data["firmwareBuildTime"] = snapshot.firmware_build_time;
    data["page"] = snapshot.page;
    data["activeLayoutId"] = settings.active_layout_id;
    data["layoutKind"] = snapshot.layout_kind;
    data["savedSsid"] = settings.ssid;
    data["wifiConnected"] = snapshot.wifi_connected;
    data["currentSsid"] = snapshot.current_ssid;
    data["ipAddress"] = snapshot.ip_address;
    data["timezone"] = settings.timezone;
    data["timeSynced"] = settings.time_synced;
    data["marketSymbol"] = settings.market_symbol;
    data["marketCode"] = snapshot.market_code;
    data["marketDisplayName"] = snapshot.market_display_name;
    data["dateFormat"] = settings.date_format;
    data["weekdayFormat"] = settings.weekday_format;
    data["dateLayout"] = settings.date_layout;
    data["dateTextSize"] = settings.date_text_size;
    data["showHoliday"] = settings.show_holiday;
    data["rtc"] = snapshot.rtc;
    data["batteryPercent"] = snapshot.battery_percent;
    data["partialCleanInterval"] = settings.partial_clean_interval;
    data["comfortTemperatureMin"] = settings.comfort_settings.min_temperature;
    data["comfortTemperatureMax"] = settings.comfort_settings.max_temperature;
    data["comfortHumidityMin"] = settings.comfort_settings.min_humidity;
    data["comfortHumidityMax"] = settings.comfort_settings.max_humidity;
    data["bluetoothPaired"] = !settings.ble_pairing_token.isEmpty();
    data["bluetoothAuthorized"] = snapshot.bluetooth_authorized;
    data["bluetoothPairingActive"] = snapshot.bluetooth_pairing_active;
    data["statusMessage"] = snapshot.status_message;
    data["statusError"] = snapshot.status_error;
    layoutdoc::PopulateLayoutDocument(data.createNestedObject("layoutDocument"),
                                      settings);
}

}  // namespace configstatus
