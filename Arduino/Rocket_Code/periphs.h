#pragma once
#include <stdint.h>

bool button_released();
bool button_pressed();
void wait_for_button_release();
bool write_int(int16_t);
bool write_triple(int16_t, float, float);
bool write_float(float);
