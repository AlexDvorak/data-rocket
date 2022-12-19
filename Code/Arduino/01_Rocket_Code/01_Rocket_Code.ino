#include "Config.h"

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

    setLED(true);
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
    bool const btn_held = getButton();
    if (Serial.available() > 0)
        rocket_state = RECOVER_DATA;
    
    switch (rocket_state) {
        break; case IDLE:
            pulse(2);
            if (btn_held)
                rocket_state = COUNTDOWN;

        break; case COUNTDOWN:
            rocket_state = countdown_2() // blocking
                ? COLLECT_DATA
                : IDLE;

        break; case RECOVER_DATA:
            recoverData(); // blocking
            rocket_state = IDLE;

        break; case COLLECT_DATA:
            rocket_state = takeData2()
                ? COLLECT_DATA
                : IDLE;
    }
}

void recoverData() {
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

bool readyCollect() {
    setLED(false);

    auto const start_time = millis();

    while(millis() - start_time < 3000) {
        if(!getButton()) {
            if(DEBUG) Serial.println("button released too early");
            return false;
        }
    }

    setLED(true);

    while(millis() - start_time < 5000) {
        if(!getButton()) {
            return true;
        }
    }

    waitForButtonRelease();
    return false;
}

void blink(unsigned long t) {
    setLED(false);
    delay(t);
    setLED(true);
    delay(t);
}

bool countdown_2() {
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

void takeDataCountdown() {
    uint8_t i = 0;
    for(i = 0; i < countdown_time; i++) {
        blink(1000 / 2);

        if(getButton()) {
            waitForButtonRelease();
            return;
        }
    }

    for(i = 0; i < countdown_time; i++) {
        blink(200 / 2);

        if(getButton()) {
            waitForButtonRelease();
            return;
        }
    }

    if(DEBUG) Serial.println("countdown complete");

    while(true) {
        takeData();
    }
}

bool takeData2() { // LED stays on
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

void takeData() { // LED stays on
    unsigned long start_time = millis();

    // if(DEBUG) Serial.println("starting data frame");

    sensors_event_t event;
    accel.getEvent(&event);
    float accel_x = event.acceleration.x; // ms^-2
    float accel_y = event.acceleration.y; // ms^-2
    float accel_z = event.acceleration.z; // ms^-2
    float accel = sqrt((accel_x * accel_x) + (accel_y * accel_y) + (accel_z * accel_z));
    // if(DEBUG) {
    //     Serial.print("done getting accel data at: ");
    //     Serial.println(millis() - start_time);
    // }

    float temperature = baro.getTemperature(); // deg C
    // if(DEBUG) {
    //     Serial.print("done getting temperature at: ");
    //     Serial.println(millis() - start_time);
    // }

    float pressure = baro.getPressure(); // Pa
    // if(DEBUG) {
    //     Serial.print("done getting pressure at: ");
    //     Serial.println(millis() - start_time);
    // }

    // float altitude = baro.getAltitude(); // m
    // float altitude = 44330.0 * (1.0 - pow(pressure / PRESSURE_SEA_LEVEL, 0.1903));
    // if(DEBUG) {
    //     Serial.print("done getting altitude at: ");
    //     Serial.println(millis() - start_time);
    // }

    writeFloat(accel);
    writeFloat(temperature);
    writeFloat(pressure);
    // if(DEBUG) {
    //     Serial.print("done writing floats at: ");
    //     Serial.println(millis() - start_time);
    // }

    unsigned long elapsed_time = millis() - start_time;
    if(DEBUG) {
        Serial.print("finished data frame after: ");
        Serial.println(elapsed_time);
    }

    if(elapsed_time < 50) {
        unsigned long remaining_time = 50 - elapsed_time;
        if(DEBUG) {
            Serial.print("waiting: ");
            Serial.println(remaining_time);
        }
        if(remaining_time > 0) {
            delay(remaining_time);
        }
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

void pulse(unsigned long ms) {
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
    setLEDAnalog(brightness);
    delay(ms);
}

void flash(unsigned long d) {
    while(true) {
        setLED(false);
        delay(d);
        setLED(true);
        delay(d);
    }
}

void setLEDAnalog(byte b) {
    analogWrite(LED, b);
}

void setLED(bool on) {
    if(DEBUG) {
        Serial.print("LED: ");
        Serial.println(on);
    }
    digitalWrite(LED, on);
}

bool getButton() {
    return !digitalRead(BUTTON);
}

void waitForButtonRelease() {
    while(getButton()) {
        pulse(8); // T = 4
    }
}
