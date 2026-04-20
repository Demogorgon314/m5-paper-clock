#pragma once

#include <cstdint>

namespace logic {

static const uint8_t kSegmentTop = 1 << 0;
static const uint8_t kSegmentUpperLeft = 1 << 1;
static const uint8_t kSegmentUpperRight = 1 << 2;
static const uint8_t kSegmentMiddle = 1 << 3;
static const uint8_t kSegmentLowerLeft = 1 << 4;
static const uint8_t kSegmentLowerRight = 1 << 5;
static const uint8_t kSegmentBottom = 1 << 6;

inline uint8_t SegmentMaskForDigit(char digit) {
    switch (digit) {
        case '0':
            return kSegmentTop | kSegmentUpperLeft | kSegmentUpperRight |
                   kSegmentLowerLeft | kSegmentLowerRight | kSegmentBottom;
        case '1':
            return kSegmentUpperRight | kSegmentLowerRight;
        case '2':
            return kSegmentTop | kSegmentUpperRight | kSegmentMiddle |
                   kSegmentLowerLeft | kSegmentBottom;
        case '3':
            return kSegmentTop | kSegmentUpperRight | kSegmentMiddle |
                   kSegmentLowerRight | kSegmentBottom;
        case '4':
            return kSegmentUpperLeft | kSegmentUpperRight | kSegmentMiddle |
                   kSegmentLowerRight;
        case '5':
            return kSegmentTop | kSegmentUpperLeft | kSegmentMiddle |
                   kSegmentLowerRight | kSegmentBottom;
        case '6':
            return kSegmentTop | kSegmentUpperLeft | kSegmentMiddle |
                   kSegmentLowerLeft | kSegmentLowerRight | kSegmentBottom;
        case '7':
            return kSegmentTop | kSegmentUpperRight | kSegmentLowerRight;
        case '8':
            return kSegmentTop | kSegmentUpperLeft | kSegmentUpperRight |
                   kSegmentMiddle | kSegmentLowerLeft | kSegmentLowerRight |
                   kSegmentBottom;
        case '9':
            return kSegmentTop | kSegmentUpperLeft | kSegmentUpperRight |
                   kSegmentMiddle | kSegmentLowerRight | kSegmentBottom;
        default:
            return 0;
    }
}

}  // namespace logic
