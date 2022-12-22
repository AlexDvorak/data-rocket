#include "periphs.h"
#include "status_light.h"
#include "Config.h"
#include <Adafruit_MMA8451.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MPL3115A2.h>

extern Adafruit_MMA8451 accel;
extern Adafruit_FRAM_I2C fram;
extern Adafruit_MPL3115A2 baro;

bool button_pressed() {
    return !digitalRead(BUTTON);
}

bool button_released() {
    return digitalRead(BUTTON);
}

void wait_for_button_release() {
    while(button_pressed()) {
        led_pulse(8); // T = 4
    }
}

static size_t addr = 0;
bool write_int(int16_t a) {
    if (addr + 2 >= MAX_FRAM_ADDR)
        return false;

    union {
        int16_t num;
        uint8_t bytes[2];
    } i = {
        .num = a
    };

    for(int j=1; j >=0; j--) {
        fram.write(addr, i.bytes[j]);
        addr++;
    }

    return true;
}

bool write_triple(int16_t a, float b, float c) {
    return write_int(a) && write_float(b) && write_float(c);
}

bool write_float(float x) {

    if (addr + 4 >= MAX_FRAM_ADDR)
        return false;

    union {
        float number;
        uint8_t bytes[4];
    } f = {x};

    for(int i = 3; i >= 0; i--) {
        fram.write(addr, f.bytes[i]);
        addr++;
    }

    return true;
}