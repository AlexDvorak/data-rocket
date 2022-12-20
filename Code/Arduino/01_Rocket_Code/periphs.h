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

bool writeTriple(float a, float b, float c) {
    return writeFloat(a) && writeFloat(b) && writeFloat(c);
}

bool writeFloat(float x) {
    static size_t addr = 0;

    union {
        float number;
        byte bytes[4];
    } f = {x};

    for(int i = 3; i >= 0; i-- && addr++) {
        fram.write(addr, f.bytes[i]);
        if(addr == MAX_FRAM_ADDR) break;
    }

    return true;
}
