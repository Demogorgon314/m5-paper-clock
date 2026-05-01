#include <unity.h>

#include <string>

#include "logic/ComfortLogic.h"
#include "logic/ClimateDisplayLogic.h"
#include "logic/AppInputLogic.h"
#include "logic/ConfigCommandLogic.h"
#include "logic/ComponentUpdateGroups.h"
#include "logic/DateDisplayLogic.h"
#include "logic/MarketLogic.h"
#include "logic/HolidayLogic.h"
#include "logic/LayoutLogic.h"
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

void test_dashboard_layout_clamps_to_screen() {
    logic::DashboardLayoutItem item =
        logic::DashboardLayoutDefaultItem(logic::DashboardComponentId::Climate);
    item.x = 900;
    item.y = -20;

    const logic::DashboardLayoutItem clamped =
        logic::ClampDashboardLayoutItem(item);

    TEST_ASSERT_EQUAL(960 - item.w, clamped.x);
    TEST_ASSERT_EQUAL(0, clamped.y);
}

void test_dashboard_component_id_from_key() {
    bool found = false;
    const logic::DashboardComponentId id =
        logic::DashboardComponentIdFromKey("market-1", found);

    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::DashboardComponentId::Summary),
        static_cast<int>(id));

    const logic::DashboardComponentId type_id =
        logic::DashboardComponentIdFromKey("market", found);
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::DashboardComponentId::Summary),
        static_cast<int>(type_id));

    logic::DashboardComponentIdFromKey("unknown", found);
    TEST_ASSERT_FALSE(found);
}

void test_component_update_groups_cover_expected_components() {
    TEST_ASSERT_EQUAL(8, static_cast<int>(logic::kFullRefreshComponents.count));
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::DashboardComponentId::Date),
        static_cast<int>(logic::kFullRefreshComponents.ids[0]));
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::DashboardComponentId::ClassicInfo),
        static_cast<int>(logic::kFullRefreshComponents.ids[7]));

    TEST_ASSERT_EQUAL(2, static_cast<int>(logic::kMinuteComponents.count));
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::DashboardComponentId::Time),
        static_cast<int>(logic::kMinuteComponents.ids[0]));
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::DashboardComponentId::ClassicTime),
        static_cast<int>(logic::kMinuteComponents.ids[1]));

    TEST_ASSERT_EQUAL(1, static_cast<int>(logic::kMarketComponents.count));
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::DashboardComponentId::Summary),
        static_cast<int>(logic::kMarketComponents.ids[0]));
}

bool component_group_contains(const logic::ComponentUpdateGroup& group,
                              logic::DashboardComponentId id) {
    for (size_t index = 0; index < group.count; ++index) {
        if (group.ids[index] == id) {
            return true;
        }
    }
    return false;
}

void assert_component_update_group_matches_flag(
    const logic::ComponentUpdateGroup& group, logic::ComponentUpdateFlag flag) {
    size_t expected_count = 0;
    for (const logic::DashboardComponentDefinition& definition :
         logic::kDashboardComponentDefinitions) {
        const bool has_flag =
            logic::HasComponentUpdateFlag(definition.update_flags, flag);
        TEST_ASSERT_EQUAL(has_flag,
                          component_group_contains(group,
                                                   definition.default_item.id));
        if (has_flag) {
            ++expected_count;
        }
    }
    TEST_ASSERT_EQUAL(expected_count, group.count);
}

void test_component_update_groups_match_registry_flags() {
    assert_component_update_group_matches_flag(logic::kFullRefreshComponents,
                                               logic::kUpdateOnFullRefresh);
    assert_component_update_group_matches_flag(logic::kMinuteComponents,
                                               logic::kUpdateOnMinute);
    assert_component_update_group_matches_flag(logic::kDateComponents,
                                               logic::kUpdateOnDate);
    assert_component_update_group_matches_flag(logic::kSensorComponents,
                                               logic::kUpdateOnSensor);
    assert_component_update_group_matches_flag(logic::kMarketComponents,
                                               logic::kUpdateOnMarket);
}

