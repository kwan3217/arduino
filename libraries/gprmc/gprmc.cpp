#include "gprmc.h"

static void expectDollar(char in);
static void expectG(char in);
static void expectP(char in);
static void expectR(char in);
static void expectM(char in);
static void expectC(char in);
static void expectComma0(char in);
static void expectHour0(char in);
static void expectHour1(char in);
static void expectMinute0(char in);
static void expectMinute1(char in);
static void expectSecond0(char in);
static void expectSecond1(char in);
static void countCommaToDate(char in);
static void expectDay0(char in);
static void expectDay1(char in);
static void expectMonth0(char in);
static void expectMonth1(char in);
static void expectYear0(char in);
static void expectYear1(char in);

signed long h=0,n=0,s=0,t=0,u=0,d=0,m=0,y=0;
static signed long gh=0,gn=0,gs=0,gd=0,gm=0,gy=0;
static int commasToCount;
int validgps=0;

State state=expectDollar;

//Hours the local time zone is behind UTC during standard time - positive for all US time zones
const int tz=7;
//Set to true if the location you are in uses DST - If so, code figures out if DST is in effect, if not, uses standard time year-round
const bool useDST=true;

void expectDollar(char in) {
  if(in=='$') state=expectG;
}

void expectG(char in) {
  if(in=='G') {
    state=expectP;
    return;
  }
  state=expectDollar;
}

void expectP(char in) {
  if(in=='P') {
    state=expectR;
    return;
  }
  state=expectDollar;
}

void expectR(char in) {
  if(in=='R') {
    state=expectM;
    return;
  }
  state=expectDollar;
}

void expectM(char in) {
  if(in=='M') {
    state=expectC;
    return;
  }
  state=expectDollar;
}

void expectC(char in) {
  if(in=='C') {
    state=expectComma0;
    return;
  }
  state=expectDollar;
}

void expectComma0(char in) {
  if(in==',') {
    state=expectHour0;
    return;
  }
  Serial.println("Problem with GPS time: Comma0");
  state=expectDollar;
}

void expectHour0(char in) {
  if(in>='0' && in<='9') {
    gh=(in-'0')*10;
    state=expectHour1;
    return;
  }
  Serial.println("Problem with GPS time: Hour0");
  state=expectDollar;
}

void expectHour1(char in) {
  if(in>='0' && in<='9') {
    gh+=(in-'0');
    state=expectMinute0;
    return;
  }
  Serial.println("Problem with GPS time: Hour1");
  state=expectDollar;
}

void expectMinute0(char in) {
  if(in>='0' && in<='9') {
    gn=(in-'0')*10;
    state=expectMinute1;
    return;
  }
  Serial.println("Problem with GPS time: Minute0");
  state=expectDollar;
}

void expectMinute1(char in) {
  if(in>='0' && in<='9') {
    gn+=(in-'0');
    state=expectSecond0;
    return;
  }
  Serial.println("Problem with GPS time: Minute1");
  state=expectDollar;
}

void expectSecond0(char in) {
  if((in>='0') && (in<='9')) {
    gs=(in-'0')*10;
    state=expectSecond1;
    return;
  }
  Serial.println("Problem with GPS time: Second0");
  state=expectDollar;
}

void expectSecond1(char in) {
  if((in>='0') & (in<='9')) {
    gs+=(in-'0');
    commasToCount=8;
    state=countCommaToDate;
    return;
  }
  Serial.println("Problem with GPS time: Second0");
  state=expectDollar;
}

void countCommaToDate(char in) {
  if(in==',') {
    commasToCount--;
    if(commasToCount==0) state=expectDay0;
    return;
  }
}

void expectDay0(char in) {
  if(in>='0' && in<='9') {
    gd=(in-'0')*10;
    state=expectDay1;
    return;
  }
  Serial.println("Problem with GPS time: Day0");
  state=expectDollar;
}

void expectDay1(char in) {
  if(in>='0' && in<='9') {
    gd+=(in-'0');
    state=expectMonth0;
    return;
  }
  Serial.println("Problem with GPS time: Day1");
  state=expectDollar;
}

void expectMonth0(char in) {
  if(in>='0' && in<='9') {
    gm=(in-'0')*10;
    state=expectMonth1;
    return;
  }
  Serial.println("Problem with GPS time: Month0");
  state=expectDollar;
}

void expectMonth1(char in) {
  if(in>='0' && in<='9') {
    gm+=(in-'0');
    state=expectYear0;
    return;
  }
  Serial.println("Problem with GPS time: Month1");
  state=expectDollar;
}

void expectYear0(char in) {
  if(in>='0' && in<='9') {
    gy=(in-'0')*10+2000;
    state=expectYear1;
    return;
  }
  Serial.println("Problem with GPS time: Year0");
  state=expectDollar;
}

