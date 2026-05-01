#pragma once

#include <cstring>
#include <cstdint>

namespace logic {

enum class ConfigCommand : uint8_t {
    Unknown,
    PairBegin,
    PairVerify,
    GetStatus,
    LayoutPreviewState,
    ScanWifi,
    ApplySettings,
    SearchMarket,
    SetMarket,
    ApplyLayout,
    SyncTime,
    OtaUpdate,
    LocalOtaBegin,
    LocalOtaChunk,
    LocalOtaStatus,
    LocalOtaEnd,
    LocalOtaAbort,
    RefreshScreen,
    Reboot,
};

inline bool ConfigCommandEquals(const char* value, const char* expected) {
    return value && std::strcmp(value, expected) == 0;
}

inline ConfigCommand ParseConfigCommand(const char* value) {
    if (ConfigCommandEquals(value, "pair_begin")) {
        return ConfigCommand::PairBegin;
    }
    if (ConfigCommandEquals(value, "pair_verify")) {
        return ConfigCommand::PairVerify;
    }
    if (ConfigCommandEquals(value, "get_status")) {
        return ConfigCommand::GetStatus;
    }
    if (ConfigCommandEquals(value, "layout_preview_state")) {
        return ConfigCommand::LayoutPreviewState;
    }
    if (ConfigCommandEquals(value, "scan_wifi")) {
        return ConfigCommand::ScanWifi;
    }
    if (ConfigCommandEquals(value, "apply_settings")) {
        return ConfigCommand::ApplySettings;
    }
    if (ConfigCommandEquals(value, "search_market")) {
        return ConfigCommand::SearchMarket;
    }
    if (ConfigCommandEquals(value, "set_market")) {
        return ConfigCommand::SetMarket;
    }
    if (ConfigCommandEquals(value, "apply_layout")) {
        return ConfigCommand::ApplyLayout;
    }
    if (ConfigCommandEquals(value, "sync_time")) {
        return ConfigCommand::SyncTime;
    }
    if (ConfigCommandEquals(value, "ota_update")) {
        return ConfigCommand::OtaUpdate;
    }
    if (ConfigCommandEquals(value, "local_ota_begin")) {
        return ConfigCommand::LocalOtaBegin;
    }
    if (ConfigCommandEquals(value, "local_ota_chunk")) {
        return ConfigCommand::LocalOtaChunk;
    }
    if (ConfigCommandEquals(value, "local_ota_status")) {
        return ConfigCommand::LocalOtaStatus;
    }
    if (ConfigCommandEquals(value, "local_ota_end")) {
        return ConfigCommand::LocalOtaEnd;
    }
    if (ConfigCommandEquals(value, "local_ota_abort")) {
        return ConfigCommand::LocalOtaAbort;
    }
    if (ConfigCommandEquals(value, "refresh_screen")) {
        return ConfigCommand::RefreshScreen;
    }
    if (ConfigCommandEquals(value, "reboot")) {
        return ConfigCommand::Reboot;
    }
    return ConfigCommand::Unknown;
}

inline bool ConfigCommandRequiresBleAuth(ConfigCommand command) {
    return command != ConfigCommand::PairBegin &&
           command != ConfigCommand::PairVerify;
}

inline bool ConfigCommandRequiresBluetooth(ConfigCommand command) {
    return command == ConfigCommand::PairBegin ||
           command == ConfigCommand::PairVerify;
}

inline bool ConfigCommandRequiresSerial(ConfigCommand command) {
    return command == ConfigCommand::LocalOtaBegin ||
           command == ConfigCommand::LocalOtaChunk ||
           command == ConfigCommand::LocalOtaStatus ||
           command == ConfigCommand::LocalOtaEnd;
}

}  // namespace logic
