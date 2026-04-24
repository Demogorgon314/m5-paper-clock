#include <unity.h>

#include <string>

#include "logic/ComfortLogic.h"
#include "logic/MarketLogic.h"
#include "logic/HolidayLogic.h"
#include "logic/SegmentLogic.h"
#include "logic/UiLogic.h"

void test_timezone_clamp() {
    TEST_ASSERT_EQUAL(-11, logic::ClampTimeZone(-99));
    TEST_ASSERT_EQUAL(12, logic::ClampTimeZone(99));
    TEST_ASSERT_EQUAL(8, logic::ClampTimeZone(8));
}

void test_partial_clean_interval_clamp() {
    TEST_ASSERT_EQUAL(1, logic::ClampPartialCleanInterval(0));
    TEST_ASSERT_EQUAL(4, logic::ClampPartialCleanInterval(4));
    TEST_ASSERT_EQUAL(30, logic::ClampPartialCleanInterval(99));
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

void test_normalize_tencent_quote_name_uses_known_utf8_fallback() {
    const std::string name_gbk = "\xC9\xCF\xD6\xA4\xD6\xB8\xCA\xFD";
    const std::string normalized =
        logic::NormalizeTencentQuoteName("000001", name_gbk);

    TEST_ASSERT_EQUAL_STRING("上证指数", normalized.c_str());
}

void test_normalize_tencent_quote_name_keeps_utf8_name() {
    const std::string normalized =
        logic::NormalizeTencentQuoteName("000001", "SSE Index");

    TEST_ASSERT_EQUAL_STRING("SSE Index", normalized.c_str());
}

void test_known_market_request_symbol() {
    TEST_ASSERT_EQUAL_STRING("sh000001",
                             logic::KnownMarketRequestSymbol("000001").c_str());
    TEST_ASSERT_EQUAL_STRING("sz399001",
                             logic::KnownMarketRequestSymbol("399001").c_str());
    TEST_ASSERT_EQUAL_STRING("",
                             logic::KnownMarketRequestSymbol("600519").c_str());
}

void test_infer_tencent_quote_symbols_for_known_index_and_stock() {
    const std::vector<std::string> index_candidates =
        logic::InferTencentQuoteSymbols("000001");
    TEST_ASSERT_EQUAL(2, static_cast<int>(index_candidates.size()));
    TEST_ASSERT_EQUAL_STRING("sh000001", index_candidates[0].c_str());
    TEST_ASSERT_EQUAL_STRING("sz000001", index_candidates[1].c_str());

    const std::vector<std::string> stock_candidates =
        logic::InferTencentQuoteSymbols("600519");
    TEST_ASSERT_EQUAL(1, static_cast<int>(stock_candidates.size()));
    TEST_ASSERT_EQUAL_STRING("sh600519", stock_candidates[0].c_str());

    const std::vector<std::string> prefixed_candidates =
        logic::InferTencentQuoteSymbols("sz300750");
    TEST_ASSERT_EQUAL(1, static_cast<int>(prefixed_candidates.size()));
    TEST_ASSERT_EQUAL_STRING("sz300750", prefixed_candidates[0].c_str());
}

void test_weekday_from_civil() {
    TEST_ASSERT_EQUAL(2, logic::WeekdayFromCivil(2026, 4, 21));
    TEST_ASSERT_EQUAL(0, logic::WeekdayFromCivil(2026, 4, 26));
}

void test_is_holiday_date() {
    TEST_ASSERT_TRUE(logic::IsHolidayDate(2026, 5, 1));
    TEST_ASSERT_FALSE(logic::IsHolidayDate(2026, 4, 21));
}

void test_cn_a_share_market_open_during_session() {
    TEST_ASSERT_TRUE(logic::IsCnAShareMarketOpen(2026, 4, 21, 10, 0));
    TEST_ASSERT_TRUE(logic::IsCnAShareMarketOpen(2026, 4, 21, 11, 30));
    TEST_ASSERT_TRUE(logic::IsCnAShareMarketOpen(2026, 4, 21, 15, 0));
}

void test_cn_a_share_market_closed_during_breaks() {
    TEST_ASSERT_FALSE(logic::IsCnAShareMarketOpen(2026, 4, 21, 9, 29));
    TEST_ASSERT_FALSE(logic::IsCnAShareMarketOpen(2026, 4, 21, 12, 0));
    TEST_ASSERT_FALSE(logic::IsCnAShareMarketOpen(2026, 4, 21, 15, 1));
}

void test_cn_a_share_market_closed_on_weekends_and_holidays() {
    TEST_ASSERT_FALSE(logic::IsCnAShareMarketOpen(2026, 4, 25, 10, 0));
    TEST_ASSERT_FALSE(logic::IsCnAShareMarketOpen(2026, 5, 1, 10, 0));
}

void test_normalize_comfort_settings_clamps_and_swaps_ranges() {
    const logic::ComfortSettings normalized =
        logic::NormalizeComfortSettings(
            logic::ComfortSettings(32.0f, 18.0f, 105.0f, -5.0f));

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 18.0f, normalized.min_temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 32.0f, normalized.max_temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, normalized.min_humidity);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, normalized.max_humidity);
}

void test_comfort_state_for_reading_respects_custom_thresholds() {
    const logic::ComfortSettings custom(22.0f, 26.0f, 35.0f, 60.0f);

    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::ComfortState::Comfortable),
        static_cast<int>(logic::ComfortStateForReading(24.5f, 48.0f, true, custom)));
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::ComfortState::Uncomfortable),
        static_cast<int>(logic::ComfortStateForReading(27.0f, 48.0f, true, custom)));
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::ComfortState::Offline),
        static_cast<int>(logic::ComfortStateForReading(24.5f, 48.0f, false, custom)));
}

