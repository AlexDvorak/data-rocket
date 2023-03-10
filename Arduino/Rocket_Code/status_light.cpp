#include "status_light.h"
#include "Config.h"

void led_set_analog(uint8_t b) {
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