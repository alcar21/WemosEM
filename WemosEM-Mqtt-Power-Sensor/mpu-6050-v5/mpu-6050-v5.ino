#include <Wire.h>

// https://olivertechnologydevelopment.wordpress.com/2017/08/24/esp8266-sensor-series-gy-521-imu-part-2/

const uint8_t MPU_addr=0x68;  // I2C address of the MPU-6050

const float MPU_GYRO_250_SCALE = 131.0;
const float MPU_GYRO_500_SCALE = 65.5;
const float MPU_GYRO_1000_SCALE = 32.8;
const float MPU_GYRO_2000_SCALE = 16.4;
const float MPU_ACCL_2_SCALE = 16384.0;
const float MPU_ACCL_4_SCALE = 8192.0;
const float MPU_ACCL_8_SCALE = 4096.0;
const float MPU_ACCL_16_SCALE = 2048.0;



struct rawdata {
  int16_t AcX;
  int16_t AcY;
  int16_t AcZ;
  int16_t Tmp;
  int16_t GyX;
  int16_t GyY;
  int16_t GyZ;
  };

struct scaleddata{
  float AcX;
  float AcY;
  float AcZ;
  float Tmp;
  float GyX;
  float GyY;
  float GyZ;
  };

rawdata offsets;

bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
rawdata mpu6050Read(byte addr, bool Debug);
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl);
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl);
scaleddata convertRawToScaled(byte addr, rawdata data_in,bool Debug);
void calibrateMPU6050(byte addr, rawdata &offsets,char up_axis, int num_samples,bool Debug);
rawdata averageSamples(rawdata * samps,int len);

void setup() {
  const uint8_t scl = D6;
  const uint8_t sda = D7;
  Wire.begin(sda, scl);
  Serial.begin(115200);

  mpu6050Begin(MPU_addr);
}

void loop() {
  rawdata next_sample;
  setMPU6050scales(MPU_addr,0b00000000,0b00010000);
  next_sample = mpu6050Read(MPU_addr, true);
  convertRawToScaled(MPU_addr, next_sample,true);

  delay(5000);  // Wait 5 seconds and scan again
}



void mpu6050Begin(byte addr){
// This function initializes the MPU-6050 IMU Sensor
//  It verifys the address is correct and wakes up the
//  MPU.
  if (checkI2c(addr)){
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    delay(30);  // Ensure gyro has enough time to power up

    calibrateMPU6050(addr,offsets,'X', 10,true);
  }
}

bool checkI2c(byte addr){
   // We are using the return value of
  // the Write.endTransmisstion to see if
  // a device did acknowledge to the address.
  Serial.println(" ");
  Wire.beginTransmission(addr);

  if (Wire.endTransmission() == 0)
    {
      Serial.print(" Device Found at 0x");
      Serial.println(addr,HEX);
      return true;
    }
  else
    {
      Serial.print(" No Device Found at 0x");
      Serial.println(addr,HEX);
      return false;
    }
}


rawdata mpu6050Read(byte addr, bool Debug){
  // This function reads the raw 16-bit data values from
  // the MPU-6050

  rawdata values;

  Wire.beginTransmission(addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr,14,true); // request a total of 14 registers
  values.AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  values.AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  values.AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  values.Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  values.GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  values.GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  values.GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)


  values.AcX-=offsets.AcX;
  values.AcY-=offsets.AcY;
  values.AcZ-=offsets.AcZ;
  values.GyX-=offsets.GyX;
  values.GyY-=offsets.GyY;
  values.GyZ-=offsets.GyZ;

  if(Debug){
    Serial.print(" GyX = "); Serial.print(values.GyX);
    Serial.print(" | GyY = "); Serial.print(values.GyY);
    Serial.print(" | GyZ = "); Serial.print(values.GyZ);
    Serial.print(" | Tmp = "); Serial.print(values.Tmp);
    Serial.print(" | AcX = "); Serial.print(values.AcX);
    Serial.print(" | AcY = "); Serial.print(values.AcY);
    Serial.print(" | AcZ = "); Serial.println(values.AcZ);
  }

  return values;
}

void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl){
  Wire.beginTransmission(addr);
  Wire.write(0x1B);  // write to register starting at 0x1B
  Wire.write(Gyro); // Self Tests Off and set Gyro FS to 250
  Wire.write(Accl); // Self Tests Off and set Accl FS to 8g
  Wire.endTransmission(true);
  calibrateMPU6050(addr,offsets,'X', 10,true);
}

