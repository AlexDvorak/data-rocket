#pragma once
#include "config.h"

void ledSet(bool on) {
  digitalWrite(LED, on);
}
void ledAnalog(uint8_t i) {
  analogWrite(LED, i);
}
void ledPulse(uint8_t d) {
  static bool brightness_inc = true;
  static uint8_t brightness = 0;

  if (brightness_inc) {
    brightness++;
    if (brightness == 255) {
      brightness_inc = false;
    }
  } else {
    brightness--;
    if (brightness == 0) {
      brightness_inc = true;
    }
  }
  ledAnalog(brightness);
  delay(d);
}

bool getButton() {
  return !digitalRead(BUTTON);
}
bool buttonPressed() {
  return getButton();
}
bool buttonReleased() {
  return !getButton();
};
void waitForButtonRelease() {
  while (buttonPressed())
    ledPulse(8);
}