//Given the current year/month/day
//Returns 0 (Sunday) through 6 (Saturday) for the day of the week
//From: http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
//This function assumes the month from the caller is 1-12
int day_of_week(int year, int month, int day) {
  //Devised by Tomohiko Sakamoto in 1993, it is accurate for any Gregorian date:
  static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4  };
  year -= month < 3;
  return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7; //This works fine for day=0 or negative (for days before 1st of month), also days after end of month (feb 31st etc)
}

//Input - time in local time zone
//  Year - 4 digit year
//  Month - month number, 1=January
//  Day - conventional calendar day of month number
//  hour - local hour in 24h format
//Return - true if in DST, false if not
bool isDST(int year, int month, int day, int hour) {
  bool result=false;
  const bool verbose=true;
  if(verbose){
    Serial.print(year);Serial.print(',');
    Serial.print(month);Serial.print(',');
    Serial.print(day);Serial.print(',');
    Serial.println(hour);
  }
  if(useDST) {
    //Is DST in effect?
    if(month < 3) { //Before March, not DST
      result=false;
      if(verbose)Serial.println("Before March, not DST");
    } else if (month>11) { //After November, not DST
      result=false;
      if(verbose)Serial.println("After November, not DST");
    } else if (month>3 && month<11) { //After march and before November, in DST
      result=true;
      if(verbose)Serial.println("After March and before November, in DST");
    } else if(month==3) { //In March, figure out what day is the second Sunday in March
      int firstDayOfMarch=day_of_week(year,3,1);
      if(firstDayOfMarch==0)firstDayOfMarch=7;
      //What day is March 1?   Number    Second sunday
      //         Sunday           0 (7)              8  (15-n)
      //         Monday           1                 14  (15-n)
      //         Tuesday          2                 13  (15-n)
      //         Wednesday        3                 12  (15-n)
      //         Thurdsay         4                 11  (15-n)
      //         Friday           5                 10  (15-n)
      //         Saturday         6                  9  (15-n)
      int secondSunday=15-firstDayOfMarch;
      if(verbose) {
        Serial.print("In March ");
        Serial.print(" day=");Serial.print(day);
        Serial.print(" secondSunday=");Serial.print(secondSunday);
      }
      if(day<secondSunday) {
        result=false;
        if(verbose)Serial.println("Before second Sunday, not DST");
      } else if(day>secondSunday) {
        result=true;
        if(verbose)Serial.println("After second Sunday, in DST");
      } else {
        result=(hour>=2);
        if(verbose){Serial.print("On second Sunday, hour=");Serial.print(hour);Serial.print(", DST is");Serial.println(result?"on":"off");}
      }
    } else { //Only November is left;            
      int firstDay=day_of_week(year,3,1);
      if(firstDay==0)firstDay=7;
      //What day is November 1?   Number    First sunday
      //         Sunday           0 (7)              1  (8-n)
      //         Monday           1                  7  (8-n)
      //         Tuesday          2                  6  (8-n)
      //         Wednesday        3                  5  (8-n)
      //         Thurdsay         4                  4  (8-n)
      //         Friday           5                  3  (8-n)
      //         Saturday         6                  2  (8-n)
      int firstSunday=8-firstDay;
      if(verbose){
        Serial.print("In November ");
        Serial.print(" day=");Serial.print(day);
        Serial.print(" firstSunday=");Serial.print(firstSunday);
      }
      if(day<firstSunday) {
        if(verbose)Serial.println("Before first Sunday, in DST");
        result=true;
      } else if(day>firstSunday) {
        if(verbose)Serial.println("After first Sunday, not DST");
        result=false;
      } else {
        result=(hour<2);
        if(verbose){Serial.print("On first Sunday, hour=");Serial.print(hour);Serial.print(", DST is");Serial.println(result?"on":"off");}
      }
    }
  }
  if(verbose){Serial.print("Timezone offset: ");Serial.println(tz-(result?1:0));}
  return result;
}

void expectYear1(char in) {
  if(in>='0' && in<='9') {
    gy+=(in-'0');
    //Validate GPS time
    if(gh>=0 && gh<24 && gn>=0 && gn<60 && gs>=0 && gs<60 && gm >=1 && gm<=12) {
      //Time is valid
      validgps++;
      m=gm;d=gd;y=gy;
      //Initial standard timezone correction. h is in 24h time to start with
      h=(gh-tz);n=gn;s=gs;
      if(h<0) {
        h+=24;
        d--; //Might result in d=0, but we don't care, below will still work 
      }
      //Now change h to 12 hour and account for DST
      bool thisDST=isDST(y,m,d,h);
      h=(h+(thisDST?1:0))%24;
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
}
