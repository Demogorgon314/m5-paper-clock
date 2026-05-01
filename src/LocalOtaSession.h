#pragma once

#include <Arduino.h>
#include <mbedtls/sha256.h>

struct LocalOtaStatus {
    bool active = false;
    bool binary_mode = false;
    size_t written = 0;
    size_t size = 0;
};

class LocalOtaSession {
public:
    bool begin(size_t size, const String& expected_sha256, bool binary_mode,
               String& error_message);
    bool writeBase64Chunk(size_t offset, const String& base64_data,
                          size_t& written, String& error_message);
    bool writeBytes(uint8_t* data, size_t length, size_t& written,
                    String& error_message);
    bool finish(String& error_message);
    void abort();

    LocalOtaStatus status() const;
    bool active() const { return active_; }
    bool binaryMode() const { return binary_mode_; }
    void stopBinaryMode() { binary_mode_ = false; }

private:
    void resetState();

    bool active_ = false;
    bool binary_mode_ = false;
    size_t expected_size_ = 0;
    size_t written_ = 0;
    String expected_sha256_;
    mbedtls_sha256_context sha_context_;
};
