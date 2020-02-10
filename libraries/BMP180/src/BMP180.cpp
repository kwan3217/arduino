#include "BMP180.h"

#undef BMP180_DEBUG 

BMP180::BMP180(TwoWire &Lport):port(Lport),OSS(3) {}

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
bool BMP180::begin() {
  port.beginTransmission(ADDRESS);
  port.write(0xE0);
  port.write(0x6B);
  port.endTransmission();
  delay(25);
  return readCalibration();
}

bool BMP180::readCalibration() {
  ac1 = read_int16(0xAA);
  ac2 = read_int16(0xAC);
  ac3 = read_int16(0xAE);
  ac4 = read_int16(0xB0);
  ac5 = read_int16(0xB2);
  ac6 = read_int16(0xB4);
  b1 = read_int16(0xB6);
  b2 = read_int16(0xB8);
  mb = read_int16(0xBA);
  mc = read_int16(0xBC);
  md = read_int16(0xBE);
  if (ac1==0 || ac1==0xFFF) return false;
  if (ac2==0 || ac2==0xFFF || ac2==ac1) return false;
  return true;
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
int16_t BMP180::getTemperature(uint16_t ut) {
  int32_t x1, x2;
  int16_t result;
  
  x1 = (((int32_t)ut - (int32_t)ac6)*(int32_t)ac5) >> 15;
  x2 = ((int32_t)mc << 11)/(x1 + md);
  b5 = x1 + x2;
  result=((b5 + 8)>>4);

  return result;  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so getTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
int32_t BMP180::getPressure(uint32_t up) {
  int32_t x1, x2, x3, b3, b6, p;
  uint32_t b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * ((b6 * b6)>>12))>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((int32_t)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (uint32_t)(x3 + 32768))>>15;
  
  b7 = ((uint32_t)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1= (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}

// Read 1 byte from the BMP085 at 'address'
int8_t BMP180::read(uint8_t address) {
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 1);
  return (int8_t)port.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t BMP180::read_int16(uint8_t address) {
  uint8_t msb, lsb;
  
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 2);
  msb = port.read();
  lsb = port.read();
  
  return (int16_t) msb<<8 | lsb;
}

void BMP180::startMeasurementCore() {
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  port.beginTransmission(ADDRESS);
  port.write(0xF4);
  port.write(0x2E);
  port.endTransmission();
}

void BMP180::finishTempCore() {
  // Read two bytes from registers 0xF6 and 0xF7
  UT = read_int16(0xF6);

  //Start pressure measurement 
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  port.beginTransmission(ADDRESS);
  port.write(0xF4);
  port.write(0x34 | (OSS<<6));
  port.endTransmission();
}

void BMP180::finishPresCore() {
  uint8_t msb, lsb, xlsb;
    
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  port.beginTransmission(ADDRESS);
  port.write(0xF6);
  port.endTransmission();
  port.requestFrom(ADDRESS, 3);
  
  msb = port.read();
  lsb = port.read();
  xlsb = port.read();
  
  UP = (((uint32_t) msb << 16) | ((uint32_t) lsb << 8) | (uint32_t) xlsb) >> (8-OSS);
}

bool BMP180::noBlockTakeMeasurement() {
  switch(phase) {
    case 0:
      t=0;
      startMeasurementCore();
      phase=1;
      return false;
    case 1:
      if(t>5) {
        finishTempCore();
        phase=2;
        return false;
      } else {
        return false;
      }
    case 2:
      if(t>(2+(3<<OSS))) {
        finishPresCore();
        phase=0;
        return true;
      } else {
        return false;
      }
  }
}

void BMP180::takeMeasurement() {
  startMeasurementCore();
  delay(5);
  finishTempCore();
  delay(2+(3<<OSS));
  finishPresCore();
}



