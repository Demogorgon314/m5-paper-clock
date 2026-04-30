// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
String ClockApp::timezoneLabel() const {
    if (settings_.timezone > 0) {
        return "UTC+" + String(settings_.timezone);
    }
    if (settings_.timezone < 0) {
        return "UTC" + String(settings_.timezone);
    }
    return "UTC";
}

String ClockApp::wifiStatusLabel() const {
    if (connectivity_.isConnected()) {
        return "Connected to " + connectivity_.currentSsid();
    }
    if (settings_.ssid.isEmpty()) {
        return "No Wi-Fi configured";
    }
    return "Saved: " + settings_.ssid;
}

String ClockApp::syncStatusLabel() const {
    return status_message_;
}

String ClockApp::formatRtcTimestamp() const {
    rtc_time_t time;
    rtc_date_t date;
    M5.RTC.getTime(&time);
    M5.RTC.getDate(&date);
    return formatDateTime(date, time);
}

uint8_t ClockApp::batteryPercentage() const {
    uint32_t voltage = M5.getBatteryVoltage();
    if (voltage < 3300) {
        voltage = 3300;
    } else if (voltage > 4350) {
        voltage = 4350;
    }

    float battery = static_cast<float>(voltage - 3300) /
                    static_cast<float>(4350 - 3300);
    if (battery <= 0.01f) {
        battery = 0.01f;
    }
    if (battery > 1.0f) {
        battery = 1.0f;
    }
    const uint8_t raw_percentage =
        static_cast<uint8_t>(battery * 100.0f);
    if (stable_battery_percentage_ == 255) {
        stable_battery_percentage_ = raw_percentage;
        battery_candidate_percentage_ = raw_percentage;
        battery_candidate_count_ = 0;
        return stable_battery_percentage_;
    }
    const int delta = static_cast<int>(raw_percentage) -
                      static_cast<int>(stable_battery_percentage_);
    if (delta >= 2 || delta <= -2) {
        stable_battery_percentage_ = raw_percentage;
        battery_candidate_percentage_ = raw_percentage;
        battery_candidate_count_ = 0;
        return stable_battery_percentage_;
    }
    if (delta == 0) {
        battery_candidate_percentage_ = raw_percentage;
        battery_candidate_count_ = 0;
        return stable_battery_percentage_;
    }
    if (battery_candidate_percentage_ != raw_percentage) {
        battery_candidate_percentage_ = raw_percentage;
        battery_candidate_count_ = 1;
        return stable_battery_percentage_;
    }
    if (battery_candidate_count_ < 3) {
        ++battery_candidate_count_;
        return stable_battery_percentage_;
    }
    stable_battery_percentage_ = raw_percentage;
    battery_candidate_count_ = 0;
    return stable_battery_percentage_;
}

String ClockApp::maskedPassword() const {
    return String(logic::MaskPassword(std::string(password_input_.c_str()),
                                      password_visible_).c_str());
}

String ClockApp::keyboardCharacterLabel(size_t index) const {
    const std::vector<String> keys = activeKeyboardLayout();
    if (index >= keys.size()) {
        return "";
    }
    return keys[index];
}

std::vector<String> ClockApp::activeKeyboardLayout() const {
    const std::vector<std::string> keys =
        logic::KeyboardLayout(keyboard_upper_, keyboard_symbols_);
    std::vector<String> output;
    output.reserve(keys.size());
    for (const std::string& key : keys) {
        output.push_back(String(key.c_str()));
    }
    return output;
}

void ClockApp::appendPasswordChar(const String& text) {
    password_input_ += text;
    updatePasswordFieldCanvas(UPDATE_MODE_GL16);
}
