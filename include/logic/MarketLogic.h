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
