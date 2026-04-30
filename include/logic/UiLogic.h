#pragma once

#include <string>
#include <vector>

namespace logic {

static constexpr int kDefaultPartialCleanInterval = 16;
static constexpr int kMinPartialCleanInterval = 1;
static constexpr int kMaxPartialCleanInterval = 30;

inline int ClampTimeZone(int value) {
    return value < -11 ? -11 : (value > 12 ? 12 : value);
}

inline int ClampPartialCleanInterval(int value) {
    return value < kMinPartialCleanInterval
               ? kMinPartialCleanInterval
               : (value > kMaxPartialCleanInterval
                      ? kMaxPartialCleanInterval
                      : value);
}

inline int PageCount(int total_items, int page_size) {
    return total_items <= 0 ? 1 : ((total_items - 1) / page_size) + 1;
}

inline int PageStart(int page_index, int page_size) {
    return page_index < 0 ? 0 : page_index * page_size;
}

inline int VisibleItemCount(int total_items, int page_index, int page_size) {
    const int start = PageStart(page_index, page_size);
    if (start >= total_items) {
        return 0;
    }
    const int remaining = total_items - start;
    return remaining < page_size ? remaining : page_size;
}

inline int WifiSignalLevelFromRssi(int rssi) {
    if (rssi >= -55) {
        return 4;
    }
    if (rssi >= -67) {
        return 3;
    }
    if (rssi >= -75) {
        return 2;
    }
    return 1;
}

inline int WifiBitmapLevel(int signal_level) {
    if (signal_level >= 3) {
        return 3;
    }
    if (signal_level >= 2) {
        return 2;
    }
    return 1;
}

inline std::string MaskPassword(const std::string& password,
                                bool visible) {
    if (password.empty()) {
        return "<empty>";
    }
    if (visible) {
        return password;
    }
    return std::string(password.size(), '*');
}

inline std::vector<std::string> KeyboardLayout(bool upper, bool symbols) {
    if (symbols) {
        return {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                "-", "/", "@", ".", ":", ";", "(", ")", "!",
                "_", "\"", ",", "?", "#", "$", "&"};
    }

    if (upper) {
        return {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
                "A", "S", "D", "F", "G", "H", "J", "K", "L",
                "Z", "X", "C", "V", "B", "N", "M"};
    }

    return {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
            "a", "s", "d", "f", "g", "h", "j", "k", "l",
            "z", "x", "c", "v", "b", "n", "m"};
}

}  // namespace logic
