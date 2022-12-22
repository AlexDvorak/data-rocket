#pragma once

#include <stdint.h>

void led_set_analog(uint8_t);
void led_set(bool);
void blink(unsigned long);
void led_pulse(unsigned long);
void led_flash(unsigned long);