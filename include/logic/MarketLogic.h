#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace logic {

struct TencentQuoteSnapshot {
    std::string name;
    std::string code;
    std::string price;
    std::string timestamp;
    std::string change;
    std::string change_percent;
    bool valid = false;
    bool positive = false;
};

inline bool IsValidUtf8(const std::string& value) {
    size_t index = 0;
    while (index < value.size()) {
        const unsigned char lead =
            static_cast<unsigned char>(value[index]);
        size_t expected_length = 0;
        if ((lead & 0x80) == 0x00) {
            expected_length = 1;
        } else if ((lead & 0xE0) == 0xC0) {
            expected_length = 2;
            if (lead < 0xC2) {
                return false;
            }
        } else if ((lead & 0xF0) == 0xE0) {
            expected_length = 3;
        } else if ((lead & 0xF8) == 0xF0) {
            expected_length = 4;
            if (lead > 0xF4) {
                return false;
            }
        } else {
            return false;
        }

        if (index + expected_length > value.size()) {
            return false;
        }

        for (size_t offset = 1; offset < expected_length; ++offset) {
            const unsigned char continuation =
                static_cast<unsigned char>(value[index + offset]);
            if ((continuation & 0xC0) != 0x80) {
                return false;
            }
        }

        if (expected_length == 3) {
            const unsigned char second =
                static_cast<unsigned char>(value[index + 1]);
            if ((lead == 0xE0 && second < 0xA0) ||
                (lead == 0xED && second >= 0xA0)) {
                return false;
            }
        } else if (expected_length == 4) {
            const unsigned char second =
                static_cast<unsigned char>(value[index + 1]);
            if ((lead == 0xF0 && second < 0x90) ||
                (lead == 0xF4 && second >= 0x90)) {
                return false;
            }
        }

        index += expected_length;
    }
    return true;
}

inline std::string KnownMarketDisplayName(const std::string& code) {
    if (code == "000001") {
        return u8"上证指数";
    }
    return "";
}

inline std::string NormalizeTencentQuoteName(const std::string& code,
                                             const std::string& name) {
    if (!name.empty() && IsValidUtf8(name)) {
        return name;
    }

    const std::string fallback_name = KnownMarketDisplayName(code);
    if (!fallback_name.empty()) {
        return fallback_name;
    }

    if (!name.empty()) {
        return name;
    }
    return code;
}

inline std::vector<std::string> SplitTencentQuoteFields(
    const std::string& payload) {
    std::vector<std::string> fields;
    size_t start = 0;
    while (start <= payload.size()) {
        const size_t end = payload.find('~', start);
        if (end == std::string::npos) {
            fields.push_back(payload.substr(start));
            break;
        }
        fields.push_back(payload.substr(start, end - start));
        start = end + 1;
    }
    return fields;
}

inline bool IsTencentQuoteTimestamp(const std::string& value) {
    return value.size() == 14 &&
           std::all_of(value.begin(), value.end(), [](unsigned char ch) {
               return std::isdigit(ch) != 0;
           });
}

inline TencentQuoteSnapshot ParseTencentQuote(const std::string& body) {
    TencentQuoteSnapshot snapshot;

    const size_t first_quote = body.find('"');
    const size_t last_quote = body.rfind('"');
    if (first_quote == std::string::npos || last_quote == std::string::npos ||
        last_quote <= first_quote + 1) {
        return snapshot;
    }

    const std::string payload =
        body.substr(first_quote + 1, last_quote - first_quote - 1);
    const std::vector<std::string> fields = SplitTencentQuoteFields(payload);
    if (fields.size() <= 3) {
        return snapshot;
    }

    snapshot.name = fields[1];
    snapshot.code = fields[2];
    snapshot.price = fields[3];

    size_t timestamp_index = fields.size();
    for (size_t index = 24; index < fields.size(); ++index) {
        if (IsTencentQuoteTimestamp(fields[index])) {
            timestamp_index = index;
            break;
        }
    }
    if (timestamp_index == fields.size() || timestamp_index + 2 >= fields.size()) {
        return snapshot;
    }

    snapshot.timestamp = fields[timestamp_index];
    snapshot.change = fields[timestamp_index + 1];
    snapshot.change_percent = fields[timestamp_index + 2];
    snapshot.valid = !snapshot.code.empty() && !snapshot.price.empty() &&
                     !snapshot.change.empty() &&
                     !snapshot.change_percent.empty();
    snapshot.positive = snapshot.valid && snapshot.change.front() != '-';
    return snapshot;
}

}  // namespace logic
