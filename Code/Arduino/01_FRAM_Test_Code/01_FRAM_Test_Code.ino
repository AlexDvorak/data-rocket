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
    while(true) {
      delay(1000);
    }
  }
  
  Serial.println("Clearing!");

  for(addr = 0; addr < N; addr++) {
    fram.write8(addr, 0);
  }

  while(true) {
    Serial.println();
    Serial.println("Ready!");
    Serial.println();
  
    while(Serial.available() <= 0) {
      delay(10);
    }
    while(Serial.available() > 0) {
      Serial.read();
    }
  
    addr = 0;
    do {
      x = fram.read8(addr);
      Serial.print(x);
      Serial.println();
      fram.write8(addr, x + 1);
      addr++;
    } while(x > 0 && addr < 255);
  }
}

void loop() {
  delay(1000);
}
