#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MPL3115A2.h>
#include "Config.h"
#include "status_light.h"
#include "periphs.h"

auto accel = Adafruit_MMA8451();
auto fram = Adafruit_FRAM_I2C();
auto baro = Adafruit_MPL3115A2();

static enum state rocket_state = IDLE;

void setup() {
    pinMode(BUTTON, INPUT_PULLUP);

    delay(2000);
    Serial.begin(250000);
    led_set(true);

    while(!(fram.begin() && accel.begin() && baro.begin())) {
        delay(5000);
        Serial.print("FRAM:\t");
        Serial.println(fram.begin());
        Serial.print("ACCEL:\t");
        Serial.println(accel.begin());
        Serial.print("BARO:\t");
        Serial.println(baro.begin());
    }

    accel.setRange(ACCEL_RANGE);
    accel.setDataRate(MMA8451_DATARATE_50_HZ);

    if(DEBUG)
        Serial.println("good startup");
}

void loop() {
    switch (rocket_state) {
        break; case IDLE:
            led_pulse(2);
            if (button_pressed())
                rocket_state = COUNTDOWN;
            else if (Serial)
                rocket_state = RECOVER_DATA;

        break; case COUNTDOWN:
            if (!init_launch_seq())
                break;
            rocket_state = countdown() == GO
                ? COLLECT_DATA
                : IDLE;

        break; case RECOVER_DATA:
            offload_data();
            rocket_state = IDLE;

        break; case COLLECT_DATA:
            rocket_state = collect(SAMPLE_PERIOD)
                ? COLLECT_DATA
                : IDLE;
    }
}

bool released_before(unsigned long timeout) {
    auto const start = millis();
    while (millis() - start < timeout) {
        if (button_released()) return true;
    }
    return false;
}

bool init_launch_seq(void) {
    led_set(false);

    if (released_before(3000)) return false;

    led_set(true);

    if (released_before(2000)) return true;
    else
        wait_for_button_release();
        return false;
}

Directive countdown(void) {
    for (int i = 0; i < countdown_time; i++) {
        blink(1000);
        if (button_pressed()) {
            wait_for_button_release();
            return ABORT;
        }
    }

    for (int i = 0; i < countdown_time; i++) {
        blink(200);
        if (button_pressed()) {
            wait_for_button_release();
            return ABORT;
        }
    }

    return GO;
}

bool collect(unsigned long period) {
    auto start_time = millis();

    bool const s = collect_data();

    auto elapsed = millis() - start_time;
    if (elapsed < period) {
        auto remaining = 50 - elapsed;
        if (remaining > 0) {
            delay(remaining);
        }
    }
    return s;
}

bool collect_data(void) {
    accel.read();
    int16_t const ax = accel.x;
    int16_t const ay = accel.y;
    int16_t const az = accel.z;

    return write_rocketlog({
        .accel = (ax*ax) + (ay*ay) + (az*az),
        .temp = baro.getTemperature(),
        .press = baro.getPressure(),
    });
}

void offload_data(void) {
    while(Serial.available() > 0)
        Serial.read();

    led_set(false);

    for (size_t a = 0; a < MAX_FRAM_ADDR; a++) {
        if(a % 64 == 0) Serial.println();

        uint8_t value = fram.read(a);
        Serial.print(value, HEX);
        Serial.print(" ");
    }

    Serial.println();
}
