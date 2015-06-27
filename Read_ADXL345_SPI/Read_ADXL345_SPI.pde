/* 
  ADXL345 over SPI 
  by: Chris Jeppesen
  Kwan Systems
  
  Modified from L3G4200D 3-axis gyro example code
  by: Jim Lindblom
  SparkFun Electronics
  date: 4/18/11
  license: CC-SA 3.0 - Use this code however you'd like, all we ask
  for is attribution. And let us know if you've improved anything!
  
  Circuit:
  11Dof---------------------------Arduino Nano
  GND-----------------------------GND
  VCC-----------------------------3.3V
  SCK-----------------------------D13
  MOSI----------------------------D11
  MISO----------------------------D12
  CS[A]---------------------------D10
  
*/

#include <SPI.h>
/*
  This file contains defines for the L3G4200D registers. For use 
  with L3G4200D_Example.pde.
*/

// pin definitions
const int int2pin = 6;
const int int1pin = 7;
const int chipSelect = 10;

unsigned char whoami=0;

// gyro readings
unsigned char xh, xl, yh,yl, zh,zl,t;

void setup() {
  Serial.begin(9600);
  Serial.println("ADXL345 readout");
  
  // Start the SPI library:
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  
  pinMode(int1pin, INPUT);
  pinMode(int2pin, INPUT);
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);
  delay(100);
  
  setupL3G4200D(2);  // Configure L3G4200 with selectabe full scale range
  delay(100);
  Serial.print("Whoami: ");
  Serial.println(whoami,OCT);
  // 0: 250 dps
  // 1: 500 dps
  // 2: 2000 dps
}

void loop() {
/*
  // Don't read gyro values until the gyro says it's ready
  while(!digitalRead(int2pin))    ;  
  getGyroValues();  // This will update x, y, and z with new values
  
  Serial.print(xh, HEX);
  Serial.print(xl, HEX);
  Serial.print("\t");
  Serial.print(yh, HEX);
  Serial.print(yl, HEX);
  Serial.print("\t");
  Serial.print(zh, HEX);
  Serial.print(zl, HEX);
  Serial.print("\t");
  Serial.print(t, DEC);
  Serial.print("\t");
  Serial.println();
  
//  delay(1000); // may want to stick this in for readability
*/
}

int readRegister(byte address) {
  int toRead;
  
  address |= 0x80;  // This tells the L3G4200D we're reading;
  
  digitalWrite(chipSelect, LOW);
  SPI.transfer(address);
  toRead = SPI.transfer(0x00);
  digitalWrite(chipSelect, HIGH);
  
  return toRead;
}

void writeRegister(byte address, byte data) {
  address &= 0x7F;  // This to tell the L3G4200D we're writing
  
  digitalWrite(chipSelect, LOW);
  SPI.transfer(address);
  SPI.transfer(data);
  digitalWrite(chipSelect, HIGH);
}

int setupL3G4200D(byte fullScale) {
  // Let's first check that we're communicating properly
  // The WHO_AM_I register should read 0xD3
  whoami=readRegister(0);
  if(whoami!=0345)
    return -1;
    
/*
// Enable x, y, z and turn off power down:
  writeRegister(CTRL_REG1, 0b00001111);
  
  // If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
  writeRegister(CTRL_REG2, 0b00000000);
  
  // Configure CTRL_REG3 to generate data ready interrupt on INT2
  // No interrupts used on INT1, if you'd like to configure INT1
  // or INT2 otherwise, consult the datasheet:
  writeRegister(CTRL_REG3, 0b00001000);
  
  // CTRL_REG4 controls the full-scale range, among other things:
  fullScale &= 0x03;
  writeRegister(CTRL_REG4, fullScale<<4);
  
  // CTRL_REG5 controls high-pass filtering of outputs, use it
  // if you'd like:
  writeRegister(CTRL_REG5, 0b00000000);
  */
}

void getGyroValues() {
  /*
  xl = (readRegister(OUT_X_L)&0xFF);
  xh= (readRegister(OUT_X_H)&0xFF)<<8;
  
  yl = (readRegister(OUT_Y_L)&0xFF);
  yh = (readRegister(OUT_Y_H)&0xFF)<<8;
  
  zl = (readRegister(OUT_Z_L)&0xFF);
  zh = (readRegister(OUT_Z_H)&0xFF)<<8;
  
  t=readRegister(OUT_TEMP);
  */
}
