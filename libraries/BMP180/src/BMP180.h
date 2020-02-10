#ifndef BMP180_H
#define BMP180_H

#include "Arduino.h"
#include "Wire.h"

class BMP180 {
  private:
    // b5 is calculated in getTemperature(...), this variable is also used in getPressure(...)
    // so ...Temperature(...) must be called before ...Pressure(...).
    int  b5; 
    TwoWire &port;
    int UT,UP;
    int phase;
    elapsedMillis t;

    int8_t read(uint8_t address);
    int16_t read_int16(uint8_t address);
    int16_t getTemperature(uint16_t ut);
    int32_t getPressure(uint32_t up);
    static const unsigned char ADDRESS=0x77;  // I2C address of BMP085
    unsigned char OSS;  // Oversampling Setting
  public:
    // Calibration values
    short  ac1;
    short  ac2; 
    short  ac3; 
    unsigned short ac4;
    unsigned short ac5;
    unsigned short ac6;
    short  b1; 
    short  b2;
    short  mb;
    short  mc;
    short  md;
    void finishTempCore();
    void finishPresCore();
    void startMeasurementCore();
    BMP180(TwoWire &Lport);
    volatile bool ready;
    bool begin();
    bool readCalibration();
    unsigned char getOSS() {return OSS;};
    bool setOSS(unsigned char Loss) {if(phase!=0) return false;OSS=Loss;return true;};
    void startMeasurement();
    int getTemperatureRaw() {return UT;};
    int getPressureRaw() {return UP;};
    int16_t getTemperature() {return getTemperature(UT);};
    int32_t getPressure() {return getPressure(UP);};
    void takeMeasurement();
    bool noBlockTakeMeasurement();
    unsigned char whoami() {return read(0xD0);};

};

#endif
