// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
void ClockApp::logOtaHeap(const char* phase) const {
    Serial.printf(
        "[ota] heap %s free=%u largest=%u internal=%u internal_largest=%u "
        "psram=%u psram_largest=%u\n",
        phase, ESP.getFreeHeap(),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL |
                                         MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM |
                                         MALLOC_CAP_8BIT));
}

void ClockApp::releaseTypographyForOta() {
    if (date_cjk_font_ready_) {
        render_.date_cjk_canvas.unloadFont();
        date_cjk_font_ready_ = false;
    }

    cjk_font_ready_ = false;

    if (littlefs_ready_) {
        SPIFFS.end();
        littlefs_ready_ = false;
    }
}

void ClockApp::prepareBleConfigForOta() {
    if (!ble_config_.ready()) {
        return;
    }

    // BLEDevice::deinit(true) can crash while the Arduino BLE stack has live
    // callbacks. Keep the service alive; the CJK font/render caches are the
    // memory we need to reclaim before the TLS handshake.
    Serial.println("[ota] BLE config service kept active");
    ble_config_.clearRx();
}

void ClockApp::prepareForOtaUpdate() {
    logOtaHeap("before prepare");
    prepareBleConfigForOta();
    releaseTypographyForOta();
    logOtaHeap("after prepare");
}

void ClockApp::restoreAfterFailedOtaUpdate() {
    if (!littlefs_ready_) {
        littlefs_ready_ = SPIFFS.begin(false);
    }
    if (littlefs_ready_ && !date_cjk_font_ready_) {
        date_cjk_font_ready_ = loadCjkFont(render_.date_cjk_canvas, {2, 3, 7});
    }
    renderPage(UPDATE_MODE_GC16, true);
}

bool ClockApp::performOtaUpdate(const String& url,
                                const String& expected_sha256,
                                ConfigTransport transport,
                                String& error_message) {
    if (url.isEmpty()) {
        error_message = "Missing OTA URL";
        return false;
    }
    if (ota::NormalizedSha256(expected_sha256).length() != 64) {
        error_message = "Missing OTA sha256";
        return false;
    }
    if (!connectivity_.isConnected()) {
        if (!connectivity_.ensureConnected(settings_.ssid, settings_.password,
                                           10000)) {
            error_message = "Wi-Fi is offline";
            return false;
        }
    }
    connectivity_.ensureDnsConfigured();
    connectivity_.logNetworkDiagnostics("before OTA");

    status_message_ = "OTA updating";
    status_error_ = false;
    prepareForOtaUpdate();
    RemoteOtaCallbackContext callback_context;
    callback_context.app = this;
    callback_context.transport = transport;
    RemoteOtaUpdate::Callbacks callbacks;
    callbacks.context = &callback_context;
    callbacks.progress = &ClockApp::handleRemoteOtaProgress;
    callbacks.checkpoint = &ClockApp::handleRemoteOtaCheckpoint;
    callbacks.yield = &ClockApp::handleRemoteOtaYield;

    const String user_agent = String("M5PaperClock/") + FIRMWARE_VERSION;
    const bool ok =
        remote_ota_.run(url, expected_sha256, user_agent, callbacks,
                        error_message);
    if (!ok) {
        restoreAfterFailedOtaUpdate();
    }
    return ok;
}

bool ClockApp::beginLocalOtaUpdate(size_t size,
                                   const String& expected_sha256,
                                   bool binary_mode,
                                   String& error_message) {
    if (!local_ota_.begin(size, expected_sha256, binary_mode, error_message)) {
        return false;
    }
    local_ota_last_reported_ = 0;
    status_message_ = "Local OTA upload ready";
    status_error_ = false;
    return true;
}

bool ClockApp::writeLocalOtaChunk(size_t offset,
                                  const String& base64_data,
                                  size_t& written,
                                  String& error_message) {
    const bool ok = local_ota_.writeBase64Chunk(offset, base64_data, written,
                                                error_message);
    if (ok) {
        const LocalOtaStatus status = local_ota_.status();
        status_message_ = status.written >= status.size
                              ? "Local OTA upload complete"
                              : "Local OTA uploading";
        status_error_ = false;
    }
    return ok;
}

