#pragma once

#include "Config.h"

bool getButton();
void waitForButtonRelease();
bool writeTriple(float, float, float);
bool writeFloat(float);


bool getButton() {
    return !digitalRead(BUTTON);
}

void waitForButtonRelease() {
    while(getButton()) {
        led_pulse(8); // T = 4
    }
}

bool writeTriple(int16_t a, float b, float c) {
    return writeint(a) && writeFloat(b) && writeFloat(c);
}

bool writeint(int16_t a) {
    if (addr + 2 >= MAX_FRAM_ADDR)
        return false;

    union {
        int16_t num,
        byte bytes[2],
    } i = {a};

    for(int j=1; j >=0; j--) {
        fram.write(addr, i.bytes[j]);
        addr++;
    }
}

static size_t addr = 0;
bool writeFloat(float x) {

    if (addr + 4 >= MAX_FRAM_ADDR)
        return false;

    union {
        float number;
        byte bytes[4];
    } f = {x};

    for(int i = 3; i >= 0; i--) {
        fram.write(addr, f.bytes[i]);
        addr++;
    }

    return true;
}
