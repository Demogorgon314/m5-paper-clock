#include <unity.h>

#include "logic/SegmentLogic.h"
#include "logic/UiLogic.h"

void test_timezone_clamp() {
    TEST_ASSERT_EQUAL(-11, logic::ClampTimeZone(-99));
    TEST_ASSERT_EQUAL(12, logic::ClampTimeZone(99));
    TEST_ASSERT_EQUAL(8, logic::ClampTimeZone(8));
}

void test_wifi_pagination() {
    TEST_ASSERT_EQUAL(3, logic::PageCount(13, 6));
    TEST_ASSERT_EQUAL(6, logic::VisibleItemCount(13, 0, 6));
    TEST_ASSERT_EQUAL(6, logic::VisibleItemCount(13, 1, 6));
    TEST_ASSERT_EQUAL(1, logic::VisibleItemCount(13, 2, 6));
}

void test_segment_masks() {
    TEST_ASSERT_EQUAL_HEX8(logic::kSegmentTop | logic::kSegmentUpperLeft |
                               logic::kSegmentUpperRight |
                               logic::kSegmentLowerLeft |
                               logic::kSegmentLowerRight |
                               logic::kSegmentBottom,
                           logic::SegmentMaskForDigit('0'));
    TEST_ASSERT_EQUAL_HEX8(logic::kSegmentUpperRight |
                               logic::kSegmentLowerRight,
                           logic::SegmentMaskForDigit('1'));
    TEST_ASSERT_EQUAL_HEX8(logic::kSegmentTop | logic::kSegmentUpperLeft |
                               logic::kSegmentUpperRight |
                               logic::kSegmentMiddle |
                               logic::kSegmentLowerLeft |
                               logic::kSegmentLowerRight |
                               logic::kSegmentBottom,
                           logic::SegmentMaskForDigit('8'));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_timezone_clamp);
    RUN_TEST(test_wifi_pagination);
    RUN_TEST(test_segment_masks);
    return UNITY_END();
}
