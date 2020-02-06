#include "RMC.h"
#include <Arduino.h> //For Serial
signed long gh=0,gn=0,gs=0,gd=0,gm=0,gy=0;
volatile int validgps=0;

//Accept one character, advance state if necessary, 
//return true if there is a new complete valid sentence, false otherwise
typedef bool (*State)(char);

static bool expectDollar(char);
static bool expectG(char);
static bool expectP(char);
static bool expectR(char);
static bool expectM(char);
static bool expectC(char);
static bool expectComma0(char);
static bool expectHour0(char);
static bool expectHour1(char);
static bool expectMinute0(char);
static bool expectMinute1(char);
static bool expectSecond0(char);
static bool expectSecond1(char);
static bool countCommaToDate(char);
static bool expectYear0(char);
static bool expectYear1(char);
static bool expectMonth0(char);
static bool expectMonth1(char);
static bool expectDay0(char);
static bool expectDay1(char);
static State state=expectDollar;

bool process(char in) {
  state(in);
}

//GPRMC parsing
static int classify(char in) {
  return in-' ';
  if(in>='0' && in<'9') return in-'0';
  if(in>='A' && in<'Z') return in-'A'+10;
  if(in==',') return 26+10+0;
  if(in==':') return 26+10+1;
  if(in=='.') return 26+10+2;
  if(in=='*') return 26+10+3;
  if(in=='$') return 26+10+4;
}

static bool expectDollar(char in) {
  if(in=='$') state=expectG;
  return false;
}

static bool expectG(char in) {
  if(in=='G') {
    state=expectP;
    return false;
  }
  state=expectDollar;
  return false;
}

static bool expectP(char in) {
  if(in=='P') {
    state=expectR;
    return false;
  }
  state=expectDollar;
  return false;
}

static bool expectR(char in) {
  if(in=='R') {
    state=expectM;
    return false;
  }
  state=expectDollar;
  return false;
}

static bool expectM(char in) {
  if(in=='M') {
    state=expectC;
    return false;
  }
  state=expectDollar;
  return false;
}

static bool expectC(char in) {
  if(in=='C') {
    state=expectComma0;
    return false;
  }
  state=expectDollar;
  return false;
}

static bool expectComma0(char in) {
  if(in==',') {
    state=expectHour0;
    return false;
  }
  Serial.println("Problem with GPS time: Comma0");
  state=expectDollar;
  return false;
}

static bool expectHour0(char in) {
  if(in>='0' && in<='9') {
    gh=(in-'0')*10;
    state=expectHour1;
    return false;
  }
  Serial.println("Problem with GPS time: Hour0");
  state=expectDollar;
  return false;
}

static bool expectHour1(char in) {
  if(in>='0' && in<='9') {
    gh+=(in-'0');
    state=expectMinute0;
    return false;
  }
  Serial.println("Problem with GPS time: Hour1");
  state=expectDollar;
  return false;
}

static bool expectMinute0(char in) {
  if(in>='0' && in<='9') {
    gn=(in-'0')*10;
    state=expectMinute1;
    return false;
  }
  Serial.println("Problem with GPS time: Minute0");
  state=expectDollar;
  return false;
}

static bool expectMinute1(char in) {
  if(in>='0' && in<='9') {
    gn+=(in-'0');
    state=expectSecond0;
    return false;
  }
  Serial.println("Problem with GPS time: Minute1");
  state=expectDollar;
  return false;
}

static bool expectSecond0(char in) {
  if(in>='0' && in<='9') {
    gs=(in-'0')*10;
    state=expectSecond1;
    return false;
  }
  Serial.println("Problem with GPS time: Second0");
  state=expectDollar;
  return false;
}

int commasToCount;

static bool expectSecond1(char in) {
  if(in>='0' & in<='9') {
    gs+=(in-'0');
    commasToCount=8;
    state=countCommaToDate;
    return false;
  }
  Serial.println("Problem with GPS time: Second0");
  state=expectDollar;
  return false;
}

static bool countCommaToDate(char in) {
  if(in==',') {
    commasToCount--;
    if(commasToCount==0) state=expectDay0;
    return false;
  }
}

static bool expectDay0(char in) {
  if(in>='0' && in<='9') {
    gd=(in-'0')*10;
    state=expectDay1;
    return false;
  }
  Serial.println("Problem with GPS time: Day0");
  state=expectDollar;
  return false;
}

static bool expectDay1(char in) {
  if(in>='0' && in<='9') {
    gd+=(in-'0');
    state=expectMonth0;
    return false;
  }
  Serial.println("Problem with GPS time: Day1");
  state=expectDollar;
  return false;
}

static bool expectMonth0(char in) {
  if(in>='0' && in<='9') {
    gm=(in-'0')*10;
    state=expectMonth1;
    return false;
  }
  Serial.println("Problem with GPS time: Month0");
  state=expectDollar;
  return false;
}

static bool expectMonth1(char in) {
  if(in>='0' && in<='9') {
    gm+=(in-'0');
    state=expectYear0;
    return false;
  }
  Serial.println("Problem with GPS time: Month1");
  state=expectDollar;
  return false;
}

static bool expectYear0(char in) {
  if(in>='0' && in<='9') {
    gy=(in-'0')*10+2000;
    state=expectYear1;
    return false;
  }
  Serial.println("Problem with GPS time: Year0");
  state=expectDollar;
  return false;
}

static bool expectYear1(char in) {
  //No matter what happens, this is the final state for this sentence. If
  //we accept, we go back to expectDollar to wait for the next sentence.
  //If we reject, we go back to expectDollar as error handling, just like
  //all other sentences.
  state=expectDollar; 
  if(in>='0' && in<='9') {
    gy+=(in-'0');
    //Validate GPS time
    if(gh>=0 && gh<24 && gn>=0 && gn<60 && gs>=0 && gs<60 && gm >=1 && gm<=12) {
      //Time is valid
      validgps++;
      return true;
    } else {
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
  state=expectDollar;
  return false;
}


