
/*
    MPU6050 Triple Axis Gyroscope & Accelerometer. Temperature Example.
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
#include <MPU6050.h>

MPU6050 mpu;
int SCL_PIN=D6;
int SDA_PIN=D7;


void setup() 
{
  WiFi.forceSleepBegin();// turn off ESP8266 RF
  delay(1);
  Serial.begin(115200);

  Serial.println("Initialize MPU6050");

  while(!mpu.beginSoftwareI2C(SCL_PIN,SDA_PIN,MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G,MPU6050_ADDRESS))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
}

void loop()
{
  float temp = mpu.readTemperature();

  Serial.print(" Temp = ");
  Serial.print(temp);
  Serial.println(" *C");
  
  delay(500);
}
