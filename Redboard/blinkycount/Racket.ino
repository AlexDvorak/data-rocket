#include "config.h"
#include "periph.h"

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(13, INPUT);
  Serial.begin(9600);
  ledSet(false);
}

auto start_TIME__ = 0;
bool collecting_data = false;
bool data_complete = false;

void loop() {
  if (collecting_data && !takeData()) {
    collecting_data = false;
    data_complete = true;
  } else if (!collecting_data && buttonPressed()) {
    if (countdown()) collecting_data = true;
    else waitForButtonRelease();
  } else if (data_complete) {
    ledPulse(2);
  } else {
    ledPulse(8);
  }
}

bool init_countdown() {
  ledSet(false);
  Serial.println("Attempting to init countdown stage 1");

  auto start_time = millis();

  while (millis() - start_time < 3000) {
    if (buttonReleased()) {
      Serial.println("Failed to init countdown: Button released too early");
      return false;
    }
  }

  ledSet(true);
  // Serial.println("Attempting to init countdown stage 2");

  while (millis() - start_time < 5000) {
    if (buttonReleased()) {
      Serial.println("Succesfully inited countdown");
      return true;
    }
  }

  Serial.println("Failed to init countdown: Button not released early enough");

  return false;
}

bool countdown() {
  if (!init_countdown()) return false;

  Serial.println("countdown stage 1");

  for (int i = 0; i < COUNTDOWN_TIME; i++) {
    ledSet(false);
    delay(500);
    ledSet(true);
    delay(500);
    if (buttonPressed()) {
      waitForButtonRelease();
      return false;
    }
  }

  Serial.println("countdown stage 2");

  for (int i = 0; i < COUNTDOWN_TIME; i++) {
    ledSet(false);
    delay(100);
    ledSet(true);
    delay(100);
    if (buttonPressed()) {
      waitForButtonRelease();
      return false;
    }
  }

  Serial.println("Countdown successful");

  start_TIME__ = millis();
  return true;
}

bool takeData() {
  return millis() - start_TIME__ < 15000;
}