bool ClockApp::writeLocalOtaBytes(uint8_t* data,
                                  size_t length,
                                  size_t& written,
                                  String& error_message) {
    if (!local_ota_.writeBytes(data, length, written, error_message)) {
        return false;
    }
    status_message_ = "Local OTA uploading";
    status_error_ = false;
    const LocalOtaStatus status = local_ota_.status();
    if (status.written >= status.size) {
        status_message_ = "Local OTA upload complete";
    }
    M5.update();
    return true;
}

bool ClockApp::finishLocalOtaUpdate(String& error_message) {
    const bool ok = local_ota_.finish(error_message);
    local_ota_last_reported_ = 0;
    return ok;
}

void ClockApp::abortLocalOtaUpdate() {
    const bool was_active = local_ota_.active();
    local_ota_.abort();
    local_ota_last_reported_ = 0;
    if (!was_active) {
        return;
    }
    status_message_ = "Local OTA upload aborted";
    status_error_ = false;
}

void ClockApp::populateLocalOtaStatus(JsonObject data) const {
    const LocalOtaStatus status = local_ota_.status();
    data["active"] = status.active;
    data["binaryMode"] = status.binary_mode;
    data["written"] = status.written;
    data["size"] = status.size;
}

void ClockApp::processLocalOtaBinaryStream() {
    uint8_t buffer[512];
    String error_message;
    while (local_ota_.binaryMode() && Serial.available() > 0) {
        const LocalOtaStatus status = local_ota_.status();
        const size_t remaining = status.size - status.written;
        const size_t available = static_cast<size_t>(Serial.available());
        const size_t to_read =
            std::min(std::min(sizeof(buffer), remaining), available);
        if (to_read == 0) {
            local_ota_.stopBinaryMode();
            break;
        }

        const size_t read_len = Serial.readBytes(buffer, to_read);
        if (read_len == 0) {
            break;
        }

        size_t written = status.written;
        if (!writeLocalOtaBytes(buffer, read_len, written, error_message)) {
            sendLocalOtaProgress(ConfigTransport::Serial);
            return;
        }

        const LocalOtaStatus updated = local_ota_.status();
        if (updated.written >= updated.size ||
            updated.written - local_ota_last_reported_ >= 32768) {
            local_ota_last_reported_ = updated.written;
            sendLocalOtaProgress(ConfigTransport::Serial);
        }
    }
}

void ClockApp::sendOtaProgress(ConfigTransport transport, const char* stage,
                               size_t written, size_t size) const {
    DynamicJsonDocument doc(512);
    doc["id"] = 0;
    doc["ok"] = true;
    doc["event"] = "ota_progress";
    JsonObject data = doc.createNestedObject("data");
    data["stage"] = stage;
    data["written"] = written;
    data["size"] = size;
    sendConfigDoc(doc, transport);
}

void ClockApp::sendLocalOtaProgress(ConfigTransport transport) const {
    DynamicJsonDocument doc(512);
    doc["id"] = 0;
    doc["ok"] = local_ota_.active();
    doc["event"] = "local_ota_progress";
    JsonObject data = doc.createNestedObject("data");
    populateLocalOtaStatus(data);
    if (!local_ota_.active()) {
        doc["error"] = status_message_;
    }
    sendConfigDoc(doc, transport);
}

void ClockApp::handleRemoteOtaProgress(void* context,
                                       const char* stage,
                                       size_t written,
                                       size_t size) {
    RemoteOtaCallbackContext* callback_context =
        static_cast<RemoteOtaCallbackContext*>(context);
    if (callback_context == nullptr || callback_context->app == nullptr) {
        return;
    }
    callback_context->app->sendOtaProgress(callback_context->transport, stage,
                                           written, size);
}

void ClockApp::handleRemoteOtaCheckpoint(void* context, const char* phase) {
    RemoteOtaCallbackContext* callback_context =
        static_cast<RemoteOtaCallbackContext*>(context);
    if (callback_context == nullptr || callback_context->app == nullptr) {
        return;
    }
    callback_context->app->logOtaHeap(phase);
}

void ClockApp::handleRemoteOtaYield(void* context) {
    (void)context;
    M5.update();
}
