#include <Wire.h>
#include <Adafruit_FRAM_I2C.h>

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

  while(Serial.available() <= 0) {
    delay(10);
  }
  
  Serial.println("Starting!");

  addr = 0;
  do {
    x = fram.read8(addr);
    Serial.print(x, HEX);
    Serial.println();
    fram.write8(addr, x + 0x1);
    addr++;
  } while(x > 0x0 && addr < 0xFF);
}

void loop() {
  delay(1000);
}
