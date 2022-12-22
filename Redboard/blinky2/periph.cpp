#include <Arduino.h>

#include "periph.h"
#include "config.h"

bool button_pressed() {
    return !digitalRead(BUTTON_PIN);
}

bool button_released() {
    return digitalRead(BUTTON_PIN);
}

bool released_before(unsigned long ms) {
    auto const start = millis();
    while (millis() - start < ms) {
        if (button_released()) return true;
    }
    return false;
}

void wait_until_released() {
    while (!button_released())
        led_pulse(8);
}

void led_set_analog(uint8_t b) {
    analogWrite(LED_PIN, b);
}

void led_set(bool on) {
    digitalWrite(LED_PIN, on);
}

void led_blink(unsigned long t) {
    led_set(false);
    delay(t / 2);
    led_set(true);
    delay(t / 2);
}

void led_pulse(unsigned long ms) {
    static bool brightness_inc = true;
    static uint8_t brightness = 0;
    if(brightness_inc) {
        brightness++;
        if(brightness == 255) {
            brightness_inc = false;
        }
    } else {
        brightness--;
        if(brightness == 0) {
            brightness_inc = true;
        }
    }
    led_set_analog(brightness);
    delay(ms);
}