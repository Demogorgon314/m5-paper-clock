#pragma once

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

}  // namespace logic
