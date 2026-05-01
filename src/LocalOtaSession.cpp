#include "LocalOtaSession.h"

#include <Update.h>
#include <mbedtls/base64.h>

#include <vector>

#include "OtaUtils.h"

bool LocalOtaSession::begin(size_t size,
                            const String& expected_sha256,
                            bool binary_mode,
                            String& error_message) {
    const String expected = ota::NormalizedSha256(expected_sha256);
    if (size == 0) {
        error_message = "Missing local OTA size";
        return false;
    }
    if (expected.length() != 64) {
        error_message = "Missing local OTA sha256";
        return false;
    }

    abort();
    if (!Update.begin(size, U_FLASH)) {
        error_message =
            String("Local OTA begin failed: ") + Update.errorString();
        return false;
    }

    mbedtls_sha256_init(&sha_context_);
    mbedtls_sha256_starts_ret(&sha_context_, 0);
    active_ = true;
    binary_mode_ = binary_mode;
    expected_size_ = size;
    written_ = 0;
    expected_sha256_ = expected;
    return true;
}

bool LocalOtaSession::writeBase64Chunk(size_t offset,
                                       const String& base64_data,
                                       size_t& written,
                                       String& error_message) {
    if (!active_) {
        error_message = "Local OTA not started";
        return false;
    }
    if (offset != written_) {
        error_message = "Local OTA offset mismatch";
        written = written_;
        return false;
    }
    if (base64_data.isEmpty()) {
        error_message = "Empty local OTA chunk";
        written = written_;
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
        written = written_;
        return false;
    }

    return writeBytes(buffer.data(), decoded_len, written, error_message);
}

bool LocalOtaSession::writeBytes(uint8_t* data,
                                 size_t length,
                                 size_t& written,
                                 String& error_message) {
    if (!active_) {
        error_message = "Local OTA not started";
        return false;
    }
    if (data == nullptr || length == 0) {
        error_message = "Empty local OTA chunk";
        written = written_;
        return false;
    }
    if (written_ + length > expected_size_) {
        error_message = "Local OTA chunk exceeds size";
        abort();
        return false;
    }

    const size_t update_written = Update.write(data, length);
    if (update_written != length) {
        error_message =
            String("Local OTA write failed: ") + Update.errorString();
        abort();
        return false;
    }

    mbedtls_sha256_update_ret(&sha_context_, data, length);
    written_ += length;
    written = written_;
    if (written_ >= expected_size_) {
        binary_mode_ = false;
    }
    return true;
}

bool LocalOtaSession::finish(String& error_message) {
    if (!active_) {
        error_message = "Local OTA not started";
        return false;
    }

    bool ok = true;
    uint8_t digest[32];
    mbedtls_sha256_finish_ret(&sha_context_, digest);
    mbedtls_sha256_free(&sha_context_);

    if (written_ != expected_size_) {
        error_message = "Local OTA incomplete upload";
        ok = false;
    }

    if (ok) {
        const String actual = ota::BytesToHex(digest, sizeof(digest));
        if (expected_sha256_ != actual) {
            error_message = "Local OTA sha256 mismatch";
            ok = false;
        }
    }

    resetState();

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

void LocalOtaSession::abort() {
    if (!active_) {
        return;
    }
    mbedtls_sha256_free(&sha_context_);
    Update.abort();
    resetState();
}

LocalOtaStatus LocalOtaSession::status() const {
    LocalOtaStatus value;
    value.active = active_;
    value.binary_mode = binary_mode_;
    value.written = written_;
    value.size = expected_size_;
    return value;
}

void LocalOtaSession::resetState() {
    active_ = false;
    binary_mode_ = false;
    expected_size_ = 0;
    written_ = 0;
    expected_sha256_.clear();
}
