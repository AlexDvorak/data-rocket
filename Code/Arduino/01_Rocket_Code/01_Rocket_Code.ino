#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_FRAM_I2C.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_MPL3115A2_Modified.h>

#define DEBUG false

#define BUTTON 8
#define LED 9

#if DEBUG
  #define COUNTDOWN_MINUTES 0
#else
  #define COUNTDOWN_MINUTES 2
#endif
 
#define MAX_ADDR 32768
#define PRESSURE_SEA_LEVEL 101325.0

Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
Adafruit_MMA8451 accel = Adafruit_MMA8451();
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

uint16_t addr = 0;

uint8_t brightness = 0;
bool brightness_inc = true;

const uint8_t countdown_time = 50 * COUNTDOWN_MINUTES;

void setup() {
  delay(2000);
  
  Serial.begin(250000);
  
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  while(!(fram.begin() && accel.begin() && baro.begin())) {
    delay(5000);
    Serial.print("FRAM:\t");
    Serial.println(fram.begin());
    Serial.print("ACCEL:\t");
    Serial.println(accel.begin());
    Serial.print("BARO:\t");
    Serial.println(baro.begin());
  }
  
  accel.setRange(MMA8451_RANGE_8_G);
  accel.setDataRate(MMA8451_DATARATE_50_HZ);

  if(DEBUG) Serial.println("good startup");
}

void loop() {
  pulse(8);

  if(Serial.available() > 0) {
    recoverData();
  } else if(getButton()) {
    buttonHeld();
    if(DEBUG) Serial.println("default state");
  }
}

void recoverData() {
  while(Serial.available() > 0) {
    Serial.read();
  }
  
  digitalWrite(LED, LOW); // prevent debug logging messing up data recovery
  
  uint8_t value;
  for (uint16_t a = 0; a < MAX_ADDR; a++) {
    if(a % 64 == 0) {
      Serial.println();
    }
    
    value = fram.read8(a);
    Serial.print(value, HEX);
    Serial.print(" ");
  }

  Serial.println();
}

void buttonHeld() {
  setLED(false);
  unsigned long start_time = millis();
  
  while(millis() - start_time < 3000) {
    if(!getButton()) {
      if(DEBUG) Serial.println("button released too early");
      return;
    }
  }
  
  setLED(true);
  bool begin_data = false;
  
  while(millis() - start_time < 5000) {
    if(!getButton()) {
      begin_data = true;
      break;
    }
  }

  if(begin_data) {
    if(DEBUG) Serial.println("countdown");
    takeDataCountdown();
  } else {
    if(DEBUG) Serial.println("button released too late");
    waitForButtonRelease();
  }
}

void takeDataCountdown() {
  uint8_t i = 0;
  for(i = 0; i < countdown_time; i++) {
    setLED(false);
    delay(500);
    setLED(true);
    delay(500);
    
    if(getButton()) {
      waitForButtonRelease();
      return;
    }
  }

  for(i = 0; i < countdown_time; i++) {
    setLED(false);
    delay(100);
    setLED(true);
    delay(100);
    
    if(getButton()) {
      waitForButtonRelease();
      return;
    }
  }

  if(DEBUG) Serial.println("countdown complete");

  while(true) {
    takeData();
  }
}

void takeData() {
  unsigned long start_time = millis();

//  if(DEBUG) Serial.println("starting data frame");
  
  sensors_event_t event; 
  accel.getEvent(&event);
  float accel_x = event.acceleration.x; // ms^-2
  float accel_y = event.acceleration.y; // ms^-2
  float accel_z = event.acceleration.z; // ms^-2
  float accel = sqrt((accel_x * accel_x) + (accel_y * accel_y) + (accel_z * accel_z));
//  if(DEBUG) {
//    Serial.print("done getting accel data at: ");
//    Serial.println(millis() - start_time);
//  }
  
  float temperature = baro.getTemperature(); // deg C
//  if(DEBUG) {
//    Serial.print("done getting temperature at: ");
//    Serial.println(millis() - start_time);
//  }

  float pressure = baro.getPressure(); // Pa
//  if(DEBUG) {
//    Serial.print("done getting pressure at: ");
//    Serial.println(millis() - start_time);
//  }
  
//  float altitude = baro.getAltitude(); // m
//  float altitude = 44330.0 * (1.0 - pow(pressure / PRESSURE_SEA_LEVEL, 0.1903));
//  if(DEBUG) {
//    Serial.print("done getting altitude at: ");
//    Serial.println(millis() - start_time);
//  }

  writeFloat(accel);
  writeFloat(temperature);
  writeFloat(pressure);
//  if(DEBUG) {
//    Serial.print("done writing floats at: ");
//    Serial.println(millis() - start_time);
//  }

  unsigned long elapsed_time = millis() - start_time;
  if(DEBUG) {
    Serial.print("finished data frame after: ");
    Serial.println(elapsed_time);
  }
  
  if(elapsed_time < 50) {
    unsigned long remaining_time = 50 - elapsed_time;
    if(DEBUG) {
      Serial.print("waiting: ");
      Serial.println(remaining_time);
    }
    if(remaining_time > 0) {
      delay(remaining_time);
    }
  }
}

void writeFloat(float x) {
  typedef union
  {
   float number;
   uint8_t bytes[4];
  } float_union_t;
  float_union_t f;

  f.number = x;
  for(uint8_t i = 0; i < 4; i++) {
    fram.write8(addr, f.bytes[3 - i]);
    
    addr++;
    if(addr == MAX_ADDR) {
      if(DEBUG) Serial.println("finished data collection!");
      while(true) {
        pulse(2);
      }
    }
  }
}

void pulse(uint8_t d) {
  if(brightness_inc) {
    brightness++;
    if(brightness == 255) {
      brightness_inc = false;
    }
  } else {
    brightness--;
    if(brightness == 0) {
      brightness_inc = true;
    }
  }
//  if(DEBUG) {
//    Serial.print("brightness: ");
//    Serial.println(brightness);
//  }
  setLEDAnalog(brightness);
  delay(d);
}

void flash(unsigned long d) {
  while(true) {
    setLED(false);
    delay(d);
    setLED(true);
    delay(d);
  }
}

void setLEDAnalog(uint8_t b) {
  analogWrite(LED, b);
}

void setLED(bool on) {
  if(DEBUG) {
    Serial.print("LED: ");
    Serial.println(on);
  }
  digitalWrite(LED, on);
}

bool getButton() {
  return !digitalRead(BUTTON);
}

void waitForButtonRelease() {
  while(getButton()) {
    pulse(8);
  }
}
