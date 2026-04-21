#include <unity.h>

#include <string>

#include "logic/MarketLogic.h"
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

void test_parse_tencent_quote() {
    const std::string body =
        "v_sh000001=\"1~上证指数~000001~4074.44~4082.13~4075.85~328933804~0~0~0.00"
        "~0~0.00~0~0.00~0~0.00~0~0.00~0~0.00~0~0.00~0~0.00~0~0.00~0~~"
        "20260421105751~-7.69~-0.19~4080.93~4062.13~4074.44/328933804/"
        "601268423736~328933804~60126842~0.68~17.75~~4080.93~4062.13~0.46~"
        "628918.59~672613.76~0.00~-1~-1~1.59~0~4071.83~~~~~~60126842.3736~"
        "0.0000~0~ ~ZS~2.66~1.19~~~~4197.23~3277.55~4.74~6.85~-0.67~"
        "4805657957182~~7.58~13.75~4805657957182~~~23.79~0.01~~CNY~0~~0.00~0~\";";

    const logic::TencentQuoteSnapshot snapshot =
        logic::ParseTencentQuote(body);

    TEST_ASSERT_TRUE(snapshot.valid);
    TEST_ASSERT_EQUAL_STRING("000001", snapshot.code.c_str());
    TEST_ASSERT_EQUAL_STRING("4074.44", snapshot.price.c_str());
    TEST_ASSERT_EQUAL_STRING("-7.69", snapshot.change.c_str());
    TEST_ASSERT_EQUAL_STRING("-0.19", snapshot.change_percent.c_str());
    TEST_ASSERT_EQUAL_STRING("20260421105751", snapshot.timestamp.c_str());
    TEST_ASSERT_FALSE(snapshot.positive);
}

void test_parse_tencent_quote_with_trade_details() {
    const std::string body =
        "v_sz000858=\"51~五 粮 液~000858~27.78~27.60~27.70~417909~190109~"
        "227800~27.78~492~27.77~332~27.76~202~27.75~334~27.74~291~27.79~"
        "305~27.80~570~27.81~269~27.82~448~27.83~127~15:00:13/27.78/4365/"
        "S/12124331/24602|14:56:55/27.80/14/S/38932/24395~20121221150355~"
        "0.18~0.65~28.11~27.55~27.80/413544/1151265041~417909~116339~1.10~"
        "10.14~~28.11~27.55~2.03~1054.39~1054.52~3.64~30.36~24.84~\";";

    const logic::TencentQuoteSnapshot snapshot =
        logic::ParseTencentQuote(body);

    TEST_ASSERT_TRUE(snapshot.valid);
    TEST_ASSERT_EQUAL_STRING("000858", snapshot.code.c_str());
    TEST_ASSERT_EQUAL_STRING("27.78", snapshot.price.c_str());
    TEST_ASSERT_EQUAL_STRING("0.18", snapshot.change.c_str());
    TEST_ASSERT_EQUAL_STRING("0.65", snapshot.change_percent.c_str());
    TEST_ASSERT_EQUAL_STRING("20121221150355", snapshot.timestamp.c_str());
    TEST_ASSERT_TRUE(snapshot.positive);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_timezone_clamp);
    RUN_TEST(test_wifi_pagination);
    RUN_TEST(test_segment_masks);
    RUN_TEST(test_parse_tencent_quote);
    RUN_TEST(test_parse_tencent_quote_with_trade_details);
    return UNITY_END();
}
