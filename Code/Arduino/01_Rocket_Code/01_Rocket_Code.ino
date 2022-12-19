#include "Config.h"
#include "status_light.h"
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MPL3115A2.h>


Adafruit_FRAM_I2C fram {};
Adafruit_MMA8451 accel {};
Adafruit_MPL3115A2 baro {};


void setup() {
    delay(2000);

    Serial.begin(250000);

    led_set(true);
    pinMode(BUTTON, INPUT_PULLUP);

    while(!(fram.begin() && accel.begin() && baro.begin())) {
        delay(5000);
        Serial.print("FRAM:\t");
        Serial.println(fram.begin());
        Serial.print("ACCEL:\t");
        Serial.println(accel.begin());
        Serial.print("BARO:\t");
        Serial.println(baro.begin());
    }

    accel.setRange(MMA8451_RANGE_8_G);
    accel.setDataRate(MMA8451_DATARATE_50_HZ);

    if(DEBUG)
        Serial.println("good startup");
}

enum state {
    IDLE,
    COUNTDOWN,
    COLLECT_DATA,
    RECOVER_DATA,
};
static enum state rocket_state = IDLE;

void loop() {
    switch (rocket_state) {
        break; case IDLE:
            led_pulse(2);
            rocket_state = Serial.available() > 0
                ? RECOVER_DATA
                : getButton()
                    ? COUNTDOWN
                    : IDLE;

        break; case COUNTDOWN:
            rocket_state = init_countdown()
                ? COLLECT_DATA
                : IDLE;

        break; case RECOVER_DATA:
            offload_data(); // blocking
            rocket_state = IDLE;

        break; case COLLECT_DATA:
            rocket_state = take_data()
                ? COLLECT_DATA
                : IDLE;
    }
}

void offload_data() {
    while(Serial.available() > 0) {
        Serial.read();
    }

    // no debug logging (messes up data recovery)

    digitalWrite(LED, LOW);

    byte value;
    for (uint16_t a = 0; a < MAX_FRAM_ADDR; a++) {
        if(a % 64 == 0) {
            Serial.println();
        }

        value = fram.read(a);
        Serial.print(value, HEX);
        Serial.print(" ");
    }

    Serial.println();
}

bool init_collect() {
    led_set(false);

    auto const start_time = millis();

    while(millis() - start_time < 3000) {
        if(!getButton()) {
            if(DEBUG) Serial.println("button released too early");
            return false;
        }
    }

    led_set(true);

    while(millis() - start_time < 5000) {
        if(!getButton()) {
            return true;
        }
    }

    waitForButtonRelease();
    return false;
}

bool init_countdown() {
    for (int i = 0; i < countdown_time; i++) {
        blink(1000 / 2);
        if (getButton()) {
            waitForButtonRelease();
            return false;
        }
    }
    
    for (int i = 0; i < countdown_time; i++) {
        blink(200 / 2);
        if (getButton()) {
            waitForButtonRelease();
            return false;
        }
    }

    return true;
}

bool take_data() { // LED stays on
    unsigned long start_time = millis();

    sensors_event_t event;
    accel.getEvent(&event);
    float accel_x = event.acceleration.x; // ms^-2
    float accel_y = event.acceleration.y; // ms^-2
    float accel_z = event.acceleration.z; // ms^-2
    float accel = sqrt((accel_x * accel_x) + (accel_y * accel_y) + (accel_z * accel_z));

    float temperature = baro.getTemperature(); // deg C

    float pressure = baro.getPressure(); // Pa

    bool const complete = writeFloat(accel)
        && writeFloat(temperature)
        && writeFloat(pressure);

    unsigned long elapsed_time = millis() - start_time;

    if(elapsed_time < 50) {
        unsigned long remaining_time = 50 - elapsed_time;
        if(remaining_time > 0) {
            delay(remaining_time);
        }
    }

    return complete;
}

bool getButton() {
    return !digitalRead(BUTTON);
}

void waitForButtonRelease() {
    while(getButton()) {
        led_pulse(8); // T = 4
    }
}

bool writeFloat(float x) {
    static size_t addr = 0;
    union {
        float number;
        uint8_t bytes[4];
    } f {x};

    for(uint8_t i = 0; i < 4; i++) {
        fram.write(addr, f.bytes[3 - i]);

        addr++;
        if(addr == MAX_FRAM_ADDR) {
            return false;
        }
    }

    return true;
}
