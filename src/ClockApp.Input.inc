// Included by ClockApp.cpp so this code can share its internal helpers while
// the large app class is being split into smaller responsibility groups.
void ClockApp::handleTouch() {
    if (!M5.TP.available()) {
        return;
    }

    M5.TP.update();
    const bool finger_up = M5.TP.isFingerUp();
    const int16_t x = M5.TP.readFingerX(0);
    const int16_t y = M5.TP.readFingerY(0);

    if (!finger_up) {
        touch_down_ = true;
        pressed_button_id_ = buttonIdAt(x, y);
    } else if (touch_down_) {
        const int released_button_id = buttonIdAt(x, y);
        if (pressed_button_id_ >= 0 && released_button_id == pressed_button_id_) {
            handleButtonPress(released_button_id);
        }
        pressed_button_id_ = -1;
        touch_down_ = false;
    }

    M5.TP.flush();
}

void ClockApp::handleHardwareButtons() {
    if (current_page_ == PageId::Clock) {
        if (M5.BtnL.wasReleased()) {
            cycleClockLayout(-1);
            return;
        }
        if (M5.BtnR.wasReleased()) {
            cycleClockLayout(1);
            return;
        }
    }

    if (M5.BtnP.isPressed() && !center_button_long_press_handled_ &&
        M5.BtnP.pressedFor(kCenterButtonLongPressMs)) {
        center_button_long_press_handled_ = true;
        if (current_page_ != PageId::Settings) {
            switchPage(PageId::Settings);
        }
        return;
    }

    if (M5.BtnP.wasReleased()) {
        if (!center_button_long_press_handled_) {
            refreshCurrentPage();
        }
        center_button_long_press_handled_ = false;
    }
}

void ClockApp::handleButtonPress(int button_id) {
    if (current_page_ == PageId::Settings) {
        switch (button_id) {
            case kButtonWifi:
                switchPage(PageId::WifiScan);
                scanWiFi();
                break;
            case kButtonTimezoneMinus:
                settings_.timezone = logic::ClampTimeZone(settings_.timezone - 1);
                store_.saveTimezone(settings_.timezone);
                renderPage(UPDATE_MODE_GL16);
                break;
            case kButtonTimezonePlus:
                settings_.timezone = logic::ClampTimeZone(settings_.timezone + 1);
                store_.saveTimezone(settings_.timezone);
                renderPage(UPDATE_MODE_GL16);
                break;
            case kButtonSyncTime:
                trySyncTime(true);
                break;
            case kButtonEnterClock:
                switchPage(PageId::Clock);
                break;
        }
        return;
    }

    if (current_page_ == PageId::WifiScan) {
        if (button_id == kButtonBack) {
            switchPage(PageId::Settings);
            return;
        }
        if (button_id == kButtonRescan) {
            scanWiFi();
            return;
        }
        if (button_id == kButtonPrevPage && wifi_page_index_ > 0) {
            --wifi_page_index_;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonNextPage &&
            wifi_page_index_ + 1 <
                logic::PageCount(static_cast<int>(wifi_networks_.size()),
                                 kWifiPageSize)) {
            ++wifi_page_index_;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id >= kButtonNetworkBase &&
            button_id < kButtonNetworkBase + kWifiPageSize) {
            const int index = logic::PageStart(wifi_page_index_, kWifiPageSize) +
                              (button_id - kButtonNetworkBase);
            if (index >= 0 && index < static_cast<int>(wifi_networks_.size())) {
                selected_ssid_ = wifi_networks_[index].ssid;
                password_input_.clear();
                password_visible_ = false;
                status_message_ = "Ready to connect";
                status_error_ = false;
                switchPage(PageId::Password);
            }
        }
        return;
    }

    if (current_page_ == PageId::Password) {
        if (button_id == kButtonBack) {
            switchPage(PageId::WifiScan);
            return;
        }
        if (button_id == kButtonShift) {
            keyboard_upper_ = !keyboard_upper_;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonKeyboardMode) {
            keyboard_symbols_ = !keyboard_symbols_;
            keyboard_upper_ = false;
            renderPage(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonBackspace) {
            if (!password_input_.isEmpty()) {
                password_input_.remove(password_input_.length() - 1);
                updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            }
            return;
        }
        if (button_id == kButtonSpace) {
            appendPasswordChar(" ");
            return;
        }
        if (button_id == kButtonClear) {
            password_input_.clear();
            updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            return;
        }
        if (button_id == kButtonPasswordVisibility) {
            password_visible_ = !password_visible_;
            updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            rebuildPasswordButtons();
            for (const Button& button : buttons_) {
                if (button.id == kButtonPasswordVisibility) {
                    updateButtonCanvas(button, UPDATE_MODE_GL16);
                    break;
                }
            }
            return;
        }
        if (button_id == kButtonConnect) {
            connectSelectedNetwork();
            return;
        }
        if (button_id >= kButtonKeyboardBase &&
            button_id < kButtonKeyboardBase + 26) {
            appendPasswordChar(keyboardCharacterLabel(
                static_cast<size_t>(button_id - kButtonKeyboardBase)));
        }
        return;
    }
}

int ClockApp::buttonIdAt(int16_t x, int16_t y) const {
    for (const Button& button : buttons_) {
        if (button.visible && button.enabled && button.bounds.contains(x, y)) {
            return button.id;
        }
    }
    return -1;
}

