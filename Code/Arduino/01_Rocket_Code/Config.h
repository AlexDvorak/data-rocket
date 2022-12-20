#pragma once

#include <Adafruit_MMA8451.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MPL3115A2.h>

byte const BUTTON = 8;
byte const LED = 9;


extern Adafruit_FRAM_I2C fram;
extern Adafruit_MMA8451 accel;
extern Adafruit_MPL3115A2 baro;

bool const DEBUG = false;
int const COUNTDOWN_MINUTES = DEBUG ? 0 : 2;
int const countdown_time = 50 * COUNTDOWN_MINUTES;

size_t const MAX_FRAM_ADDR = 32768;
float const PRESSURE_SEA_LEVEL = 101325.0;
