#include <Wire.h>

// https://olivertechnologydevelopment.wordpress.com/2017/08/24/esp8266-sensor-series-gy-521-imu-part-2/

const uint8_t MPU_addr=0x68; // I2C address of the MPU-6050

struct STREG {
uint8_t reg_13;
uint8_t reg_14;
uint8_t reg_15;
uint8_t reg_16;
} stRegs;

void readSelfTestRegisters(byte addr, STREG &RegsIn,bool Debug);
void check_I2c(byte addr);

void setup() {
  const uint8_t scl = D6;
  const uint8_t sda = D7;
  Wire.begin(sda, scl);
Serial.begin(115200);

Wire.beginTransmission(MPU_addr);
Wire.write(0x6B); // PWR_MGMT_1 register
Wire.write(0); // set to zero (wakes up the MPU-6050)
Wire.endTransmission(true);
}

void loop() {

// Read Factory Trim Settings
readSelfTestRegisters(MPU_addr, stRegs,true);
delay(5000); // Wait 5 seconds and scan again
}

void check_I2c(byte addr){
// We are using the return value of
// the Write.endTransmisstion to see if
// a device did acknowledge to the address.
byte error;
Wire.beginTransmission(addr);
error = Wire.endTransmission();
Serial.println(" ");
if (error == 0)
{
Serial.print(" Device Found at 0x");
Serial.println(addr,HEX);
}
else
{
Serial.print(" No Device Found at 0x");
Serial.println(addr,HEX);
}
}

void readSelfTestRegisters(byte addr, STREG &RegsIn,bool Debug){
// This function reads the self test registers and stores them in RegsIn
// If Debug is true then it sends the contents of the registers to the serial port

Wire.beginTransmission(addr);
Wire.write(0x0D); // starting with register 0x0D (ACCEL_XOUT_H)
Wire.endTransmission(false);
Wire.requestFrom(addr,4,true); // request a total of 4 registers
RegsIn.reg_13=Wire.read();
RegsIn.reg_14=Wire.read();
RegsIn.reg_15=Wire.read();
RegsIn.reg_16=Wire.read();

if(Debug){
Serial.print("Register 13 = "); Serial.print(RegsIn.reg_13,BIN);
Serial.print(" | Register 14 = "); Serial.print(RegsIn.reg_14,BIN);
Serial.print(" | Register 15 = "); Serial.print(RegsIn.reg_15,BIN);
Serial.print(" | Register 16 = "); Serial.println(RegsIn.reg_16,BIN);
}
}

struct TESTVals{
uint8_t XA_TEST;
uint8_t YA_TEST;
uint8_t ZA_TEST;
uint8_t XG_TEST;
uint8_t YG_TEST;
uint8_t ZG_TEST;
} TEST;

struct FTVals {
float Xg;
float Yg;
float Zg;
float Xa;
float Ya;
float Za;
} FT;

void assembleTESTValues(STREG Regs,TESTVals &Tests, bool Debug){
// This function converts the self test registers to the appropriate
// TEST values which are then later used to calculate the FT Values.

Tests.XG_TEST = Regs.reg_13&0b00011111;
Tests.YG_TEST = Regs.reg_14&0b00011111;
Tests.ZG_TEST = Regs.reg_15&0b00011111;
Tests.XA_TEST = ((Regs.reg_13&0b11100000)>>3)|((Regs.reg_16>>4)&3);
Tests.YA_TEST = ((Regs.reg_14&0b11100000)>>3)|((Regs.reg_16>>2)&3);
Tests.ZA_TEST = ((Regs.reg_15&0b11100000)>>3)|((Regs.reg_16)&3);

if(Debug){
Serial.print("XG_TEST = "); Serial.print(Tests.XG_TEST,BIN);
Serial.print(" | YG_TEST = "); Serial.print(Tests.YG_TEST,BIN);
Serial.print(" | ZG_TEST = "); Serial.print(Tests.ZG_TEST,BIN);
Serial.print(" | XA_TEST = "); Serial.print(Tests.XA_TEST,BIN);
Serial.print(" | YA_TEST = "); Serial.print(Tests.YA_TEST,BIN);
Serial.print(" | ZA_TEST = "); Serial.println(Tests.ZA_TEST,BIN);
}
}

