#include "BackgroundConnectivityController.h"

void BackgroundConnectivityController::startIfNeeded(
    bool on_clock_page,
    const String& ssid,
    const String& password,
    int8_t timezone,
    uint32_t now_ms) {
    if (!on_clock_page || ssid.isEmpty() || connectivity_.isConnected() ||
        task_ != Task::Idle) {
        return;
    }

    ssid_ = ssid;
    password_ = password;
    timezone_ = timezone;
    task_ = Task::ReconnectScheduled;
    due_ms_ = now_ms + config_.reconnect_start_delay_ms;
}

void BackgroundConnectivityController::poll(
    bool on_clock_page,
    bool has_ssid,
    bool uses_dashboard_layout,
    uint32_t now_ms,
    BackgroundConnectivityEvents& events) {
    if (task_ == Task::Idle) {
        return;
    }

    if (task_ == Task::ReconnectScheduled) {
        if (!on_clock_page || !has_ssid || connectivity_.isConnected()) {
            cancel();
            return;
        }
        if (now_ms < due_ms_) {
            return;
        }

        const ConnectivityService::AsyncConnectState state =
            connectivity_.beginConnectAsync(ssid_, password_,
                                            config_.reconnect_timeout_ms);
        if (state == ConnectivityService::AsyncConnectState::InProgress) {
            task_ = Task::Reconnecting;
            return;
        }

        if (state == ConnectivityService::AsyncConnectState::Succeeded) {
            task_ = Task::SyncingTime;
            notifyConnected(on_clock_page, uses_dashboard_layout, events);
            beginTimeSync(timezone_);
            return;
        }

        cancel();
        return;
    }

    if (task_ == Task::Reconnecting) {
        const ConnectivityService::AsyncConnectState state =
            connectivity_.pollConnectAsync();
        if (state == ConnectivityService::AsyncConnectState::InProgress ||
            state == ConnectivityService::AsyncConnectState::Idle) {
            return;
        }

        if (state == ConnectivityService::AsyncConnectState::Failed) {
            task_ = Task::Idle;
            connectivity_.cancelConnectAsync();
            return;
        }

        connectivity_.cancelConnectAsync();
        task_ = Task::SyncingTime;
        notifyConnected(on_clock_page, uses_dashboard_layout, events);
        beginTimeSync(timezone_);
        return;
    }

    const ConnectivityService::AsyncTimeSyncState sync_state =
        connectivity_.pollTimeSyncAsync();
    if (sync_state == ConnectivityService::AsyncTimeSyncState::InProgress ||
        sync_state == ConnectivityService::AsyncTimeSyncState::Idle) {
        return;
    }

    connectivity_.cancelTimeSyncAsync();
    task_ = Task::Idle;

    if (sync_state == ConnectivityService::AsyncTimeSyncState::Succeeded) {
        events.onBackgroundTimeSynced();
    }
}

void BackgroundConnectivityController::cancel() {
    task_ = Task::Idle;
    due_ms_ = 0;
    connectivity_.cancelConnectAsync();
    connectivity_.cancelTimeSyncAsync();
}

void BackgroundConnectivityController::beginTimeSync(int8_t timezone) {
    connectivity_.beginTimeSyncAsync(timezone, config_.time_sync_timeout_ms);
}

void BackgroundConnectivityController::notifyConnected(
    bool on_clock_page,
    bool uses_dashboard_layout,
    BackgroundConnectivityEvents& events) {
    (void)uses_dashboard_layout;
    if (on_clock_page) {
        events.onBackgroundConnected();
    }
}
