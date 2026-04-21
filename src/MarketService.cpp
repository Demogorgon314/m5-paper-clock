#include "MarketService.h"

#include <WiFiClient.h>

#include "logic/MarketLogic.h"

namespace {
constexpr const char* kShanghaiIndexHost = "qt.gtimg.cn";
constexpr uint16_t kShanghaiIndexPort = 80;
constexpr const char* kShanghaiIndexPath = "/q=sh000001";
constexpr uint16_t kHttpTimeoutMs = 5000;

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
}

MarketQuote MarketService::fetchShanghaiIndex() const {
    MarketQuote quote;

    WiFiClient client;
    client.setTimeout(kHttpTimeoutMs);
    if (!client.connect(kShanghaiIndexHost, kShanghaiIndexPort)) {
        quote.error_message = "Connect failed";
        return quote;
    }

    client.print(String("GET ") + kShanghaiIndexPath + " HTTP/1.0\r\n");
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

    quote.symbol = "SH000001";
    quote.display_name = "SSE Index";
    quote.price = String(parsed.price.c_str());
    quote.change = String(parsed.change.c_str());
    quote.change_percent = String(parsed.change_percent.c_str());
    quote.updated_at = String(parsed.timestamp.c_str());
    quote.error_message = "";
    quote.valid = true;
    quote.positive = parsed.positive;
    return quote;
}
