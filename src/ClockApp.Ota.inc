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
        date_cjk_canvas_.unloadFont();
        date_cjk_font_ready_ = false;
    }

    cjk_font_ready_ = false;

    if (littlefs_ready_) {
        SPIFFS.end();
        littlefs_ready_ = false;
    }
}

void ClockApp::prepareBleConfigForOta() {
    if (!ble_config_ready_) {
        return;
    }

    // BLEDevice::deinit(true) can crash while the Arduino BLE stack has live
    // callbacks. Keep the service alive; the CJK font/render caches are the
    // memory we need to reclaim before the TLS handshake.
    Serial.println("[ota] BLE config service kept active");
    ble_config_rx_buffer_.clear();
    ble_config_pending_chunks_.clear();
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
        date_cjk_font_ready_ = loadCjkFont(date_cjk_canvas_, {2, 3, 7});
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
    const String expected = normalizedSha256(expected_sha256);
    if (expected.length() != 64) {
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

    status_message_ = "OTA updating";
    status_error_ = false;
    prepareForOtaUpdate();
    auto fail_after_prepare = [&]() {
        restoreAfterFailedOtaUpdate();
        return false;
    };

    std::unique_ptr<WiFiClient> plain_client;
    std::unique_ptr<WiFiClientSecure> secure_client;
    std::unique_ptr<HTTPClient> http(new HTTPClient());
    if (!http) {
        error_message = "OTA HTTP allocation failed";
        return fail_after_prepare();
    }

    const bool use_https = url.startsWith("https://");
    if (use_https) {
        secure_client.reset(new WiFiClientSecure());
        if (!secure_client) {
            error_message = "OTA HTTPS allocation failed";
            return fail_after_prepare();
        }
        secure_client->setInsecure();
        if (!http->begin(*secure_client, url)) {
            error_message = "OTA HTTPS begin failed";
            return fail_after_prepare();
        }
    } else {
        plain_client.reset(new WiFiClient());
        if (!plain_client) {
            error_message = "OTA HTTP client allocation failed";
            return fail_after_prepare();
        }
        if (!http->begin(*plain_client, url)) {
            error_message = "OTA HTTP begin failed";
            return fail_after_prepare();
        }
    }

    http->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http->setTimeout(15000);
    http->addHeader("User-Agent", "M5PaperClock/" FIRMWARE_VERSION);

    const int http_code = http->GET();
    logOtaHeap("after GET");
    if (http_code != HTTP_CODE_OK) {
        error_message = http_code > 0 ? "OTA HTTP " + String(http_code)
                                      : "OTA connect failed";
        http->end();
        return fail_after_prepare();
    }

    const int content_length = http->getSize();
    if (content_length <= 0) {
        error_message = "OTA missing content length";
        http->end();
        return fail_after_prepare();
    }
    const size_t total_size = static_cast<size_t>(content_length);
    sendOtaProgress(transport, "downloading", 0, total_size);

    if (!Update.begin(total_size, U_FLASH)) {
        error_message = String("OTA begin failed: ") + Update.errorString();
        http->end();
        return fail_after_prepare();
    }

    WiFiClient* stream = http->getStreamPtr();
    mbedtls_sha256_context sha_context;
    mbedtls_sha256_init(&sha_context);
    mbedtls_sha256_starts_ret(&sha_context, 0);

    constexpr size_t kOtaBufferSize = 1024;
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[kOtaBufferSize]);
    if (!buffer) {
        error_message = "OTA buffer allocation failed";
        mbedtls_sha256_free(&sha_context);
        http->end();
        Update.abort();
        return fail_after_prepare();
    }

    size_t written = 0;
    uint32_t last_progress_ms = millis();
    uint32_t last_report_ms = millis();
    size_t last_reported = 0;
    bool ok = true;

    while (written < total_size) {
        const size_t available = stream->available();
        if (available == 0) {
            if (millis() - last_progress_ms > 30000) {
                error_message = "OTA download timeout";
                ok = false;
                break;
            }
            delay(10);
            M5.update();
            continue;
        }

        const size_t to_read = std::min(available, kOtaBufferSize);
        const int read_count = stream->readBytes(buffer.get(), to_read);
        if (read_count <= 0) {
            continue;
        }

        const size_t update_written =
            Update.write(buffer.get(), static_cast<size_t>(read_count));
        if (update_written != static_cast<size_t>(read_count)) {
            error_message = String("OTA write failed: ") + Update.errorString();
            ok = false;
            break;
        }

        mbedtls_sha256_update_ret(
            &sha_context, buffer.get(), static_cast<size_t>(read_count));
        written += static_cast<size_t>(read_count);
        last_progress_ms = millis();
        if (written >= total_size || written - last_reported >= 32768 ||
            millis() - last_report_ms >= 1000) {
            last_reported = written;
            last_report_ms = millis();
            sendOtaProgress(transport, "downloading", written, total_size);
        }
        M5.update();
    }

    if (ok) {
        sendOtaProgress(transport, "verifying", written, total_size);
    }

    uint8_t digest[32];
    mbedtls_sha256_finish_ret(&sha_context, digest);
    mbedtls_sha256_free(&sha_context);
    http->end();

    if (ok && written != total_size) {
        error_message = "OTA incomplete download";
        ok = false;
    }

    if (ok) {
        const String actual = bytesToHex(digest, sizeof(digest));
        if (expected != actual) {
            error_message = "OTA sha256 mismatch";
            ok = false;
        }
    }

    if (!ok) {
        Update.abort();
        return fail_after_prepare();
    }

    if (!Update.end(true)) {
        error_message = String("OTA end failed: ") + Update.errorString();
        return fail_after_prepare();
    }
    if (!Update.isFinished()) {
        error_message = "OTA not finished";
        return fail_after_prepare();
    }
    sendOtaProgress(transport, "installed", total_size, total_size);
    return true;
}

