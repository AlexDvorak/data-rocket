#pragma once
#include <stdint.h>

struct rocketlog_t {
    int16_t accel;
    float temp;
    float press;
};

bool button_released();
bool button_pressed();
void wait_for_button_release();
bool write_rocketlog(struct rocketlog_t);
