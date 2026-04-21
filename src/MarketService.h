#pragma once

#include <Arduino.h>

struct MarketQuote {
    String symbol = "SH000001";
    String display_name = "SSE Index";
    String price = "--";
    String change = "--";
    String change_percent = "--";
    String updated_at;
    String error_message;
    int status_code = 0;
    bool valid = false;
    bool positive = false;
};

class MarketService {
public:
    MarketQuote fetchShanghaiIndex() const;
};
