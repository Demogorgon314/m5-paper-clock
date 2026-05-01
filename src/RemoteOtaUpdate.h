#pragma once

#include <Arduino.h>

class RemoteOtaUpdate {
public:
    using ProgressCallback = void (*)(void* context, const char* stage,
                                      size_t written, size_t size);
    using CheckpointCallback = void (*)(void* context, const char* phase);
    using YieldCallback = void (*)(void* context);

    struct Callbacks {
        void* context = nullptr;
        ProgressCallback progress = nullptr;
        CheckpointCallback checkpoint = nullptr;
        YieldCallback yield = nullptr;
    };

    bool run(const String& url, const String& expected_sha256,
             const String& user_agent, const Callbacks& callbacks,
             String& error_message);

private:
    void notifyProgress(const Callbacks& callbacks, const char* stage,
                        size_t written, size_t size) const;
    void notifyCheckpoint(const Callbacks& callbacks, const char* phase) const;
    void notifyYield(const Callbacks& callbacks) const;
};
