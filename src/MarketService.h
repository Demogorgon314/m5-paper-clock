#pragma once

#include <Arduino.h>
#include <vector>

struct MarketQuote {
    String request_symbol = "sh000001";
    String symbol = "000001";
    String display_name = "上证指数";
    String price = "--";
    String change = "--";
    String change_percent = "--";
    String updated_at;
    String error_message;
    int status_code = 0;
    bool valid = false;
    bool positive = false;
};

struct MarketSearchResult {
    String request_symbol;
    String code;
    String display_name;
    String kind;
};

class MarketService {
public:
    MarketQuote fetchQuote(const String& request_symbol) const;
    std::vector<MarketSearchResult> searchMarkets(const String& query) const;
    static std::vector<MarketSearchResult> hotMarkets();
};
