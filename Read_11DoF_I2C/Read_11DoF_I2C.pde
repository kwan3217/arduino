/*
An Arduino code example for interfacing with the HMC5883

by: Jordan McConnell
 SparkFun Electronics
 created on: 6/30/11
 license: OSHW 1.0, http://freedomdefined.org/OSHW

Analog input 4 I2C SDA
Analog input 5 I2C SCL
*/

#include <Wire.h> //I2C Arduino Library

#define HMC5883_ADDRESS 0x1E //0011110b, I2C 7bit address of HMC5883

#define BMP085_ADDRESS 0x77  // I2C address of BMP085

const unsigned char OSS = 0;  // Oversampling Setting

// Calibration values
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;

// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 

short temperature;
long pressure;

void setup(){
  //Initialize Serial and I2C communications
  Serial.begin(9600);
  Serial.print("Something!");
  Wire.begin();
  bmp085Calibration();
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(HMC5883_ADDRESS); //open communication with HMC5883
  Wire.send(0x02); //select mode register
  Wire.send(0x00); //continuous measurement mode
  Wire.endTransmission();
}

void loop(){
  
  int x,y,z; //triple axis data

  //Tell the HMC5883 where to begin reading data
  Wire.beginTransmission(HMC5883_ADDRESS);
  Wire.send(0x03); //select register 3, X MSB register
  Wire.endTransmission();
  
 
 //Read data from each axis, 2 registers per axis
  Wire.requestFrom(HMC5883_ADDRESS, 6);
  if(6<=Wire.available()){
    x = Wire.receive()<<8; //X msb
    x |= Wire.receive(); //X lsb
    z = Wire.receive()<<8; //Z msb
    z |= Wire.receive(); //Z lsb
    y = Wire.receive()<<8; //Y msb
    y |= Wire.receive(); //Y lsb
  }
  
  
  temperature = bmp085GetTemperature(bmp085ReadUT());
  pressure = bmp085GetPressure(bmp085ReadUP());


  //Print out values of each axis
  Serial.print("bx: ");
  Serial.print(x);
  Serial.print("  by: ");
  Serial.print(y);
  Serial.print("  bz: ");
  Serial.print(z);
  Serial.print("  temp degC: ");
  Serial.print(temperature/10, DEC);
  Serial.print(".");
  Serial.print(temperature%10, DEC);
  Serial.print("  pres Pa: ");
  Serial.print(pressure, DEC);
  Serial.print("  PresAlt ft: ");
  Serial.print(PresAlt(pressure), DEC);
  Serial.println();
  delay(1000);
}

long PresAlt(long presPa) {
  float f_presPa=presPa;
  float f_altm=44330*(1-pow(f_presPa/101325.0,1.0/5.252));
  return (long)(f_altm/0.3048);
}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
  Serial.print("ac1: ");
  Serial.println(ac1,HEX);
  Serial.print("ac2: ");
  Serial.println(ac2,HEX);
  Serial.print("ac3: ");
  Serial.println(ac3,HEX);
  Serial.print("ac4: ");
  Serial.println(ac4,HEX);
  Serial.print("ac5: ");
  Serial.println(ac5,HEX);
  Serial.print("ac6: ");
  Serial.println(ac6,HEX);
  Serial.print("b1: ");
  Serial.println(b1,HEX);
  Serial.print("b2: ");
  Serial.println(b2,HEX);
  Serial.print("mb: ");
  Serial.println(mb,HEX);
  Serial.print("mc: ");
  Serial.println(mc,HEX);
  Serial.print("md: ");
  Serial.println(md,HEX);
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut)
{
  long x1t, x2t;
  short result;
  
  print("x1t: ",x1t = (((long)ut - (long)ac6)*(long)ac5) >> 15);
  print("x2t: ",x2t = ((long)mc << 11)/(x1t + md));
  print("b5:  ",b5 = x1t + x2t);
  print("getT:",result=((b5 + 8)>>4));

  return result;  
}

void print(char* tag, long arg) {
  Serial.print(tag);
  Serial.println(arg,DEC);
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  print("b6: ",b6 = b5 - 4000);
  // Calculate B3
  print("x1a: ",x1 = (b2 * (b6 * b6)>>12)>>11);
  print("x2a: ",x2 = (ac2 * b6)>>11);
  print("x3a: ",x3 = x1 + x2);
  print("b3:  ",b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2);
  
  // Calculate B4
  print("x1b: ",x1 = (ac3 * b6)>>13);
  print("x2b: ",x2 = (b1 * ((b6 * b6)>>12))>>16);
  print("x3b: ",x3 = ((x1 + x2) + 2)>>2);
  print("b4:  ",b4 = (ac4 * (unsigned long)(x3 + 32768))>>15);
  
  print("b7:  ",b7 = ((unsigned long)(up - b3) * (50000>>OSS)));
  if (b7 < 0x80000000)
    print("pa:   ",p = (b7<<1)/b4);
  else
    print("pb:   ",p = (b7/b4)<<1);
    
  print("x1c: ",x1= (p>>8) * (p>>8));
  print("x1d: ",x1 = (x1 * 3038)>>16);
  print("x2:  ",x2 = (-7357 * p)>>16);
  print("pc:  ",p += (x1 + x2 + 3791)>>4);
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available())
    ;
    
  return Wire.receive();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address) {
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.receive();
  lsb = Wire.receive();
  
  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF4);
  Wire.send(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(5);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value
unsigned long bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF4);
  Wire.send(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.send(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.receive();
  lsb = Wire.receive();
  xlsb = Wire.receive();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}