void test_next_holiday_countdown() {
    const logic::HolidayCountdown countdown =
        logic::NextHolidayCountdown(2026, 4, 21);

    TEST_ASSERT_TRUE(countdown.valid);
    TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayId::LaoDong),
                      static_cast<int>(countdown.id));
    TEST_ASSERT_EQUAL(10, countdown.days_remaining);
}

void test_holiday_name_zh() {
    TEST_ASSERT_EQUAL_STRING("劳动节",
                             logic::HolidayNameZh(logic::HolidayId::LaoDong));
}

const logic::HolidayPeriod* find_holiday_period(logic::HolidayId id,
                                                int start_year) {
    for (const logic::HolidayPeriod& period : logic::kHolidayPeriods) {
        if (period.id == id && period.start_year == start_year) {
            return &period;
        }
    }
    return nullptr;
}

void test_holiday_display_in_holiday() {
    const logic::HolidayPeriod* period =
        find_holiday_period(logic::HolidayId::LaoDong, 2026);

    TEST_ASSERT_NOT_NULL(period);
    const logic::HolidayDisplay display = logic::HolidayDisplayForDate(
        period->start_year, period->start_month, period->start_day);

    if (period->total_days > 1) {
        TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayDisplayState::InHoliday),
                          static_cast<int>(display.state));
        TEST_ASSERT_EQUAL(1, display.holiday_day_index);
        TEST_ASSERT_EQUAL(period->total_days, display.holiday_total_days);
    } else {
        TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayDisplayState::LastDay),
                          static_cast<int>(display.state));
    }
    TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayId::LaoDong),
                      static_cast<int>(display.id));
}

void test_holiday_display_last_day() {
    const logic::HolidayPeriod* period =
        find_holiday_period(logic::HolidayId::LaoDong, 2026);

    TEST_ASSERT_NOT_NULL(period);
    const logic::HolidayDisplay display =
        logic::HolidayDisplayForDate(period->end_year, period->end_month,
                                     period->end_day);

    TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayDisplayState::LastDay),
                      static_cast<int>(display.state));
    TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayId::LaoDong),
                      static_cast<int>(display.id));
    TEST_ASSERT_EQUAL(period->total_days, display.holiday_total_days);
    TEST_ASSERT_EQUAL(period->total_days, display.holiday_day_index);
}

void test_next_holiday_skips_current_holiday_start() {
    const logic::HolidayCountdown countdown =
        logic::NextHolidayCountdown(2026, 5, 1);

    TEST_ASSERT_TRUE(countdown.valid);
    TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayId::DuanWu),
                      static_cast<int>(countdown.id));
    TEST_ASSERT_EQUAL(49, countdown.days_remaining);
}

void test_next_holiday_falls_back_to_next_new_year() {
    const logic::HolidayCountdown countdown =
        logic::NextHolidayCountdown(2026, 10, 8);
    const int current_days = logic::DaysFromCivil(2026, 10, 8);
    int expected_days = 0;

    for (const logic::HolidayEntry& entry : logic::kHolidayEntries) {
        if (entry.id != logic::HolidayId::YuanDan) {
            continue;
        }
        const int delta =
            logic::DaysFromCivil(entry.year, entry.month, entry.day) -
            current_days;
        if (delta <= 0) {
            continue;
        }
        if (expected_days == 0 || delta < expected_days) {
            expected_days = delta;
        }
    }

    TEST_ASSERT_TRUE(countdown.valid);
    TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayId::YuanDan),
                      static_cast<int>(countdown.id));
    TEST_ASSERT_GREATER_THAN_INT(0, expected_days);
    TEST_ASSERT_EQUAL(expected_days, countdown.days_remaining);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_timezone_clamp);
    RUN_TEST(test_partial_clean_interval_clamp);
    RUN_TEST(test_wifi_pagination);
    RUN_TEST(test_segment_masks);
    RUN_TEST(test_parse_tencent_quote);
    RUN_TEST(test_parse_tencent_quote_with_trade_details);
    RUN_TEST(test_normalize_tencent_quote_name_uses_known_utf8_fallback);
    RUN_TEST(test_normalize_tencent_quote_name_keeps_utf8_name);
    RUN_TEST(test_known_market_request_symbol);
    RUN_TEST(test_infer_tencent_quote_symbols_for_known_index_and_stock);
    RUN_TEST(test_weekday_from_civil);
    RUN_TEST(test_is_holiday_date);
    RUN_TEST(test_cn_a_share_market_open_during_session);
    RUN_TEST(test_cn_a_share_market_closed_during_breaks);
    RUN_TEST(test_cn_a_share_market_closed_on_weekends_and_holidays);
    RUN_TEST(test_normalize_comfort_settings_clamps_and_swaps_ranges);
    RUN_TEST(test_comfort_state_for_reading_respects_custom_thresholds);
    RUN_TEST(test_next_holiday_countdown);
    RUN_TEST(test_holiday_name_zh);
    RUN_TEST(test_holiday_display_in_holiday);
    RUN_TEST(test_holiday_display_last_day);
    RUN_TEST(test_next_holiday_skips_current_holiday_start);
    RUN_TEST(test_next_holiday_falls_back_to_next_new_year);
    return UNITY_END();
}
