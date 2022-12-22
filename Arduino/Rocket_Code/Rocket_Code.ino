#include "Config.h"
#include "status_light.h"
#include "periphs.h"

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MPL3115A2.h>

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
            if (getButton())
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

bool init_launch_seq(void) {
    auto const start_time = millis();

    led_set(false);
    while(millis() - start_time < 3000) {
        if(!getButton()) return false;
    }

    led_set(true);
    while(millis() - start_time < 5000) {
        if(!getButton()) return true;
    }

    waitForButtonRelease();
    return false;
}

Directive countdown(void) {
    for (int i = 0; i < countdown_time; i++) {
        blink(1000);
        if (getButton()) {
            waitForButtonRelease();
            return ABORT;
        }
    }

    for (int i = 0; i < countdown_time; i++) {
        blink(200);
        if (getButton()) {
            waitForButtonRelease();
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

    int16_t const accel = (ax*ax) + (ay*ay) + (az*az);

    float const temperature = baro.getTemperature(); // deg C
    float const pressure = baro.getPressure(); // Pa

    return writeTriple(accel, temperature, pressure);
}

void offload_data(void) {
    while(Serial.available() > 0)
        Serial.read();

    digitalWrite(LED, LOW);

    for (size_t a = 0; a < MAX_FRAM_ADDR; a++) {
        if(a % 64 == 0) Serial.println();

        byte value = fram.read(a);
        Serial.print(value, HEX);
        Serial.print(" ");
    }

    Serial.println();
}