#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "MarketService.h"
#include "SensorService.h"
#include "logic/ComfortLogic.h"
#include "logic/HolidayLogic.h"

namespace displayfmt {

String TwoDigits(int value);
String TimeDigits(const rtc_time_t& time);
String DateTime(const rtc_date_t& date, const rtc_time_t& time);
String DateOnly(const rtc_date_t& date);
String ConfiguredDate(const rtc_date_t& date, const String& format);
String ClassicHumidityField(const EnvironmentReading& reading);
String ClassicTemperatureField(const EnvironmentReading& reading);
String DashboardHumidity(const EnvironmentReading& reading);
String DashboardTemperature(const EnvironmentReading& reading);
String NormalizeDateFormat(const String& value);
String NormalizeWeekdayFormat(const String& value);
String NormalizeDateLayout(const String& value);
uint8_t NormalizeDateTextSize(uint8_t value);
String HolidayDisplaySignature(const logic::HolidayDisplay& display);
String HolidayDisplayText(const logic::HolidayDisplay& display,
                          bool show_progress);
String HolidayText(const rtc_date_t& date, bool show_holiday);
String WeekdayLabel(uint8_t week);
String WeekdayLabel(uint8_t week, const String& format);
uint8_t CalculateWeekday(const rtc_date_t& date);
uint8_t DaysInMonth(int year, int month);
String DashboardMonth(const rtc_date_t& date);
String MarketQuoteTimeLabel(const MarketQuote& quote);
String MarketStatusLabel(const MarketQuote& quote,
                         bool market_open,
                         bool use_cjk_font);
String MarketDisplayLabel(const MarketQuote& quote, bool prefer_display_name);
String MarketCodeFromRequestSymbol(const String& request_symbol);
bool MarketSearchMatchesQuery(const MarketSearchResult& result,
                              const String& query);
String ComfortFace(const EnvironmentReading& reading,
                   const logic::ComfortSettings& comfort_settings);
uint8_t WifiSignalLevelFromRssi(int32_t rssi);
uint8_t WifiBitmapLevel(uint8_t signal_level);

}  // namespace displayfmt
