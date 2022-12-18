#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MPL3115A2.h>

bool const DEBUG = false;
int const COUNTDOWN_MINUTES = DEBUG ? 0 : 2;

byte const BUTTON = 8;
byte const LED = 9;

size_t const MAX_ADDR = 32768;
float const PRESSURE_SEA_LEVEL = 101325.0;

Adafruit_FRAM_I2C fram {};
Adafruit_MMA8451 accel {};
Adafruit_MPL3115A2 baro {};

// uint16_t addr = 0;

int brightness = 0;
bool brightness_inc = true;

const int countdown_time = 50 * COUNTDOWN_MINUTES;

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

void loop() {
    pulse(8); // T = 4

    if(Serial.available() > 0) {
        recoverData();
    } else if(getButton()) {
        if(readyCollect()) {
            if(DEBUG)
                Serial.println("countdown");
            takeDataCountdown();
        } else {
            if(DEBUG)
                Serial.println("button released too late");
        }
        if(DEBUG) Serial.println("default state");
    }
}

void recoverData() {
    while(Serial.available() > 0) {
        Serial.read();
    }

    // no debug logging (messes up data recovery)

    digitalWrite(LED, LOW);

    byte value;
    for (uint16_t a = 0; a < MAX_ADDR; a++) {
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

void takeDataCountdown() {
    uint8_t i = 0;
    for(i = 0; i < countdown_time; i++) {
        setLED(false);
        delay(500);
        setLED(true);
        delay(500);

        if(getButton()) {
            waitForButtonRelease();
            return;
        }
    }

    for(i = 0; i < countdown_time; i++) {
        setLED(false);
        delay(100);
        setLED(true);
        delay(100);

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

void writeFloat(float x) {
    static size_t addr = 0;
    union {
        float number;
        uint8_t bytes[4];
    } f {x};

    for(uint8_t i = 0; i < 4; i++) {
        fram.write(addr, f.bytes[3 - i]);

        addr++;
        if(addr == MAX_ADDR) {
            if(DEBUG) Serial.println("finished data collection!");
            while(true) {
                pulse(2); // T = 1
            }
        }
    }
}

void pulse(byte d) {
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
    delay(d);
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
