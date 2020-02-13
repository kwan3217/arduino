#ifndef BMP180_H
#define BMP180_H

#include "Arduino.h"
#include "Wire.h"

class BMP180 {
  private:
    // b5 is calculated in getTemperature(...), this variable is also used in getPressure(...)
    // so ...Temperature(...) must be called before ...Pressure(...).
    static const uint8_t ADDRESS=0x77;  // I2C address of BMP085
    TwoWire &port;
    //These are the raw measurements. Actually 16-bit unsigned, but 
    //the reference algorithm has them as 32-bit signed. We will keep it this way
     int32_t UT,UP;
    int phase;
    elapsedMillis t;
    // Calibration values
     int16_t ac1;
     int16_t ac2; 
     int16_t ac3; 
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
     int16_t b1; 
     int16_t b2;
     int16_t mb;
     int16_t mc;
     int16_t md;
    //intermediate values
     int32_t x1_0,x2_0;
     int32_t b5;
    //int32_t T
     int32_t b6;
     int32_t x1_1,x2_1,x3_1;
     int32_t b3;
     int32_t x1_2,x2_2,x3_2;
    uint32_t b4;
    uint32_t b7;
     int32_t p_0;
     int32_t x1_3a,x1_3b,x2_3;
     int32_t p_1;
    //int32_t P;
    int8_t read(uint8_t address);
    int16_t read_int16(uint8_t address);
    int32_t getTemperature(int32_t ut); //Could take a uint16_t, but algorihtm has int32_t. Return could fit in an int16_t, but algorithm has int32_t
    int32_t getPressure(int32_t up);    //Could take a uint16_t, but algorihtm has int32_t. Return could fit in a uint32_t, but algorithm has int32_t
    uint8_t OSS;  // Oversampling Setting
  public:
    void finishTempCore();
    void finishPresCore();
    void startMeasurementCore();
    BMP180(TwoWire &Lport);
    volatile bool ready;
    bool begin();
    bool readCalibration();
    void getCalibration(int16_t& Lac1, int16_t& Lac2, int16_t& Lac3, uint16_t& Lac4, uint16_t& Lac5, uint16_t& Lac6,
                        int16_t& Lb1,  int16_t& Lb2,  int16_t& Lmb,   int16_t& Lmc,   int16_t& Lmd) {
      Lac1=ac1; Lac2=ac2; Lac3=ac3; Lac4=ac4; Lac5=ac5; Lac6=ac6;
      Lb1 =b1;  Lb2 =b2;  Lmb =mb;  Lmc =mc;  Lmd =md;
    }
    void getIntermediate(
     int32_t& Lx1_0,
     int32_t& Lx2_0,
     int32_t& Lb5,
     int32_t& Lb6,
     int32_t& Lx1_1,
     int32_t& Lx2_1,
     int32_t& Lx3_1,
     int32_t& Lb3,
     int32_t& Lx1_2,
     int32_t& Lx2_2,
     int32_t& Lx3_2,
    uint32_t& Lb4,
    uint32_t& Lb7,
     int32_t& Lp_0,
     int32_t& Lx1_3a,
     int32_t& Lx1_3b,
     int32_t& Lx2_3,
     int32_t& Lp_1
   ) {
      Lx1_0 =x1_0;
      Lx2_0 =x2_0;
      Lb5   =b5;
      Lb6   =b6;
      Lx1_1 =x1_1;
      Lx2_1 =x2_1;
      Lx3_1 =x3_1;
      Lb3   =b3;
      Lx1_2 =x1_2;
      Lx2_2 =x2_2;
      Lx3_2 =x3_2;
      Lb4   =b4;
      Lb7   =b7;
      Lp_0  =p_0;
      Lx1_3a=x1_3a;
      Lx1_3b=x1_3b;
      Lx2_3 =x2_3;
      Lp_1  =p_1;
    };
    uint8_t getOSS() {return OSS;};
    bool setOSS(unsigned char Loss) {if(phase!=0) return false;OSS=Loss;return true;};
    int32_t getTemperatureRaw() {return UT;};
    int32_t getPressureRaw() {return UP;};
    int32_t getTemperature() {return getTemperature(UT);};
    int32_t getPressure() {return getPressure(UP);};
    void takeMeasurement();
    bool noBlockTakeMeasurement(int& phaseChange);
    bool noBlockTakeMeasurement() {int blah;return noBlockTakeMeasurement(blah);};
    unsigned char whoami() {return read(0xD0);};

};

#endif
