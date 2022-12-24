#include <Adafruit_FRAM_I2C.h>

size_t const N = 1 << 15;

auto fram = Adafruit_FRAM_I2C();

void setup() {
  uint16_t addr = 0;

  Serial.begin(250000);

  if(!fram.begin()) {
    Serial.println("FRAM issue!");
    return;
  }

  Serial.println("clearing...");

  for(addr = 0; addr < N; addr++) {
    fram.write(addr, 0);
  }

  Serial.println("cleared!");
}

void loop() {
  delay(1000);
}
