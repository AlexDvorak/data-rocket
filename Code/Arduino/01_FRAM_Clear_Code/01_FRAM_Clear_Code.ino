#include <Wire.h>
#include <Adafruit_FRAM_I2C.h>

#define N 32768

Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();

void setup() {
  uint16_t addr = 0;
  uint8_t x = 0;

  Serial.begin(250000);

  if(!fram.begin()) {
    Serial.println("FRAM issue!");
    return;
  }
  
  Serial.println("Clearing...");

  for(addr = 0; addr < N; addr++) {
    fram.write8(addr, 0);
  }

  Serial.println("Cleared!");
}

void loop() {
  delay(1000);
}
