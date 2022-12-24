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

bool write_rocketlog(struct rocketlog_t data) {
    addr += sizeof(data);

    if (addr > MAX_FRAM_ADDR)
        return false;

    return sizeof(data) == fram.writeObject(addr, &data);
}
