#pragma once
#include "Config.h"

void led_set_analog(byte b) {
    analogWrite(LED, b);
}

void led_set(bool on) {
    if(DEBUG) {
        Serial.print("LED: ");
        Serial.println(on);
    }
    digitalWrite(LED, on);
}

void blink(unsigned long t) {
    led_set(false);
    delay(t);
    led_set(true);
    delay(t);
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
    //  if(DEBUG) {
    //    Serial.print("brightness: ");
    //    Serial.println(brightness);
    //  }
    led_set_analog(brightness);
    delay(ms);
}

void led_flash(unsigned long d) {
    while(true) {
        led_set(false);
        delay(d);
        led_set(true);
        delay(d);
    }
}