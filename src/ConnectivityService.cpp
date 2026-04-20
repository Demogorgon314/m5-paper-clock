#include "ConnectivityService.h"

namespace {
constexpr const char* kNtpServer = "time.cloudflare.com";
}

bool ConnectivityService::connect(const String& ssid, const String& password,
                                  uint32_t timeout_ms) {
    if (ssid.isEmpty()) {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout_ms) {
        delay(50);
        M5.update();
    }
    return WiFi.status() == WL_CONNECTED;
}

bool ConnectivityService::ensureConnected(const String& ssid,
                                          const String& password,
                                          uint32_t timeout_ms) {
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        return true;
    }
    return connect(ssid, password, timeout_ms);
}

bool ConnectivityService::syncTimeToRtc(int8_t timezone, uint32_t timeout_ms) {
    configTime(static_cast<long>(timezone) * 3600, 0, kNtpServer);

    struct tm time_info;
    if (!getLocalTime(&time_info, timeout_ms)) {
        return false;
    }

    rtc_time_t rtc_time;
    rtc_time.hour = time_info.tm_hour;
    rtc_time.min = time_info.tm_min;
    rtc_time.sec = time_info.tm_sec;
    M5.RTC.setTime(&rtc_time);

    rtc_date_t rtc_date;
    rtc_date.week = time_info.tm_wday;
    rtc_date.mon = time_info.tm_mon + 1;
    rtc_date.day = time_info.tm_mday;
    rtc_date.year = time_info.tm_year + 1900;
    M5.RTC.setDate(&rtc_date);
    return true;
}

bool ConnectivityService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String ConnectivityService::currentSsid() const {
    return isConnected() ? WiFi.SSID() : String("");
}