bool ClockApp::beginLocalOtaUpdate(size_t size,
                                   const String& expected_sha256,
                                   bool binary_mode,
                                   String& error_message) {
    const String expected = normalizedSha256(expected_sha256);
    if (size == 0) {
        error_message = "Missing local OTA size";
        return false;
    }
    if (expected.length() != 64) {
        error_message = "Missing local OTA sha256";
        return false;
    }

    abortLocalOtaUpdate();
    if (!Update.begin(size, U_FLASH)) {
        error_message =
            String("Local OTA begin failed: ") + Update.errorString();
        return false;
    }

    mbedtls_sha256_init(&local_ota_sha_context_);
    mbedtls_sha256_starts_ret(&local_ota_sha_context_, 0);
    local_ota_active_ = true;
    local_ota_binary_mode_ = binary_mode;
    local_ota_expected_size_ = size;
    local_ota_written_ = 0;
    local_ota_last_reported_ = 0;
    local_ota_expected_sha256_ = expected;
    status_message_ = "Local OTA upload ready";
    status_error_ = false;
    return true;
}

bool ClockApp::writeLocalOtaChunk(size_t offset,
                                  const String& base64_data,
                                  size_t& written,
                                  String& error_message) {
    if (!local_ota_active_) {
        error_message = "Local OTA not started";
        return false;
    }
    if (offset != local_ota_written_) {
        error_message = "Local OTA offset mismatch";
        written = local_ota_written_;
        return false;
    }
    if (base64_data.isEmpty()) {
        error_message = "Empty local OTA chunk";
        written = local_ota_written_;
        return false;
    }

    const size_t max_decoded_len = (base64_data.length() / 4) * 3;
    std::vector<uint8_t> buffer(max_decoded_len);
    size_t decoded_len = 0;
    const int decode_result = mbedtls_base64_decode(
        buffer.data(), buffer.size(), &decoded_len,
        reinterpret_cast<const unsigned char*>(base64_data.c_str()),
        base64_data.length());
    if (decode_result != 0 || decoded_len == 0) {
        error_message = "Invalid local OTA chunk";
        written = local_ota_written_;
        return false;
    }
    if (local_ota_written_ + decoded_len > local_ota_expected_size_) {
        error_message = "Local OTA chunk exceeds size";
        abortLocalOtaUpdate();
        return false;
    }

    return writeLocalOtaBytes(buffer.data(), decoded_len, written,
                              error_message);
}

