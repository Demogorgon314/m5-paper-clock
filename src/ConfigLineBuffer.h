#pragma once

#include <Arduino.h>

class ConfigLineBuffer {
public:
    explicit ConfigLineBuffer(size_t max_length) : max_length_(max_length) {
    }

    bool append(char ch, String& line) {
        if (ch == '\r') {
            return false;
        }
        if (ch == '\n') {
            line = buffer_;
            buffer_.clear();
            line.trim();
            return true;
        }
        if (buffer_.length() < max_length_) {
            buffer_ += ch;
        }
        return false;
    }

    void clear() { buffer_.clear(); }

private:
    size_t max_length_ = 0;
    String buffer_;
};
