// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
String ClockApp::currentIpAddress() const {
    return connectivity_.currentIpAddress();
}

String ClockApp::currentMarketCode() const {
    return MarketCodeFromRequestSymbol(settings_.market_symbol);
}

String ClockApp::currentMarketDisplayName() const {
    if (!settings_.market_name.isEmpty()) {
        return settings_.market_name;
    }
    if (!market_quote_.display_name.isEmpty()) {
        return market_quote_.display_name;
    }
    return currentMarketCode();
}

ClockApp::MarketRenderState& ClockApp::marketRenderStateFor(
    const MarketLayoutItem& item) {
    for (MarketRenderState& state : market_render_states_) {
        if (state.instance_id == item.instance_id) {
            if (state.symbol != item.symbol) {
                state.symbol = item.symbol;
                seedMarketQuoteFromSettings(state, item.symbol);
                state.last_fetch_ms = 0;
                state.render_state = render::DashboardMarketRenderState {};
            }
            return state;
        }
    }

    MarketRenderState state;
    state.instance_id = item.instance_id;
    state.symbol = item.symbol.isEmpty() ? String("sh000001") : item.symbol;
    seedMarketQuoteFromSettings(state, state.symbol);
    market_render_states_.push_back(state);
    return market_render_states_.back();
}

void ClockApp::syncMarketRenderStates() {
    for (const MarketLayoutItem& item : settings_.market_layout) {
        marketRenderStateFor(item);
    }
    for (auto it = market_render_states_.begin();
         it != market_render_states_.end();) {
        bool found = false;
        for (const MarketLayoutItem& item : settings_.market_layout) {
            if (item.instance_id == it->instance_id) {
                found = true;
                break;
            }
        }
        if (found) {
            ++it;
        } else {
            it = market_render_states_.erase(it);
        }
    }
}

void ClockApp::seedMarketQuoteFromSettings(MarketRenderState& state,
                                           const String& symbol,
                                           const String& display_name) {
    state.quote = MarketQuote {};
    state.symbol = symbol.isEmpty() ? String("sh000001") : symbol;
    state.quote.request_symbol = state.symbol;
    state.quote.symbol = MarketCodeFromRequestSymbol(state.symbol);
    if (!display_name.isEmpty()) {
        state.quote.display_name = display_name;
        return;
    }
    const std::string fallback_name =
        logic::KnownMarketDisplayName(std::string(state.quote.symbol.c_str()));
    state.quote.display_name = fallback_name.empty()
                                   ? state.quote.symbol
                                   : String(fallback_name.c_str());
}

void ClockApp::seedMarketQuoteFromSettings() {
    market_quote_ = MarketQuote {};
    market_quote_.request_symbol = settings_.market_symbol.isEmpty()
                                       ? String("sh000001")
                                       : settings_.market_symbol;
    market_quote_.symbol =
        MarketCodeFromRequestSymbol(market_quote_.request_symbol);
    if (!settings_.market_name.isEmpty()) {
        market_quote_.display_name = settings_.market_name;
        return;
    }

    const std::string fallback_name =
        logic::KnownMarketDisplayName(std::string(market_quote_.symbol.c_str()));
    market_quote_.display_name = fallback_name.empty()
                                     ? market_quote_.symbol
                                     : String(fallback_name.c_str());
    syncMarketRenderStates();
    if (!market_render_states_.empty()) {
        MarketRenderState& first = market_render_states_.front();
        seedMarketQuoteFromSettings(first, settings_.market_symbol,
                                    settings_.market_name);
        market_quote_ = first.quote;
    }
}
