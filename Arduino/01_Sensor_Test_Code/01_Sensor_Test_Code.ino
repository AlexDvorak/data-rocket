#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_Sensor.h>

Adafruit_MMA8451 accel = Adafruit_MMA8451();
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

void setup() {
  Serial.begin(19200);

  Wire.begin();

  while(!(accel.begin() && baro.begin())) {
    Serial.println("I2C failed.");
    delay(1000);
  }

  accel.setRange(MMA8451_RANGE_2_G);
  accel.setDataRate(MMA8451_DATARATE_100_HZ);
}

void loop() {
  sensors_event_t event; 
  accel.getEvent(&event);
  float accel_x = event.acceleration.x; // ms^-2
  float accel_y = event.acceleration.y; // ms^-2
  float accel_z = event.acceleration.z; // ms^-2

  float altitude = baro.getAltitude(); // m
  float temperature = baro.getTemperature(); // deg C
  float pressure = baro.getPressure(); // Pa

  writeFloat("accel_x", accel_x);
  Serial.println("ms^-2");
  writeFloat("accel_y", accel_y);
  Serial.println("ms^-2");
  writeFloat("accel_z", accel_z);
  Serial.println("ms^-2");
  writeFloat("altitude", altitude);
  Serial.println("m");
  writeFloat("temperature", temperature);
  Serial.println("C");
  writeFloat("pressure", pressure);
  Serial.println("Pa");
  Serial.println();
}

void writeFloat(char title[], float x) {
  Serial.print(title);
  Serial.print("\t: ");
  Serial.print(x, 2);
  Serial.print(" ");
}
