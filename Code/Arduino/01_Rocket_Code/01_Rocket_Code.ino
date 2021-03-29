#include <Wire.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_Sensor.h>

#define MODE_BUTTON 9
#define LED 7

typedef union
{
 float number;
 uint8_t bytes[4];
} float_union_t;

Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
Adafruit_MMA8451 accel = Adafruit_MMA8451();
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

uint16_t addr = 0;
float_union_t f;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(MODE_BUTTON, INPUT_PULLUP);

  if(digitalRead(MODE_BUTTON) == HIGH) { // data recovery mode
    Serial.begin(250000);

    while(true) {
      while(Serial.available() <= 0){
        delay(10);
      }
      Serial.read();
      
      uint8_t value;
      for (uint16_t a = 0x0; a < 32768; a++) {
        if(a % 64 == 0) {
          Serial.println();
        }
        
        value = fram.read8(a);
        Serial.print(value, HEX);
        Serial.print(" ");
      }
    }
  } else { // data collection mode
    if(!(fram.begin() && accel.begin() && baro.begin())) {
      pulse(500);
    }
    
    accel.setRange(MMA8451_RANGE_2_G);
    accel.setDataRate(MMA8451_DATARATE_100_HZ);
    
    for(int i = 0; i < 20; i++) {
      digitalWrite(LED, LOW);
      delay(100);
      digitalWrite(LED, HIGH);
      delay(100);
    }
  }
}

void loop() {
  unsigned long start_time = millis();
  
  sensors_event_t event; 
  accel.getEvent(&event);
  float accel_x = event.acceleration.x; // ms^-2
  float accel_y = event.acceleration.y; // ms^-2
  float accel_z = event.acceleration.z; // ms^-2

  float altitude = baro.getAltitude(); // m
  float temperature = baro.getTemperature(); // deg C
  float pressure = baro.getPressure(); // Pa

  writeFloat(accel_x);
  writeFloat(accel_y);
  writeFloat(accel_z);
  writeFloat(altitude);
  writeFloat(temperature);
  writeFloat(pressure);

  int remaining_time = 50 - (millis() - start_time);
  if(remaining_time > 0) {
    delay(remaining_time);
  }
}

void writeFloat(float x) {
  f.number = x;
  for(int i = 0; i < 4; i++) {
    fram.write8(addr, f.bytes[i]);
    
    addr++;
    if(addr == 32768) {
      pulse(2000);
    }
  }
}

void pulse(int d) {
  while(true) {
    digitalWrite(LED, LOW);
    delay(d);
    digitalWrite(LED, HIGH);
    delay(d);
  }
}