void test_component_registry_exposes_defaults_and_labels() {
    const logic::DashboardLayout layout = logic::DefaultDashboardLayout();

    TEST_ASSERT_EQUAL(logic::kDashboardComponentCount, layout.size());
    TEST_ASSERT_EQUAL_STRING(
        "date",
        logic::DashboardComponentPerfLabel(logic::DashboardComponentId::Date));
    TEST_ASSERT_EQUAL_STRING(
        "summary",
        logic::DashboardComponentPerfLabel(logic::DashboardComponentId::Summary));
    TEST_ASSERT_EQUAL_STRING(
        "market",
        logic::DashboardComponentDefinitionForId(
            logic::DashboardComponentId::Summary).default_item.type);
    TEST_ASSERT_EQUAL(332, logic::MarketComponentDefaultItem().w);
    TEST_ASSERT_EQUAL(4,
                      static_cast<int>(
                          logic::kClassicLayoutComponentDefinitions.size()));
    TEST_ASSERT_EQUAL_STRING(
        "classic-time",
        logic::kClassicLayoutComponentDefinitions[2].instance_id);
}

void test_wifi_pagination() {
    TEST_ASSERT_EQUAL(3, logic::PageCount(13, 6));
    TEST_ASSERT_EQUAL(6, logic::VisibleItemCount(13, 0, 6));
    TEST_ASSERT_EQUAL(6, logic::VisibleItemCount(13, 1, 6));
    TEST_ASSERT_EQUAL(1, logic::VisibleItemCount(13, 2, 6));
}

void test_wifi_signal_levels() {
    TEST_ASSERT_EQUAL(4, logic::WifiSignalLevelFromRssi(-55));
    TEST_ASSERT_EQUAL(3, logic::WifiSignalLevelFromRssi(-67));
    TEST_ASSERT_EQUAL(2, logic::WifiSignalLevelFromRssi(-75));
    TEST_ASSERT_EQUAL(1, logic::WifiSignalLevelFromRssi(-76));
    TEST_ASSERT_EQUAL(3, logic::WifiBitmapLevel(4));
    TEST_ASSERT_EQUAL(2, logic::WifiBitmapLevel(2));
    TEST_ASSERT_EQUAL(1, logic::WifiBitmapLevel(1));
}

void test_password_mask_and_keyboard_layouts() {
    TEST_ASSERT_EQUAL_STRING("<empty>", logic::MaskPassword("", false).c_str());
    TEST_ASSERT_EQUAL_STRING("******",
                             logic::MaskPassword("secret", false).c_str());
    TEST_ASSERT_EQUAL_STRING("secret",
                             logic::MaskPassword("secret", true).c_str());

    const std::vector<std::string> lower = logic::KeyboardLayout(false, false);
    TEST_ASSERT_EQUAL(26, static_cast<int>(lower.size()));
    TEST_ASSERT_EQUAL_STRING("q", lower.front().c_str());
    TEST_ASSERT_EQUAL_STRING("m", lower.back().c_str());

    const std::vector<std::string> upper = logic::KeyboardLayout(true, false);
    TEST_ASSERT_EQUAL_STRING("Q", upper.front().c_str());
    TEST_ASSERT_EQUAL_STRING("M", upper.back().c_str());

    const std::vector<std::string> symbols = logic::KeyboardLayout(false, true);
    TEST_ASSERT_EQUAL(26, static_cast<int>(symbols.size()));
    TEST_ASSERT_EQUAL_STRING("1", symbols.front().c_str());
    TEST_ASSERT_EQUAL_STRING("&", symbols.back().c_str());
}

void assert_button_action(logic::ButtonAction expected_action,
                          int expected_index,
                          logic::AppPage page,
                          int button_id,
                          int wifi_page_index = 0,
                          int wifi_network_count = 0,
                          int wifi_page_size = 6) {
    const logic::ButtonActionDecision decision = logic::DecideButtonAction(
        page, button_id, wifi_page_index, wifi_network_count, wifi_page_size);
    TEST_ASSERT_EQUAL(static_cast<int>(expected_action),
                      static_cast<int>(decision.action));
    TEST_ASSERT_EQUAL(expected_index, decision.index);
}

