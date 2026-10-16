#!/usr/bin/env python3
"""Exercise the production GPIO0 button state machine with simulated GPIO/time."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'firmware/src'


def main():
    with tempfile.TemporaryDirectory(prefix='gk01-button-') as directory:
        tmp = Path(directory)
        (tmp / 'core').mkdir()
        (tmp / 'Arduino.h').write_text('''#pragma once
#include <cstdint>
#define LOW 0
#define HIGH 1
#define INPUT_PULLUP 2
#define F(x) x
uint32_t millis();
void pinMode(uint8_t, uint8_t);
int digitalRead(uint8_t);
void digitalWrite(uint8_t, int);
''')
        (tmp / 'core/live_log.h').write_text('''#pragma once
struct Logger { void println(const char*) {} };
extern Logger appLog;
''')
        (tmp / 'test.cpp').write_text(r'''
#include <cassert>
#include <cstdio>
#include "core/pins.h"
#include "hw/button_manager.h"
#include "hw/button_manager.cpp"
Logger appLog;
uint32_t tick = 0;
int gpio[17];
int shorts = 0, resets = 0, modes = 0;
uint32_t millis() { return tick; }
void pinMode(uint8_t pin, uint8_t mode) {
    // The button must remain an input and must never reconfigure the LED pin.
    assert(pin == PIN_BUTTON && pin == 0 && mode == INPUT_PULLUP);
    ++modes;
}
int digitalRead(uint8_t pin) { return gpio[pin]; }
void digitalWrite(uint8_t pin, int value) {
    assert(pin != PIN_BUTTON);
    gpio[pin] = value;
}
void shortPress() { ++shorts; }
void reset() { assert(gpio[PIN_BUTTON] == HIGH); ++resets; }
void advance(uint32_t duration) {
    for (uint32_t i = 0; i < duration; i += 20) { tick += 20; button.loop(); }
}
void init(bool yellow = false) {
    for (auto& level : gpio) level = HIGH;
    gpio[PIN_LED_YELLOW] = yellow ? LOW : HIGH;
    shorts = resets = 0;
    button.begin(shortPress, reset);
    advance(20);
}
void press() { gpio[PIN_BUTTON] = LOW; advance(20); }
void release() { gpio[PIN_BUTTON] = HIGH; advance(20); }
int main() {
    init(); press(); advance(20); release(); assert(shorts == 0 && resets == 0);
    press(); advance(100); release(); assert(shorts == 1 && resets == 0);
    advance(100); assert(shorts == 1);
    for (bool yellow : {false, true}) {
        init(yellow); press(); advance(3000); release();
        assert(shorts == 0 && resets == 0);
        assert(gpio[PIN_LED_YELLOW] == (yellow ? LOW : HIGH));
        press(); advance(6000); assert(resets == 0 && shorts == 0);
        advance(10000); assert(resets == 0); // No reboot while IO0 is grounded.
        release(); assert(resets == 1 && shorts == 0);
        assert(gpio[PIN_LED_YELLOW] == (yellow ? LOW : HIGH));
        advance(100); assert(resets == 1);
        press(); advance(100); release(); assert(shorts == 1);
    }
    tick = UINT32_MAX - 1000; init(); press(); advance(6000);
    assert(resets == 0); release(); assert(resets == 1);
    assert(modes == 4);
    puts("PASS GPIO0 button: short press, noise, medium hold, reset only after release, no repeat, LED restore, millis wrap");
}
'''.replace('#include <cstdio>', '#include <cstdio>\n#include <initializer_list>'))
        binary = tmp / 'button_test'
        subprocess.run([os.environ.get('CXX', 'c++'), '-std=c++17', '-Wall', '-Wextra',
                        f'-I{tmp}', f'-I{SRC}', str(tmp / 'test.cpp'), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    main()
