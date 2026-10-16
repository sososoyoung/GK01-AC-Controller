#include "core/live_log.h"
#include "hw/button_manager.h"
#include "core/pins.h"

ButtonManager button;

void ButtonManager::begin(ShortPressCallback onShort, LongPressCallback onLong) {
    onShort_       = onShort;
    onLong_        = onLong;
    btnPressed_     = false;
    resetArmed_     = false;
    feedbackActive_ = false;
    btnPressStart_  = 0;
    lastCheckMs_    = 0;
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    appLog.println(F("[MODULE] button_manager real impl loaded"));
}

void ButtonManager::loop() {
    uint32_t now = millis();
    if ((uint32_t)(now - lastCheckMs_) < BTN_CHECK_INTERVAL_MS) return;
    lastCheckMs_ = now;

    bool btnState = digitalRead(PIN_BUTTON);

    if (btnState == LOW) {
        if (!btnPressed_) {
            btnPressed_     = true;
            resetArmed_     = false;
            btnPressStart_  = millis();
            appLog.println(F("[BTN] Pressed"));
        } else {
            unsigned long elapsed = millis() - btnPressStart_;
            if (elapsed >= BTN_LONG_PRESS_MS && !resetArmed_) {
                resetArmed_ = true;
                appLog.println(F("[BTN] Reset armed; release GPIO0 to reset"));
            }
            if (elapsed >= 2000) {
                if (!feedbackActive_) {
                    yellowWasOn_ = digitalRead(PIN_LED_YELLOW) == LED_LEVEL_ON;
                    feedbackActive_ = true;
                }
                if ((millis() / 200) % 2 == 0) ledOn(PIN_LED_YELLOW);
                else ledOff(PIN_LED_YELLOW);
            }
        }
    } else {
        if (btnPressed_) {
            unsigned long elapsed = millis() - btnPressStart_;
            bool doReset = resetArmed_ || elapsed >= BTN_LONG_PRESS_MS;
            btnPressed_ = false;
            resetArmed_ = false;
            if (feedbackActive_) {
                if (yellowWasOn_) ledOn(PIN_LED_YELLOW); else ledOff(PIN_LED_YELLOW);
                feedbackActive_ = false;
            }
            // GPIO0 must be released before the callback restarts the ESP8266.
            if (doReset) {
                if (onLong_) onLong_();
            } else if (elapsed >= BTN_SHORT_PRESS_MIN_MS &&
                elapsed <= BTN_SHORT_PRESS_MAX_MS) {
                if (onShort_) onShort_();
            } else if (elapsed > BTN_SHORT_PRESS_MAX_MS) {
                appLog.println(F("[BTN] Release ignored (hold too long for shortcut)"));
            }
        }
    }
}