void test_button_action_logic_for_settings_and_wifi_pages() {
    assert_button_action(logic::ButtonAction::OpenWifiScan, -1,
                         logic::AppPage::Settings, logic::kButtonWifi);
    assert_button_action(logic::ButtonAction::DecreaseTimezone, -1,
                         logic::AppPage::Settings,
                         logic::kButtonTimezoneMinus);
    assert_button_action(logic::ButtonAction::None, -1,
                         logic::AppPage::Settings, logic::kButtonNextPage);

    assert_button_action(logic::ButtonAction::PreviousWifiPage, -1,
                         logic::AppPage::WifiScan, logic::kButtonPrevPage, 1,
                         8);
    assert_button_action(logic::ButtonAction::None, -1,
                         logic::AppPage::WifiScan, logic::kButtonPrevPage, 0,
                         8);
    assert_button_action(logic::ButtonAction::NextWifiPage, -1,
                         logic::AppPage::WifiScan, logic::kButtonNextPage, 0,
                         8);
    assert_button_action(logic::ButtonAction::None, -1,
                         logic::AppPage::WifiScan, logic::kButtonNextPage, 1,
                         8);
    assert_button_action(logic::ButtonAction::SelectWifiNetwork, 7,
                         logic::AppPage::WifiScan, logic::kButtonNetworkBase + 1,
                         1, 8);
    assert_button_action(logic::ButtonAction::None, -1,
                         logic::AppPage::WifiScan, logic::kButtonNetworkBase + 4,
                         1, 8);
}

void test_button_action_logic_for_password_page() {
    assert_button_action(logic::ButtonAction::BackToWifiScan, -1,
                         logic::AppPage::Password, logic::kButtonBack);
    assert_button_action(logic::ButtonAction::ToggleKeyboardShift, -1,
                         logic::AppPage::Password, logic::kButtonShift);
    assert_button_action(logic::ButtonAction::ToggleKeyboardMode, -1,
                         logic::AppPage::Password, logic::kButtonKeyboardMode);
    assert_button_action(logic::ButtonAction::BackspacePassword, -1,
                         logic::AppPage::Password, logic::kButtonBackspace);
    assert_button_action(logic::ButtonAction::AppendPasswordSpace, -1,
                         logic::AppPage::Password, logic::kButtonSpace);
    assert_button_action(logic::ButtonAction::ClearPassword, -1,
                         logic::AppPage::Password, logic::kButtonClear);
    assert_button_action(logic::ButtonAction::TogglePasswordVisibility, -1,
                         logic::AppPage::Password,
                         logic::kButtonPasswordVisibility);
    assert_button_action(logic::ButtonAction::ConnectSelectedNetwork, -1,
                         logic::AppPage::Password, logic::kButtonConnect);
    assert_button_action(logic::ButtonAction::AppendKeyboardCharacter, 25,
                         logic::AppPage::Password,
                         logic::kButtonKeyboardBase + 25);
    assert_button_action(logic::ButtonAction::None, -1,
                         logic::AppPage::Password,
                         logic::kButtonKeyboardBase + 26);
}

