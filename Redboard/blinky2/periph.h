#pragma once

#include <stdint.h>

bool button_pressed();
bool button_released();
bool released_before(unsigned long);
void wait_until_released();

void led_set_analog(uint8_t);
void led_set(bool);
void led_blink(unsigned long);
void led_pulse(unsigned long);
