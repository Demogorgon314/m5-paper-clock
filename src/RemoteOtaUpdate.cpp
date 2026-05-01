#include "RemoteOtaUpdate.h"

#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <mbedtls/sha256.h>

#include <algorithm>
#include <memory>

#include "OtaUtils.h"

bool RemoteOtaUpdate::run(const String& url,
                          const String& expected_sha256,
                          const String& user_agent,
                          const Callbacks& callbacks,
                          String& error_message) {
    if (url.isEmpty()) {
        error_message = "Missing OTA URL";
        return false;
    }
    const String expected = ota::NormalizedSha256(expected_sha256);
    if (expected.length() != 64) {
        error_message = "Missing OTA sha256";
        return false;
    }

    std::unique_ptr<WiFiClient> plain_client;
    std::unique_ptr<WiFiClientSecure> secure_client;
    std::unique_ptr<HTTPClient> http(new HTTPClient());
    if (!http) {
        error_message = "OTA HTTP allocation failed";
        return false;
    }

    const bool use_https = url.startsWith("https://");
    if (use_https) {
        secure_client.reset(new WiFiClientSecure());
        if (!secure_client) {
            error_message = "OTA HTTPS allocation failed";
            return false;
        }
        secure_client->setInsecure();
        if (!http->begin(*secure_client, url)) {
            error_message = "OTA HTTPS begin failed";
            return false;
        }
    } else {
        plain_client.reset(new WiFiClient());
        if (!plain_client) {
            error_message = "OTA HTTP client allocation failed";
            return false;
        }
        if (!http->begin(*plain_client, url)) {
            error_message = "OTA HTTP begin failed";
            return false;
        }
    }

    http->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http->setTimeout(15000);
    http->addHeader("User-Agent", user_agent);

    const int http_code = http->GET();
    notifyCheckpoint(callbacks, "after GET");
    if (http_code != HTTP_CODE_OK) {
        error_message = http_code > 0 ? "OTA HTTP " + String(http_code)
                                      : "OTA connect failed";
        http->end();
        return false;
    }

    const int content_length = http->getSize();
    if (content_length <= 0) {
        error_message = "OTA missing content length";
        http->end();
        return false;
    }
    const size_t total_size = static_cast<size_t>(content_length);
    notifyProgress(callbacks, "downloading", 0, total_size);

    if (!Update.begin(total_size, U_FLASH)) {
        error_message = String("OTA begin failed: ") + Update.errorString();
        http->end();
        return false;
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
        return false;
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
            notifyYield(callbacks);
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
            notifyProgress(callbacks, "downloading", written, total_size);
        }
        notifyYield(callbacks);
    }

    if (ok) {
        notifyProgress(callbacks, "verifying", written, total_size);
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
        const String actual = ota::BytesToHex(digest, sizeof(digest));
        if (expected != actual) {
            error_message = "OTA sha256 mismatch";
            ok = false;
        }
    }

    if (!ok) {
        Update.abort();
        return false;
    }

    if (!Update.end(true)) {
        error_message = String("OTA end failed: ") + Update.errorString();
        return false;
    }
    if (!Update.isFinished()) {
        error_message = "OTA not finished";
        return false;
    }
    notifyProgress(callbacks, "installed", total_size, total_size);
    return true;
}

void RemoteOtaUpdate::notifyProgress(const Callbacks& callbacks,
                                     const char* stage,
                                     size_t written,
                                     size_t size) const {
    if (callbacks.progress) {
        callbacks.progress(callbacks.context, stage, written, size);
    }
}

void RemoteOtaUpdate::notifyCheckpoint(const Callbacks& callbacks,
                                       const char* phase) const {
    if (callbacks.checkpoint) {
        callbacks.checkpoint(callbacks.context, phase);
    }
}

void RemoteOtaUpdate::notifyYield(const Callbacks& callbacks) const {
    if (callbacks.yield) {
        callbacks.yield(callbacks.context);
    }
}
