#pragma once

byte const BUTTON = 8;
byte const LED = 9;

bool const DEBUG = false;
int const COUNTDOWN_MINUTES = DEBUG ? 0 : 2;
int const countdown_time = 50 * COUNTDOWN_MINUTES;

size_t const MAX_FRAM_ADDR = 32768;
float const PRESSURE_SEA_LEVEL = 101325.0;
