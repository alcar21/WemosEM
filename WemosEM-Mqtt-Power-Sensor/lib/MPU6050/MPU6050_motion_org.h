/*
    MPU6050 Triple Axis Gyroscope & Accelerometer. Motion detection.
    Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-zyroskop-i-akcelerometr-mpu6050.html
    GIT: https://github.com/jarzebski/Arduino-MPU6050
    Web: http://www.jarzebski.pl
    (c) 2014 by Korneliusz Jarzebski

    ESP8266 Version by Emanuele Buchicchio
    30 october 2016
    https://github.com/emanbuc/ESP8266_MPU6050
*/

#include <Wire.h>
#include <ESP8266WiFi.h>
#include "MPU6050.h"

MPU6050 mpu;

void checkSettings()
{
  Serial.println();

  Serial.print(" * Sleep Mode:                ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Motion Interrupt:     ");
  Serial.println(mpu.getIntMotionEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Zero Motion Interrupt:     ");
  Serial.println(mpu.getIntZeroMotionEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Free Fall Interrupt:       ");
  Serial.println(mpu.getIntFreeFallEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Motion Threshold:          ");
  Serial.println(mpu.getMotionDetectionThreshold());

  Serial.print(" * Motion Duration:           ");
  Serial.println(mpu.getMotionDetectionDuration());

  Serial.print(" * Zero Motion Threshold:     ");
  Serial.println(mpu.getZeroMotionDetectionThreshold());

  Serial.print(" * Zero Motion Duration:      ");
  Serial.println(mpu.getZeroMotionDetectionDuration());

  Serial.print(" * Clock Source:              ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }

  Serial.print(" * Accelerometer:             ");
  switch(mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }

  Serial.print(" * Accelerometer offsets:     ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());

  Serial.print(" * Accelerometer power delay: ");
  switch(mpu.getAccelPowerOnDelay())
  {
    case MPU6050_DELAY_3MS:            Serial.println("3ms"); break;
    case MPU6050_DELAY_2MS:            Serial.println("2ms"); break;
    case MPU6050_DELAY_1MS:            Serial.println("1ms"); break;
    case MPU6050_NO_DELAY:             Serial.println("0ms"); break;
  }

  Serial.println();
}

void setupMPU6050()
{
  // WiFi.forceSleepBegin();// turn off ESP8266 RF
  // delay(1);
  // Serial.begin(115200);
  // delay(500);

  Serial.println("Initialize MPU6050");

  if(!mpu.beginSoftwareI2C(SCL_PIN,SDA_PIN,MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    // delay(500);
  }

  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);

  mpu.setIntFreeFallEnabled(false);
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(false);

  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  mpu.setMotionDetectionThreshold(2);
  mpu.setMotionDetectionDuration(5);

  mpu.setZeroMotionDetectionThreshold(4);
  mpu.setZeroMotionDetectionDuration(2);

  checkSettings();

  pinMode(D4, OUTPUT);
  digitalWrite(D4, LOW);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);
}



void loopMPU6050()
{
  unsigned long now = millis();
  // If enough time since sending last message, or if birth message must be published
  if (now - lastMsgMQTT < message_interval) {
    return;
  }

  lastMsgMQTT = now;

  Vector rawAccel = mpu.readRawAccel();
  Activites act = mpu.readActivites();
  float temp = mpu.readTemperature();


  if (act.isActivity)
  {
    digitalWrite(D4, HIGH);
  } else
  {
    digitalWrite(D4, LOW);
  }

  if (act.isInactivity)
  {
    digitalWrite(D0, HIGH);
  } else
  {
    digitalWrite(D0, LOW);
  }


  Serial.print(act.isActivity);
  Serial.print("-");
  Serial.print(act.isInactivity);

  Serial.print(" X: ");
  Serial.print(act.isPosActivityOnX);
  Serial.print("-");
  Serial.print(act.isNegActivityOnX);
  Serial.print("->");
  Serial.print(rawAccel.XAxis);

  Serial.print(" Y: ");
  Serial.print(act.isPosActivityOnY);
  Serial.print("-");
  Serial.print(act.isNegActivityOnY);
  Serial.print("->");
  Serial.print(rawAccel.YAxis);

  Serial.print(" Z: ");
  Serial.print(act.isPosActivityOnZ);
  Serial.print("-");
  Serial.print(act.isNegActivityOnZ);
  Serial.print("->");
  Serial.print(rawAccel.ZAxis);

  Serial.print(" Temp: ");
  Serial.print(temp);
  Serial.println(" *C");
  // delay(50);
}