void test_config_command_logic_parses_commands_and_policies() {
    TEST_ASSERT_EQUAL(static_cast<int>(logic::ConfigCommand::GetStatus),
                      static_cast<int>(
                          logic::ParseConfigCommand("get_status")));
    TEST_ASSERT_EQUAL(static_cast<int>(logic::ConfigCommand::ApplyLayout),
                      static_cast<int>(
                          logic::ParseConfigCommand("apply_layout")));
    TEST_ASSERT_EQUAL(static_cast<int>(logic::ConfigCommand::LocalOtaChunk),
                      static_cast<int>(
                          logic::ParseConfigCommand("local_ota_chunk")));
    TEST_ASSERT_EQUAL(static_cast<int>(logic::ConfigCommand::Unknown),
                      static_cast<int>(logic::ParseConfigCommand("missing")));
    TEST_ASSERT_EQUAL(static_cast<int>(logic::ConfigCommand::Unknown),
                      static_cast<int>(logic::ParseConfigCommand(nullptr)));

    TEST_ASSERT_FALSE(logic::ConfigCommandRequiresBleAuth(
        logic::ConfigCommand::PairBegin));
    TEST_ASSERT_FALSE(logic::ConfigCommandRequiresBleAuth(
        logic::ConfigCommand::PairVerify));
    TEST_ASSERT_TRUE(logic::ConfigCommandRequiresBleAuth(
        logic::ConfigCommand::GetStatus));

    TEST_ASSERT_TRUE(logic::ConfigCommandRequiresBluetooth(
        logic::ConfigCommand::PairBegin));
    TEST_ASSERT_FALSE(logic::ConfigCommandRequiresBluetooth(
        logic::ConfigCommand::GetStatus));

    TEST_ASSERT_TRUE(logic::ConfigCommandRequiresSerial(
        logic::ConfigCommand::LocalOtaBegin));
    TEST_ASSERT_TRUE(logic::ConfigCommandRequiresSerial(
        logic::ConfigCommand::LocalOtaEnd));
    TEST_ASSERT_FALSE(logic::ConfigCommandRequiresSerial(
        logic::ConfigCommand::LocalOtaAbort));
}

void test_date_display_normalization() {
    TEST_ASSERT_EQUAL_STRING("yyyy-mm-dd",
                             logic::NormalizeDateFormat("bad-format"));
    TEST_ASSERT_EQUAL_STRING("mm-dd", logic::NormalizeDateFormat("mm-dd"));
    TEST_ASSERT_EQUAL_STRING("short",
                             logic::NormalizeWeekdayFormat("unexpected"));
    TEST_ASSERT_EQUAL_STRING("english-short",
                             logic::NormalizeWeekdayFormat("english-short"));
    TEST_ASSERT_EQUAL_STRING("inline",
                             logic::NormalizeDateLayout("stacked"));
    TEST_ASSERT_EQUAL_STRING("two-line",
                             logic::NormalizeDateLayout("two-line"));
    TEST_ASSERT_EQUAL(7, logic::NormalizeDateTextSize(99));
    TEST_ASSERT_EQUAL(3, logic::NormalizeDateTextSize(3));
}

