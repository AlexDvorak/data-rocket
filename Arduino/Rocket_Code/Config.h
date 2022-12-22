#pragma once

#include <Adafruit_MMA8451.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MPL3115A2.h>

uint8_t const BUTTON = 8;
uint8_t const LED = 9;

bool const DEBUG = false;
int const COUNTDOWN_MINUTES = DEBUG ? 0 : 2;
int const countdown_time = 50 * COUNTDOWN_MINUTES;

size_t const MAX_FRAM_ADDR = 32768;
float const PRESSURE_SEA_LEVEL = 101325.0;

auto const ACCEL_RANGE = mma8451_range_t::MMA8451_RANGE_8_G;
int const SAMPLE_RATE = 50; // Hz
int const SAMPLE_PERIOD = 1000 / SAMPLE_RATE; // millisecs


enum Directive {
    ABORT,
    GO,
};

enum state {
    IDLE,
    COUNTDOWN,
    COLLECT_DATA,
    RECOVER_DATA,
};