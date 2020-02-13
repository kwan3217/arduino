#include "gprmc.h"

//Hours the local time zone is behind UTC during standard time - positive for all US time zones
const int tz=7;
//Set to true if the location you are in uses DST - If so, code figures out if DST is in effect, if not, uses standard time year-round
const bool useDST=true;

bool GPRMC::expectDollar(char in) {
  if(in=='$') state=&GPRMC::expectG;
  return false;
}

bool GPRMC::expectG(char in) {
  if(in=='G') {
    state=&GPRMC::expectP;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectP(char in) {
  if(in=='P') {
    state=&GPRMC::expectR;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectR(char in) {
  if(in=='R') {
    state=&GPRMC::expectM;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectM(char in) {
  if(in=='M') {
    state=&GPRMC::expectC;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectC(char in) {
  if(in=='C') {
    state=&GPRMC::expectComma2;
    return false;
  }
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectComma2(char in) {
  if(in==',') {
    state=&GPRMC::expectHour0;
    return false;
  }
  Serial.println("Problem with GPS time: Comma0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectHour0(char in) {
  if(in>='0' && in<='9') {
    gh=(in-'0')*10;
    state=&GPRMC::expectHour1;
    return false;
  }
  Serial.println("Problem with GPS time: Hour0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectHour1(char in) {
  if(in>='0' && in<='9') {
    gh+=(in-'0');
    state=&GPRMC::expectMinute0;
    return false;
  }
  Serial.println("Problem with GPS time: Hour1");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMinute0(char in) {
  if(in>='0' && in<='9') {
    gn=(in-'0')*10;
    state=&GPRMC::expectMinute1;
    return false;
  }
  Serial.println("Problem with GPS time: Minute0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMinute1(char in) {
  if(in>='0' && in<='9') {
    gn+=(in-'0');
    state=&GPRMC::expectSecond0;
    return false;
  }
  Serial.println("Problem with GPS time: Minute1");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectSecond0(char in) {
  if((in>='0') && (in<='9')) {
    gs=(in-'0')*10;
    state=&GPRMC::expectSecond1;
    return false;
  }
  Serial.println("Problem with GPS time: Second0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectSecond1(char in) {
  if((in>='0') & (in<='9')) {
    gs+=(in-'0');
    state=&GPRMC::maybeFraction;
    return false;
  }
  Serial.println("Problem with GPS time: Second0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::maybeFraction(char in) {
  if(in==',') {
    state=&GPRMC::expectAV;
    return false;
  } else if(in=='.') {
    state=&GPRMC::expectFraction;
    divisor=1;
    gf=0;
    return false;
  }
  Serial.println("Problem with GPS second fraction: maybeFraction");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectFraction(char in) {
  if(in==',') {
    gf/=divisor;
    state=&GPRMC::expectAV;
    return false;
  } else if((in>='0') & (in<='9')) {
    divisor*=10;
    gf*=10;
    gf+=(in-'0');
    return false;
  }
  Serial.println("Problem with GPS second fraction: expectFraction");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectAV(char in) {
  if(in=='A') {
    gvalid=true;
    commasToCount=7;
    state=&GPRMC::countCommaToDate;
    return false;
  } else if(in=='V') {
    gvalid=false;
    commasToCount=7;
    state=&GPRMC::countCommaToDate;
    return false;
  } else if(in==',') {
    gvalid=false;
    commasToCount=6;
    state=&GPRMC::countCommaToDate;
  }
  Serial.println("Problem with GPS time: expectAV");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::countCommaToDate(char in) {
  if(in==',') {
    commasToCount--;
    if(commasToCount==0) state=&GPRMC::expectDay0;
    return false;
  }
  return false;
}

bool GPRMC::expectDay0(char in) {
  if(in>='0' && in<='9') {
    gd=(in-'0')*10;
    state=&GPRMC::expectDay1;
    return false;
  }
  Serial.println("Problem with GPS time: Day0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectDay1(char in) {
  if(in>='0' && in<='9') {
    gd+=(in-'0');
    state=&GPRMC::expectMonth0;
    return false;
  }
  Serial.print("Problem with GPS time: Day1 ");
  Serial.println(in);
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMonth0(char in) {
  if(in>='0' && in<='9') {
    gm=(in-'0')*10;
    state=&GPRMC::expectMonth1;
    return false;
  }
  Serial.println("Problem with GPS time: Month0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectMonth1(char in) {
  if(in>='0' && in<='9') {
    gm+=(in-'0');
    state=&GPRMC::expectYear0;
    return false;
  }
  Serial.println("Problem with GPS time: Month1");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectYear0(char in) {
  if(in>='0' && in<='9') {
    gy=(in-'0')*10+2000;
    state=&GPRMC::expectYear1;
    return false;
  }
  Serial.println("Problem with GPS time: Year0");
  state=&GPRMC::expectDollar;
  return false;
}

bool GPRMC::expectYear1(char in) {
  if(in>='0' && in<='9') {
    gy+=(in-'0');
    //Validate GPS time
    if(gh>=0 && gh<24 && gn>=0 && gn<60 && gs>=0 && gs<60 && gm >=1 && gm<=12) {
      //Time is valid
      wellformed=true;
      valid=gvalid;
      count++;
      m=gm;d=gd;y=gy;
      h=gh;n=gn;s=gs;
    } else {
      wellformed=false;
      Serial.print("Problem with GPS time: ");
      if(gh<10) Serial.print("0");
      Serial.print(gh,DEC);
      Serial.print(":");
      if(gn<10) Serial.print("0");
      Serial.print(gn,DEC);
      Serial.print(":");
      if(gs<10) Serial.print("0");
      Serial.print(gs,DEC);
      Serial.print(" ");
      Serial.print(gm,DEC);
      Serial.print("/");
      Serial.print(gd,DEC);
      Serial.print("/");
      Serial.println(gy,DEC);
    }
  }
  state=&GPRMC::expectDollar;
  return wellformed;
}
