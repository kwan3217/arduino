#include "ak8975.h"
//#include "LPC214x.h"
//#include "Time.h"

// Sets the configuration and mode registers such that the part is continuously
// generating measurements. This function should be called at the beginning of 
// the program
void AK8975::begin() {
  //No special setup is needed. Part runs in single-shot mode.

}

// Read 1 byte from the BMP085 at 'address'
int8_t AK8975::read(uint8_t address) {
  port.beginTransmission(partAddress);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(partAddress, 1);
  return port.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t AK8975::read16(uint8_t address) {
  uint8_t msb, lsb;
  
  port.beginTransmission(partAddress);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(partAddress, 2);
  lsb = port.read();
  msb = port.read();
  
  return (int16_t) msb<<8 | lsb;
}

void AK8975::startMeasurementCore() {
  port.beginTransmission(partAddress);
  port.write(0x0A); //Start at register 0x0A
  port.write(0x01); //Set to single-shot mode
  port.endTransmission();
}  

void AK8975::startMeasurement() {
  if(start) return; //If a measurement is already started, keep doing that one
  startMeasurementCore();
  start=true;
  ready=false;
}

bool AK8975::checkMeasurement() {
  if(!start) {
    return true;
  } else if(!ready) {
    port.beginTransmission(partAddress);
    port.write(0x02); //Start at register 2
    port.endTransmission();
    port.requestFrom(partAddress, 1);
    st1=port.read();
    if(st1&0x01) {
      start=false;
      ready=true;
      finishMeasurementCore();
      return true;
    } else {
      return false;
    }
  }
}

void AK8975::finishMeasurementCore() {
  uint8_t msbx, lsbx;
  uint8_t msby, lsby;
  uint8_t msbz, lsbz;
  
  port.requestFrom(partAddress, 7);
  //Note that this is a little-endian part -- the first byte of each measurement is the low-order byte.
  //st1 is read during checkMeasurement()
  //st1=port.read();
  lsbx = port.read();
  msbx = port.read();
  lsby = port.read();
  msby = port.read();
  lsbz = port.read();
  msbz = port.read();
  st2=port.read();
  x=( (int16_t) msbx<<8 | lsbx);
  y=( (int16_t) msby<<8 | lsby);
  z=( (int16_t) msbz<<8 | lsbz);
  
}

uint8_t AK8975::whoami() {
  return read(0x00);
}

//Manages start and checking of a measurement. Call this in a busy loop, it will return quickly if no measurement is available.
// @return If true, a new consistent measurement set are availalble through read().
bool AK8975::noblockMeasurement() {
  if(!start) startMeasurement();
  return checkMeasurement();
}