void test_days_in_month_handles_boundaries_and_leap_years() {
    TEST_ASSERT_EQUAL(29, logic::DaysInMonth(2024, 2));
    TEST_ASSERT_EQUAL(28, logic::DaysInMonth(2026, 2));
    TEST_ASSERT_EQUAL(30, logic::DaysInMonth(2026, 4));
    TEST_ASSERT_EQUAL(30, logic::DaysInMonth(2026, 13));
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

void test_market_display_helpers() {
    TEST_ASSERT_EQUAL_STRING(
        "000001", logic::MarketCodeFromRequestSymbol("sh000001").c_str());
    TEST_ASSERT_EQUAL_STRING(
        "abc", logic::MarketCodeFromRequestSymbol("abc").c_str());
    TEST_ASSERT_TRUE(logic::MarketSearchMatchesQuery("sh000001", "000001",
                                                     "000"));
    TEST_ASSERT_TRUE(logic::MarketSearchMatchesQuery("sh000001", "000001",
                                                     "sh"));
    TEST_ASSERT_FALSE(logic::MarketSearchMatchesQuery("sh000001", "000001",
                                                      "sz"));
    TEST_ASSERT_EQUAL_STRING(
        "10:57", logic::MarketQuoteTimeLabel("20260421105751").c_str());
    TEST_ASSERT_EQUAL_STRING(
        "Upd 10:57",
        logic::MarketStatusLabel("20260421105751", true, false).c_str());
    TEST_ASSERT_EQUAL_STRING(
        "休市 10:57",
        logic::MarketStatusLabel("20260421105751", false, true).c_str());
    TEST_ASSERT_EQUAL_STRING(
        "上证指数",
        logic::MarketDisplayLabel("000001", "上证指数", true).c_str());
    TEST_ASSERT_EQUAL_STRING(
        "000001", logic::MarketDisplayLabel("000001", "上证指数", false).c_str());
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

void test_climate_display_formatting() {
    TEST_ASSERT_EQUAL_STRING("24.5", logic::FormatTemperature(24.45f).c_str());
    TEST_ASSERT_EQUAL_STRING("49", logic::FormatHumidity(48.6f).c_str());
    TEST_ASSERT_EQUAL_STRING(" 49",
                             logic::FormatClassicHumidityField(48.6f, true).c_str());
    TEST_ASSERT_EQUAL_STRING(" --",
                             logic::FormatClassicHumidityField(48.6f, false).c_str());
    TEST_ASSERT_EQUAL_STRING("245",
                             logic::FormatClassicTemperatureField(24.5f, true).c_str());
    TEST_ASSERT_EQUAL_STRING("-35",
                             logic::FormatClassicTemperatureField(-3.5f, true).c_str());
    TEST_ASSERT_EQUAL_STRING("---",
                             logic::FormatClassicTemperatureField(24.5f, false).c_str());
    TEST_ASSERT_EQUAL_STRING("--",
                             logic::FormatDashboardHumidity(48.6f, false).c_str());
    TEST_ASSERT_EQUAL_STRING("--.-",
                             logic::FormatDashboardTemperature(24.5f, false).c_str());
}

void test_holiday_display_countdown() {
    const logic::HolidayDisplay display =
        logic::HolidayDisplayForDate(2026, 4, 21);

    TEST_ASSERT_TRUE(display.valid());
    TEST_ASSERT_EQUAL(
        static_cast<int>(logic::HolidayDisplayState::Countdown),
        static_cast<int>(display.state));
    TEST_ASSERT_EQUAL(static_cast<int>(logic::HolidayId::LaoDong),
                      static_cast<int>(display.id));
    TEST_ASSERT_EQUAL(10, display.days_remaining);
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

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_timezone_clamp);
    RUN_TEST(test_partial_clean_interval_clamp);
    RUN_TEST(test_dashboard_layout_clamps_to_screen);
    RUN_TEST(test_dashboard_component_id_from_key);
    RUN_TEST(test_component_update_groups_cover_expected_components);
    RUN_TEST(test_component_update_groups_match_registry_flags);
    RUN_TEST(test_component_registry_exposes_defaults_and_labels);
    RUN_TEST(test_wifi_pagination);
    RUN_TEST(test_wifi_signal_levels);
    RUN_TEST(test_password_mask_and_keyboard_layouts);
    RUN_TEST(test_button_action_logic_for_settings_and_wifi_pages);
    RUN_TEST(test_button_action_logic_for_password_page);
    RUN_TEST(test_config_command_logic_parses_commands_and_policies);
    RUN_TEST(test_date_display_normalization);
    RUN_TEST(test_days_in_month_handles_boundaries_and_leap_years);
    RUN_TEST(test_segment_masks);
    RUN_TEST(test_parse_tencent_quote);
    RUN_TEST(test_parse_tencent_quote_with_trade_details);
    RUN_TEST(test_normalize_tencent_quote_name_uses_known_utf8_fallback);
    RUN_TEST(test_normalize_tencent_quote_name_keeps_utf8_name);
    RUN_TEST(test_known_market_request_symbol);
    RUN_TEST(test_infer_tencent_quote_symbols_for_known_index_and_stock);
    RUN_TEST(test_market_display_helpers);
    RUN_TEST(test_weekday_from_civil);
    RUN_TEST(test_is_holiday_date);
    RUN_TEST(test_cn_a_share_market_open_during_session);
    RUN_TEST(test_cn_a_share_market_closed_during_breaks);
    RUN_TEST(test_cn_a_share_market_closed_on_weekends_and_holidays);
    RUN_TEST(test_normalize_comfort_settings_clamps_and_swaps_ranges);
    RUN_TEST(test_comfort_state_for_reading_respects_custom_thresholds);
    RUN_TEST(test_climate_display_formatting);
    RUN_TEST(test_holiday_display_countdown);
    RUN_TEST(test_holiday_name_zh);
    RUN_TEST(test_holiday_display_in_holiday);
    RUN_TEST(test_holiday_display_last_day);
    return UNITY_END();
}
