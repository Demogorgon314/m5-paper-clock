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
    const logic::ButtonActionDecision decision = logic::DecideButtonAction(
        current_page_, button_id, wifi_page_index_,
        static_cast<int>(wifi_networks_.size()), kWifiPageSize);

    switch (decision.action) {
        case logic::ButtonAction::OpenWifiScan:
            switchPage(PageId::WifiScan);
            scanWiFi();
            return;
        case logic::ButtonAction::DecreaseTimezone:
            settings_.timezone = logic::ClampTimeZone(settings_.timezone - 1);
            store_.saveTimezone(settings_.timezone);
            renderPage(UPDATE_MODE_GL16);
            return;
        case logic::ButtonAction::IncreaseTimezone:
            settings_.timezone = logic::ClampTimeZone(settings_.timezone + 1);
            store_.saveTimezone(settings_.timezone);
            renderPage(UPDATE_MODE_GL16);
            return;
        case logic::ButtonAction::SyncTime:
            trySyncTime(true);
            return;
        case logic::ButtonAction::EnterClock:
            switchPage(PageId::Clock);
            return;
        case logic::ButtonAction::BackToSettings:
            switchPage(PageId::Settings);
            return;
        case logic::ButtonAction::RescanWifi:
            scanWiFi();
            return;
        case logic::ButtonAction::PreviousWifiPage:
            --wifi_page_index_;
            renderPage(UPDATE_MODE_GL16);
            return;
        case logic::ButtonAction::NextWifiPage:
            ++wifi_page_index_;
            renderPage(UPDATE_MODE_GL16);
            return;
        case logic::ButtonAction::SelectWifiNetwork:
            selected_ssid_ = wifi_networks_[decision.index].ssid;
            password_input_.clear();
            password_visible_ = false;
            status_message_ = "Ready to connect";
            status_error_ = false;
            switchPage(PageId::Password);
            return;
        case logic::ButtonAction::BackToWifiScan:
            switchPage(PageId::WifiScan);
            return;
        case logic::ButtonAction::ToggleKeyboardShift:
            keyboard_upper_ = !keyboard_upper_;
            renderPage(UPDATE_MODE_GL16);
            return;
        case logic::ButtonAction::ToggleKeyboardMode:
            keyboard_symbols_ = !keyboard_symbols_;
            keyboard_upper_ = false;
            renderPage(UPDATE_MODE_GL16);
            return;
        case logic::ButtonAction::BackspacePassword:
            if (!password_input_.isEmpty()) {
                password_input_.remove(password_input_.length() - 1);
                updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            }
            return;
        case logic::ButtonAction::AppendPasswordSpace:
            appendPasswordChar(" ");
            return;
        case logic::ButtonAction::ClearPassword:
            password_input_.clear();
            updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            return;
        case logic::ButtonAction::TogglePasswordVisibility:
            password_visible_ = !password_visible_;
            updatePasswordFieldCanvas(UPDATE_MODE_GL16);
            rebuildPasswordButtons();
            for (const UiButton& button : buttons_) {
                if (button.id == kButtonPasswordVisibility) {
                    updateButtonCanvas(button, UPDATE_MODE_GL16);
                    break;
                }
            }
            return;
        case logic::ButtonAction::ConnectSelectedNetwork:
            connectSelectedNetwork();
            return;
        case logic::ButtonAction::AppendKeyboardCharacter:
            appendPasswordChar(
                keyboardCharacterLabel(static_cast<size_t>(decision.index)));
            return;
        case logic::ButtonAction::None:
        default:
            return;
    }
}

int ClockApp::buttonIdAt(int16_t x, int16_t y) const {
    for (const UiButton& button : buttons_) {
        if (button.visible && button.enabled && button.bounds.contains(x, y)) {
            return button.id;
        }
    }
    return -1;
}
