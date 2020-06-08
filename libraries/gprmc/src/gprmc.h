#ifndef gprmc_h
#define gprmc_h

#include "Arduino.h"
#include <cmath>

/*
eg3. $GPRMC,220516.000,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70
              1        2    3    4    5     6    7    8      9     10  11 12

      1   220516     Time Stamp   HHNNSS(.S+)
      2   A          validity - A-ok, V-invalid
      3   5133.82    current Latitude   (DDMM.MM)?
      4   N          North/South       [NS]?
      5   00042.24   current Longitude (DDDMM.MM)?
      6   W          East/West         [EW]?
      7   173.8      Speed in knots    (KKK(.K)?)?
      8   231.8      True course       (KKK(.K)?)?
      9   130694     Date Stamp        DDMMYY
      10  004.2      Variation         (KKK(.K)?)?
      11  W          East/West         [EW]?
      12  *70        checksum

 */

class GPRMC {
private:
  float f;
  unsigned int gh,gn;
  float gs;
  unsigned int gd,gm,gy;
  bool gvalid;
  float divisor;
  int glatdeg;
  float glatmin;
  int glondeg;
  float glonmin;
  float gspd,gtrk;
  int commasToCount;
  char runChksum,recChksum;
  bool (GPRMC::*state)(char);
  bool expectDollar(char in);
  bool expectG(char in);
  bool expectP(char in);
  bool expectR(char in);
  bool expectM(char in);
  bool expectC(char in);
  bool expectComma1(char in);
  bool expectHour0(char in);
  bool expectHour1(char in);
  bool expectMinute0(char in);
  bool expectMinute1(char in);
  bool expectSecond0(char in);
  bool expectSecond1(char in);
  bool maybeSFraction(char in);
  bool expectSFraction(char in);
  bool expectAV(char in);
  bool expectComma3(char in);
  bool expectLatDegMin(char in);
  bool expectLatFrac(char in);
  bool expectNS(char in);
  bool expectComma5(char in);
  bool expectLonDegMin(char in);
  bool expectLonFrac(char in);
  bool expectEW(char in);
  bool expectComma7(char in);
  bool expectSpd(char in);
  bool expectSpdFrac(char in);
  bool expectTrk(char in);
  bool expectTrkFrac(char in);
  bool expectDay0(char in);
  bool expectDay1(char in);
  bool expectMonth0(char in);
  bool expectMonth1(char in);
  bool expectYear0(char in);
  bool expectYear1(char in);
  bool waitForStar(char in);
  bool expectChksum0(char in);
  bool expectChksum1(char in);
  bool validate();
public:
  unsigned int h=0,n=0;
  float s=0;
  unsigned int d=0,m=0,y=0;
  float lat=NAN,lon=NAN;
  //Current position is valid, as indicated by NMEA A/V mark
  bool valid=false;
  //Most recent GPRMC sentence was syntactically well-formed
  bool wellformed=false;
  //Count of well-formed GPRMC sentences
  unsigned int count=0;
  //Process a character of an NMEA data stream. Returns true if a new
  //GPRMC sentence has been successfully processed.
  bool process(char c) {runChksum^=c;return (this->*state)(c);};
  GPRMC():state(&GPRMC::expectDollar),lat(40.0123456789),lon(-105.123456789),valid(false) {};
};

#endif

