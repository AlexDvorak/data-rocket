#include <Arduino.h>
#include <Adafruit_FRAM_I2C.h>
#include <stdint.h>
#include "recover.h"
#include "periph.h"
#include "config.h"

static auto fram = Adafruit_FRAM_I2C();
static size_t addr_index = 0;
static size_t const MAX_FRAM_ADDR = 1 << 15;

bool fram_init() {
    return fram.begin();
}

bool dummy_write(float f) {
    (void) f;
    return ++addr_index < MAX_FRAM_ADDR;
}

bool write_float(float f) {
    if (addr_index + 4 > MAX_FRAM_ADDR)
        return false;

    union {
        float num;
        uint8_t arr[4];
    } s;
    s.num = f;

    for (int i = 3; i >= 0; i--) {
        fram.write(addr_index, s.arr[i]);
        addr_index++;
    }

    return true;
}

void recover_data() {
    while (Serial.available() > 0)
        Serial.readStringUntil('\n');

    led_set(false);

    for (size_t addr = 0; addr < MAX_FRAM_ADDR; addr++) {
        if (DEBUG && addr % 64 == 0) {
            Serial.print("Block #");
            Serial.println((addr / 64) + 1);
        }

        // uint8_t value = fram.read(addr);
        // Serial.print(value, HEX);
        // Serial.print(" ");
    }
}