void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl){
  Wire.beginTransmission(addr);
  Wire.write(0x1B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(addr,2,true); // request a total of 14 registers
  Gyro = (Wire.read()&(bit(3)|bit(4)))>>3;
  Accl = (Wire.read()&(bit(3)|bit(4)))>>3;
}


scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug){

  scaleddata values;
  float scale_value = 0.0;
  byte Gyro, Accl;

  getMPU6050scales(MPU_addr, Gyro, Accl);

  if(Debug){
    Serial.print("Gyro Full-Scale = ");
    }

  switch (Gyro){
    case 0:
      scale_value = MPU_GYRO_250_SCALE;
      if(Debug){
        Serial.println("±250 °/s");
        }
    break;
    case 1:
      scale_value = MPU_GYRO_500_SCALE;
    if(Debug){
        Serial.println("±500 °/s");
        }
    break;
    case 2:
      scale_value = MPU_GYRO_1000_SCALE;
    if(Debug){
        Serial.println("±1000 °/s");
        }
    break;
    case 3:
      scale_value = MPU_GYRO_2000_SCALE;
    if(Debug){
        Serial.println("±2000 °/s");
        }
    break;
    default:
    break;
  }

  values.GyX = (float) data_in.GyX / scale_value;
  values.GyY = (float) data_in.GyY / scale_value;
  values.GyZ = (float) data_in.GyZ / scale_value;

  scale_value = 0.0;
  if(Debug){
    Serial.print("Accl Full-Scale = ");
    }
  switch (Accl){
    case 0:
      scale_value = MPU_ACCL_2_SCALE;
      if(Debug){
        Serial.println("±2 g");
        }
    break;
    case 1:
      scale_value = MPU_ACCL_4_SCALE;
      if(Debug){
        Serial.println("±4 g");
        }
    break;
    case 2:
      scale_value = MPU_ACCL_8_SCALE;
      if(Debug){
        Serial.println("±8 g");
        }
    break;
    case 3:
      scale_value = MPU_ACCL_16_SCALE;
      if(Debug){
        Serial.println("±16 g");
        }
    break;
    default:
    break;
  }
  values.AcX = (float) data_in.AcX / scale_value;
  values.AcY = (float) data_in.AcY / scale_value;
  values.AcZ = (float) data_in.AcZ / scale_value;


  values.Tmp = (float) data_in.Tmp / 340.0 + 36.53;

  if(Debug){
    Serial.print(" GyX = "); Serial.print(values.GyX);
    Serial.print(" °/s| GyY = "); Serial.print(values.GyY);
    Serial.print(" °/s| GyZ = "); Serial.print(values.GyZ);
    Serial.print(" °/s| Tmp = "); Serial.print(values.Tmp);
    Serial.print(" °C| AcX = "); Serial.print(values.AcX);
    Serial.print(" g| AcY = "); Serial.print(values.AcY);
    Serial.print(" g| AcZ = "); Serial.print(values.AcZ);Serial.println(" g");
  }

  return values;
  }


  void calibrateMPU6050(byte addr,rawdata &offsets,char up_axis ,int num_samples, bool Debug){
    // This function reads in the first num_samples and averages them
    //  to determine calibration offsets, which are then used in
    //  when the sensor data is read.

    //  It simply assumes that the up_axis is vertical and that the sensor is not
    //  moving.
    rawdata temp[num_samples];
    int scale_value;
    byte Gyro, Accl;

    for(int i=0; i<num_samples; i++){
      temp[i] = mpu6050Read(addr,false);
      }

    offsets = averageSamples(temp,num_samples);
    getMPU6050scales(MPU_addr, Gyro, Accl);

    switch (Accl){
      case 0:
        scale_value = (int)MPU_ACCL_2_SCALE;
      break;
      case 1:
        scale_value = (int)MPU_ACCL_4_SCALE;
      break;
      case 2:
        scale_value = (int)MPU_ACCL_8_SCALE;
      break;
      case 3:
        scale_value = (int)MPU_ACCL_16_SCALE;
      break;
      default:
      break;
    }


    switch(up_axis){
      case 'X':
        offsets.AcX -= scale_value;
      break;
      case 'Y':
        offsets.AcY -= scale_value;
      break;
      case 'Z':
        offsets.AcZ -= scale_value;
      break;
      default:
      break;
    }
    if(Debug){
      Serial.print(" Offsets:  GyX = "); Serial.print(offsets.GyX);
      Serial.print(" | GyY = "); Serial.print(offsets.GyY);
      Serial.print(" | GyZ = "); Serial.print(offsets.GyZ);
      Serial.print(" | AcX = "); Serial.print(offsets.AcX);
      Serial.print(" | AcY = "); Serial.print(offsets.AcY);
      Serial.print(" | AcZ = "); Serial.println(offsets.AcZ);
      }
  }

  rawdata averageSamples(rawdata * samps,int len){
     rawdata out_data;
     scaleddata temp;

     temp.GyX = 0.0;
     temp.GyY = 0.0;
     temp.GyZ = 0.0;
     temp.AcX = 0.0;
     temp.AcY = 0.0;
     temp.AcZ = 0.0;

     for(int i = 0; i < len; i++){
     temp.GyX += (float)samps[i].GyX;
     temp.GyY += (float)samps[i].GyY;
     temp.GyZ += (float)samps[i].GyZ;
     temp.AcX += (float)samps[i].AcX;
     temp.AcY += (float)samps[i].AcY;
     temp.AcZ += (float)samps[i].AcZ;
      }

     out_data.GyX = (int16_t)(temp.GyX/(float)len);
     out_data.GyY = (int16_t)(temp.GyY/(float)len);
     out_data.GyZ = (int16_t)(temp.GyZ/(float)len);
     out_data.AcX = (int16_t)(temp.AcX/(float)len);
     out_data.AcY = (int16_t)(temp.AcY/(float)len);
     out_data.AcZ = (int16_t)(temp.AcZ/(float)len);

     return out_data;

    }
