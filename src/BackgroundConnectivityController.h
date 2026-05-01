#pragma once

#include <Arduino.h>

#include "ConnectivityService.h"

class BackgroundConnectivityEvents {
public:
    virtual ~BackgroundConnectivityEvents() = default;
    virtual void onBackgroundConnected() = 0;
    virtual void onBackgroundTimeSynced() = 0;
};

class BackgroundConnectivityController {
public:
    enum class Task : uint8_t {
        Idle = 0,
        ReconnectScheduled,
        Reconnecting,
        SyncingTime,
    };

    struct Config {
        uint32_t reconnect_start_delay_ms = 1800;
        uint32_t reconnect_timeout_ms = 4000;
        uint32_t time_sync_timeout_ms = 3000;
    };

    explicit BackgroundConnectivityController(ConnectivityService& connectivity)
        : connectivity_(connectivity) {
    }

    void configure(const Config& config) { config_ = config; }
    void startIfNeeded(bool on_clock_page, const String& ssid,
                       const String& password, int8_t timezone,
                       uint32_t now_ms);
    void poll(bool on_clock_page, bool has_ssid, bool uses_dashboard_layout,
              uint32_t now_ms, BackgroundConnectivityEvents& events);
    void cancel();

    Task task() const { return task_; }

private:
    void beginTimeSync(int8_t timezone);
    void notifyConnected(bool on_clock_page, bool uses_dashboard_layout,
                         BackgroundConnectivityEvents& events);

    ConnectivityService& connectivity_;
    Config config_;
    Task task_ = Task::Idle;
    String ssid_;
    String password_;
    int8_t timezone_ = 8;
    uint32_t due_ms_ = 0;
};