void generateFTValues(TESTVals Tests,FTVals &ftVal,bool Debug){
// This function generates the FT Values from the TEST values

ftVal.Xg = FT_gyro_calc(Tests.XG_TEST);
ftVal.Yg = -FT_gyro_calc(Tests.YG_TEST); // The Y axis gyro setting is negative
ftVal.Zg = FT_gyro_calc(Tests.ZG_TEST);
ftVal.Xa = FT_accel_calc(Tests.XA_TEST);
ftVal.Ya = FT_accel_calc(Tests.YA_TEST);
ftVal.Za = FT_accel_calc(Tests.ZA_TEST);

if(Debug){
Serial.print(" FT_XG = "); Serial.print(ftVal.Xg);
Serial.print(" | FT_YG = "); Serial.print(ftVal.Yg);
Serial.print(" | FT_ZG = "); Serial.print(ftVal.Zg);
Serial.print(" | FT_XA = "); Serial.print(ftVal.Xa);
Serial.print(" | FT_YA = "); Serial.print(ftVal.Ya);
Serial.print(" | FT_ZA = "); Serial.println(ftVal.Za);
}
}

float FT_accel_calc(uint8_t TEST){
// This is the Acceleration FT calculation
float value = 0.0;
if (TEST !=0){
value = 4096*0.34*131*pow((0.92/0.34),(((float)TEST-1)/(pow(2,5)-2)));
}
return value;
}

float FT_gyro_calc(uint8_t TEST){
// This is the gyro FT calculation
float value = 0.0;
if (TEST !=0){
value = 25*131*pow(1.046,(float)TEST-1);
}
return value;
}

void evalSelfTest(FTVals STR,FTVals FT,bool Debug){
// This function evaluates and reports the reults of
// the self test
FTVals percs;

percs.Xg = change(STR.Xg,FT.Xg);
percs.Yg = change(STR.Yg,FT.Yg);
percs.Zg = change(STR.Zg,FT.Zg);
percs.Xa = change(STR.Xa,FT.Xa);
percs.Ya = change(STR.Ya,FT.Ya);
percs.Za = change(STR.Za,FT.Za);

bool passed = true;

if(abs(percs.Xg) > 14.0){
Serial.println(" X axis Gyro Failed Self Test!");
passed = false;
};
if(abs(percs.Yg) > 14.0){
Serial.println(" Y axis Gyro Failed Self Test!");
passed = false;
};
if(abs(percs.Zg) > 14.0){
Serial.println(" Z axis Gyro Failed Self Test!");
passed = false;
};
if(abs(percs.Xa) > 14.0){
Serial.println(" X axis Accelerometer Failed Self Test!");
passed = false;
};
if(abs(percs.Yg) > 14.0){
Serial.println(" Y axis Accelerometer Failed Self Test!");
passed = false;
};
if(abs(percs.Zg) > 14.0){
Serial.println(" Z axis Accelerometer Failed Self Test!");
passed = false;
};

if(passed){
Serial.println(" ALL AXIS PASSED SELF TEST!");
};
if(Debug){
Serial.print(" perc_Xg = "); Serial.print(percs.Xg);
Serial.print("% | perc_Yg = "); Serial.print(percs.Yg);
Serial.print("% | perc_Zg = "); Serial.print(percs.Zg);
Serial.print("% | perc_Xa = "); Serial.print(percs.Xa);
Serial.print("% | perc_Ya = "); Serial.print(percs.Ya);
Serial.print("% | perc_Za = "); Serial.print(percs.Za); Serial.println("%");
}
}

float change(float STR,float FT){
// This is a simple calculation to determine the
// percent change.
float perc;
perc = 100.0*(STR-FT)/FT;
return perc;
}
