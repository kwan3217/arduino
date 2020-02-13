#ifndef gprmc_h
#define gprmc_h

#include "Arduino.h"
#include <cmath>

class GPRMC {
private:
  unsigned int gh,gn,gs;
  float gf;
  unsigned int gd,gm,gy;
  bool gvalid;
  float divisor;
  int commasToCount;
  bool (GPRMC::*state)(char);
  bool expectDollar(char in);
  bool expectG(char in);
  bool expectP(char in);
  bool expectR(char in);
  bool expectM(char in);
  bool expectC(char in);
  bool expectComma2(char in);
  bool expectHour0(char in);
  bool expectHour1(char in);
  bool expectMinute0(char in);
  bool expectMinute1(char in);
  bool expectSecond0(char in);
  bool expectSecond1(char in);
  bool maybeFraction(char in);
  bool expectFraction(char in);
  bool expectAV(char in);
  bool countCommaToDate(char in);
  bool expectDay0(char in);
  bool expectDay1(char in);
  bool expectMonth0(char in);
  bool expectMonth1(char in);
  bool expectYear0(char in);
  bool expectYear1(char in);
public:
  unsigned int h,n,s,d,m,y;
  float f,lat,lon;
  //Current position is valid, as indicated by NMEA A/V mark
  bool valid;
  //Most recent GPRMC sentence was syntactically well-formed
  bool wellformed;
  //Count of well-formed GPRMC sentences
  unsigned int count;
  //Process a character of an NMEA data stream. Returns true if a new
  //GPRMC sentence has been successfully processed.
  bool process(char c) {Serial.print(c);return (this->*state)(c);};
  GPRMC():state(&GPRMC::expectDollar),lat(40.0123456789),lon(-105.123456789),valid(false) {};
};

#endif