bool ClockApp::writeLocalOtaBytes(uint8_t* data,
                                  size_t length,
                                  size_t& written,
                                  String& error_message) {
    if (!local_ota_active_) {
        error_message = "Local OTA not started";
        return false;
    }
    if (data == nullptr || length == 0) {
        error_message = "Empty local OTA chunk";
        written = local_ota_written_;
        return false;
    }
    if (local_ota_written_ + length > local_ota_expected_size_) {
        error_message = "Local OTA chunk exceeds size";
        abortLocalOtaUpdate();
        return false;
    }

    const size_t update_written = Update.write(data, length);
    if (update_written != length) {
        error_message =
            String("Local OTA write failed: ") + Update.errorString();
        abortLocalOtaUpdate();
        return false;
    }

    mbedtls_sha256_update_ret(&local_ota_sha_context_, data, length);
    local_ota_written_ += length;
    written = local_ota_written_;
    status_message_ = "Local OTA uploading";
    status_error_ = false;
    if (local_ota_written_ >= local_ota_expected_size_) {
        local_ota_binary_mode_ = false;
        status_message_ = "Local OTA upload complete";
    }
    M5.update();
    return true;
}

bool ClockApp::finishLocalOtaUpdate(String& error_message) {
    if (!local_ota_active_) {
        error_message = "Local OTA not started";
        return false;
    }

    bool ok = true;
    uint8_t digest[32];
    mbedtls_sha256_finish_ret(&local_ota_sha_context_, digest);
    mbedtls_sha256_free(&local_ota_sha_context_);

    if (local_ota_written_ != local_ota_expected_size_) {
        error_message = "Local OTA incomplete upload";
        ok = false;
    }

    if (ok) {
        const String actual = bytesToHex(digest, sizeof(digest));
        if (local_ota_expected_sha256_ != actual) {
            error_message = "Local OTA sha256 mismatch";
            ok = false;
        }
    }

    local_ota_active_ = false;
    local_ota_binary_mode_ = false;
    local_ota_expected_size_ = 0;
    local_ota_written_ = 0;
    local_ota_last_reported_ = 0;
    local_ota_expected_sha256_.clear();

    if (!ok) {
        Update.abort();
        return false;
    }

    if (!Update.end(true)) {
        error_message =
            String("Local OTA end failed: ") + Update.errorString();
        return false;
    }
    if (!Update.isFinished()) {
        error_message = "Local OTA not finished";
        return false;
    }
    return true;
}

void ClockApp::abortLocalOtaUpdate() {
    if (!local_ota_active_) {
        return;
    }
    mbedtls_sha256_free(&local_ota_sha_context_);
    Update.abort();
    local_ota_active_ = false;
    local_ota_binary_mode_ = false;
    local_ota_expected_size_ = 0;
    local_ota_written_ = 0;
    local_ota_last_reported_ = 0;
    local_ota_expected_sha256_.clear();
    status_message_ = "Local OTA upload aborted";
    status_error_ = false;
}

void ClockApp::populateLocalOtaStatus(JsonObject data) const {
    data["active"] = local_ota_active_;
    data["binaryMode"] = local_ota_binary_mode_;
    data["written"] = local_ota_written_;
    data["size"] = local_ota_expected_size_;
}

void ClockApp::processLocalOtaBinaryStream() {
    uint8_t buffer[512];
    String error_message;
    while (local_ota_binary_mode_ && Serial.available() > 0) {
        const size_t remaining = local_ota_expected_size_ - local_ota_written_;
        const size_t available = static_cast<size_t>(Serial.available());
        const size_t to_read =
            std::min(std::min(sizeof(buffer), remaining), available);
        if (to_read == 0) {
            local_ota_binary_mode_ = false;
            break;
        }

        const size_t read_len = Serial.readBytes(buffer, to_read);
        if (read_len == 0) {
            break;
        }

        size_t written = local_ota_written_;
        if (!writeLocalOtaBytes(buffer, read_len, written, error_message)) {
            sendLocalOtaProgress(ConfigTransport::Serial);
            return;
        }

        if (local_ota_written_ >= local_ota_expected_size_ ||
            local_ota_written_ - local_ota_last_reported_ >= 32768) {
            local_ota_last_reported_ = local_ota_written_;
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
    doc["ok"] = local_ota_active_;
    doc["event"] = "local_ota_progress";
    JsonObject data = doc.createNestedObject("data");
    populateLocalOtaStatus(data);
    if (!local_ota_active_) {
        doc["error"] = status_message_;
    }
    sendConfigDoc(doc, transport);
}
