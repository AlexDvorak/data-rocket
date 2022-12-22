#pragma once
#include <stdint.h>

bool getButton();
void waitForButtonRelease();
bool writeint(int16_t);
bool writeTriple(int16_t, float, float);
bool writeFloat(float);
