#include "MarketService.h"

#include <WiFiClient.h>

#include "logic/MarketLogic.h"

namespace {
constexpr const char* kShanghaiIndexHost = "qt.gtimg.cn";
constexpr uint16_t kShanghaiIndexPort = 80;
constexpr int32_t kHttpConnectTimeoutMs = 10000;
constexpr uint16_t kHttpTimeoutMs = 5000;
const IPAddress kMarketFallbackIps[] = {
    IPAddress(119, 147, 12, 144),
    IPAddress(119, 147, 12, 145),
};

struct MarketPreset {
    const char* request_symbol;
    const char* code;
    const char* display_name;
    const char* kind;
};

constexpr MarketPreset kHotMarketPresets[] = {
    {"sh000001", "000001", u8"上证指数", "index"},
    {"sz399001", "399001", u8"深证成指", "index"},
    {"sz399006", "399006", u8"创业板指", "index"},
    {"sh000300", "000300", u8"沪深300", "index"},
    {"sh000905", "000905", u8"中证500", "index"},
    {"sh000688", "000688", u8"科创50", "index"},
    {"sh600519", "600519", u8"贵州茅台", "stock"},
    {"sh601318", "601318", u8"中国平安", "stock"},
    {"sz000858", "000858", u8"五粮液", "stock"},
    {"sz300750", "300750", u8"宁德时代", "stock"},
    {"sz000001", "000001", u8"平安银行", "stock"},
    {"sh601127", "601127", u8"赛力斯", "stock"},
};

int parseHttpStatusCode(const String& status_line) {
    const int first_space = status_line.indexOf(' ');
    if (first_space < 0) {
        return 0;
    }

    const int second_space = status_line.indexOf(' ', first_space + 1);
    const String code =
        second_space < 0 ? status_line.substring(first_space + 1)
                         : status_line.substring(first_space + 1, second_space);
    return code.toInt();
}

bool startsWith(const String& value, const String& prefix) {
    return prefix.isEmpty() || value.startsWith(prefix);
}

void appendUniqueSearchResult(std::vector<MarketSearchResult>& results,
                              const MarketSearchResult& next_result) {
    for (const MarketSearchResult& current : results) {
        if (current.request_symbol == next_result.request_symbol) {
            return;
        }
    }
    results.push_back(next_result);
}

bool isKnownPresetSymbol(const String& request_symbol) {
    for (const MarketPreset& preset : kHotMarketPresets) {
        if (request_symbol == preset.request_symbol) {
            return true;
        }
    }
    return false;
}

bool connectMarketClient(WiFiClient& client) {
    if (client.connect(kShanghaiIndexHost, kShanghaiIndexPort,
                       kHttpConnectTimeoutMs)) {
        return true;
    }
    client.stop();
    for (const IPAddress& ip : kMarketFallbackIps) {
        if (client.connect(ip, kShanghaiIndexPort, kHttpConnectTimeoutMs)) {
            return true;
        }
        client.stop();
    }
    return false;
}
}

MarketQuote MarketService::fetchQuote(const String& request_symbol) const {
    MarketQuote quote;
    quote.request_symbol = request_symbol.isEmpty() ? String("sh000001")
                                                    : request_symbol;

    WiFiClient client;
    client.setTimeout(kHttpTimeoutMs);
    if (!connectMarketClient(client)) {
        quote.error_message = "Connect failed";
        return quote;
    }

    client.print(String("GET /q=") + quote.request_symbol + " HTTP/1.0\r\n");
    client.print(String("Host: ") + kShanghaiIndexHost + "\r\n");
    client.print("User-Agent: M5PaperClock/1.0\r\n");
    client.print("Accept: */*\r\n");
    client.print("Referer: https://gu.qq.com/\r\n");
    client.print("Connection: close\r\n\r\n");

    String status_line = client.readStringUntil('\n');
    status_line.trim();
    quote.status_code = parseHttpStatusCode(status_line);
    if (quote.status_code != 200) {
        quote.error_message =
            quote.status_code > 0 ? "HTTP " + String(quote.status_code)
                                  : "Bad response";
        client.stop();
        return quote;
    }

    while (client.connected()) {
        String header_line = client.readStringUntil('\n');
        if (header_line == "\r" || header_line.length() == 0) {
            break;
        }
    }

    String body;
    uint32_t last_read_ms = millis();
    while (millis() - last_read_ms < kHttpTimeoutMs) {
        while (client.available()) {
            body += static_cast<char>(client.read());
            last_read_ms = millis();
        }
        if (!client.connected() && !client.available()) {
            break;
        }
        delay(5);
    }
    client.stop();

    if (body.isEmpty()) {
        quote.error_message = "Empty response";
        return quote;
    }

    const logic::TencentQuoteSnapshot parsed =
        logic::ParseTencentQuote(std::string(body.c_str()));
    if (!parsed.valid) {
        quote.error_message = "Parse failed";
        return quote;
    }

    quote.symbol = parsed.code.empty() ? String("000001")
                                       : String(parsed.code.c_str());
    const std::string normalized_name =
        logic::NormalizeTencentQuoteName(parsed.code, parsed.name);
    quote.display_name = normalized_name.empty() ? String("上证指数")
                                                 : String(normalized_name.c_str());
    quote.price = String(parsed.price.c_str());
    quote.change = String(parsed.change.c_str());
    quote.change_percent = String(parsed.change_percent.c_str());
    quote.updated_at = String(parsed.timestamp.c_str());
    quote.error_message = "";
    quote.valid = true;
    quote.positive = parsed.positive;
    return quote;
}

std::vector<MarketSearchResult> MarketService::hotMarkets() {
    std::vector<MarketSearchResult> results;
    results.reserve(sizeof(kHotMarketPresets) / sizeof(kHotMarketPresets[0]));

    for (const MarketPreset& preset : kHotMarketPresets) {
        MarketSearchResult result;
        result.request_symbol = preset.request_symbol;
        result.code = preset.code;
        result.display_name = preset.display_name;
        result.kind = preset.kind;
        results.push_back(result);
    }
    return results;
}

std::vector<MarketSearchResult> MarketService::searchMarkets(
    const String& raw_query) const {
    const String query =
        String(logic::NormalizeMarketSearchQuery(std::string(raw_query.c_str())).c_str());
    std::vector<MarketSearchResult> results;

    for (const MarketPreset& preset : kHotMarketPresets) {
        const String preset_symbol = preset.request_symbol;
        const String preset_code = preset.code;
        if (!query.isEmpty() && !startsWith(preset_code, query) &&
            !startsWith(preset_symbol, query)) {
            continue;
        }

        MarketSearchResult result;
        result.request_symbol = preset.request_symbol;
        result.code = preset.code;
        result.display_name = preset.display_name;
        result.kind = preset.kind;
        appendUniqueSearchResult(results, result);
    }

    const std::vector<std::string> inferred_symbols =
        logic::InferTencentQuoteSymbols(std::string(query.c_str()));
    for (const std::string& inferred_symbol : inferred_symbols) {
        const String request_symbol(inferred_symbol.c_str());
        if (isKnownPresetSymbol(request_symbol)) {
            continue;
        }

        const MarketQuote quote = fetchQuote(request_symbol);
        if (!quote.valid) {
            continue;
        }

        MarketSearchResult result;
        result.request_symbol = request_symbol;
        result.code = quote.symbol;
        result.display_name = quote.display_name;
        result.kind = String(
            logic::MarketKindForRequestSymbol(
                inferred_symbol, std::string(quote.symbol.c_str()))
                .c_str());
        appendUniqueSearchResult(results, result);
    }

    return results;
}
