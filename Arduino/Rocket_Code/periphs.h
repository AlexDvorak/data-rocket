#pragma once
#include <stdint.h>

bool buttonReleased();
bool buttonPressed();
void waitForButtonRelease();
bool writeint(int16_t);
bool writeTriple(int16_t, float, float);
bool writeFloat(float);
