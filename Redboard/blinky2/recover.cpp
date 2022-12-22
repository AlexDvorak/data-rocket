#include <Arduino.h>
#include "recover.h"
#include "periph.h"

static auto fram = Adafruit_FRAM_I2C();
static size_t addr_index = 0;

bool fram_init() {
    return fram.begin();
}

bool write_float(float f) {
    union {
        float num,
        uint8_t arr[4],
    } s;
    s.num = f;

    for (int i = 3; i >= 0; i--) {
        fram.write(addr_index, s.arr[i]);
        addr_index++;
    }
}

void recover_data() {
    while (Serial.read() != -1)
        ;

    led_set(false);

    for (size_t addr = 0; addr < MAX_FRAM_ADDR; addr++) {
        if (addr % 64 == 0)
            Serial.println();

        uint8_t value = fram.read(addr);
        Serial.print(value, HEX);
        Serial.print(" ");
    }
}
