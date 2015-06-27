#include <PCharlie.h>

//Hours the local time zone is behind UTC during standard time - positive for all US time zones
const int tz=7;
//Set to true if the location you are in uses DST - If so, code figures out if DST is in effect, if not, uses standard time year-round
const bool useDST=true;

volatile signed long h=0,n=0,s=0,t=0,u=0,d=0,m=0,y=0;
signed long gh=0,gn=0,gs=0,gd=0,gm=0,gy=0;

volatile int ppsCount=0,validgps=0;
volatile bool lockU=false;
//which of the 60 lights in 5 virtual banks is on
//bank 0=hour, 1=minute, 2=second, 3=third, 4=PPS indicator (re-use of hour)
//if ls[x]>60, then no lights in bank x will be on
volatile int ls[5];         
//Each time the lights are switched, we use this array
//to tell us which bank to light this time. If a bank
//appears more often, it will be brighter
                            //0 1 2 3 4 5 6 7 8 9 A B C D E F
static const int multiplex[]={2,3,1,3,3,2,3,3,0,3,1,3,4,3,2,3};
volatile signed long last_micro=0;
volatile signed long lightRate=0;

const unsigned long MILLION=1000000;

void setup() {
  Serial.begin(115200);
  Serial.print("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0*28\r\n");  
  allDark();
  ls[4]=0;
  //Set the ATmega to listen to the GPS, not the USB port
  //Pin 12 - low is GPS, high is USB. There is an external 
  //pullup, so input is USB also. This means that when the
  //part resets, it automatically switches back to USB, 
  //otherwise Arduino programming wouldn't work.
  pinMode(12,OUTPUT);
  digitalWrite(12,LOW);
//Activate the 20k pullup on the PPS line. With no GPS,
//this means that the pin will not respond to things like
//touching the PPS pins. With a GPS, the line will follow
//whatever the GPS puts on it.
  pinMode(2,INPUT);
  digitalWrite(2,HIGH); 
  attachInterrupt(0,pps,RISING);
}

void setClockLights() {
  ls[0]=(h*5+n/12)%60;
  ls[1]=n;
  ls[2]=s;
  ls[3]=t;
}

void pps() {
  ppsCount++;
  signed long oldS=s;
  signed long oldN=n;
  signed long oldH=h;
    s++;
    if(s==60) {
      Serial.print("lightRate: ");
      Serial.println(lightRate,DEC);
      Serial.print("ppsCount: ");
      Serial.println(ppsCount,DEC);
      Serial.print("validgps: ");
      Serial.println(validgps,DEC);
      if(gh<10) Serial.print('0');Serial.print(gh,DEC);
      Serial.print(":");
      if(gn<10) Serial.print('0');Serial.print(gn,DEC);
      Serial.print(":");
      if(gs<10) Serial.print('0');Serial.println(gs,DEC);
      ppsCount=0;
      validgps=0;
    }
    lightRate=0;
  u=0;
  
  while(s>=60) {
    n++;
    s-=60;
  }
  while(n>=60) {
    h++;
    n-=60;
  }
  while(h>=24) {
    h-=24;
  }
  setClockLights();
}

void update_clock() {
  unsigned long this_micro=micros();
  if (this_micro<last_micro) {
    //Easier done than said.
    //Pretend that we are using an unsigned int that has a maximum value of 9 and min of 0.
    //Last time around, it was 8, this time it is 2. The clock ticked from 8 to 9, (1 tick)
    //then 0 (1 more tick) then to 2 (2 more ticks). So, we want (max-last)+1+this.
    unsigned long delta=0xFFFFFFFF-last_micro;
    delta+=1;
    delta+=this_micro;
    u+=delta;
  } else {
    u+=(this_micro-last_micro);
  }
  t=u*60/MILLION;
  last_micro=this_micro;
  setClockLights();
}

//GPRMC parsing
int classify(char in) {
  return in-' ';
  if(in>='0' && in<'9') return in-'0';
  if(in>='A' && in<'Z') return in-'A'+10;
  if(in==',') return 26+10+0;
  if(in==':') return 26+10+1;
  if(in=='.') return 26+10+2;
  if(in=='*') return 26+10+3;
  if(in=='$') return 26+10+4;
}

typedef void (*State)(char);

State state=expectDollar;

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
  if(in>='0' && in<='9') {
    gs=(in-'0')*10;
    state=expectSecond1;
    return;
  }
      Serial.println("Problem with GPS time: Second0");
  state=expectDollar;
}

int commasToCount;

void expectSecond1(char in) {
  if(in>='0' & in<='9') {
    gs+=(in-'0');
    commasToCount=6;
    state=countCommaToDate;
    return;
  }
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
  state=expectDollar;
}

void expectDay1(char in) {
  if(in>='0' && in<='9') {
    gd+=(in-'0');
    state=expectMonth0;
    return;
  }
  state=expectDollar;
}

void expectMonth0(char in) {
  if(in>='0' && in<='9') {
    gm=(in-'0')*10;
    state=expectMonth1;
    return;
  }
  state=expectDollar;
}

void expectMonth1(char in) {
  if(in>='0' && in<='9') {
    gm+=(in-'0');
    state=expectYear0;
    return;
  }
  state=expectDollar;
}

void expectYear0(char in) {
  if(in>='0' && in<='9') {
    gy=(in-'0')*10+2000;
    state=expectYear1;
    return;
  }
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
  if(useDST) {
    //Is DST in effect?
    if(m < 3) { //Before March, not DST
      result=false;
    } else if (m>11) { //After November, not DST
      result=false;
    } else if (m>3 && m<11) { //After march and before November, in DST
      result=true;
    } else if(m==3) { //In March, figure out what day is the second Sunday in March
      int firstDayOfMarch=day_of_week(y,3,1);
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
      if(d<secondSunday) {
        result=false;
      else if(d>secondSunday) {
        result=true;
      else result=(h>=2);
    } else { //Only November is left;            
      int firstDay=day_of_week(y,3,1);
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
      if(d<firstSunday) {
        result=true;
      else if(d>firstSunday) {
        result=false;
      else result=(h<2);
    }
  }
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
      h=(h+isDST(y,m,d,h)?1:0)%12
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

void lights() {
  update_clock();
  ls[4]=digitalRead(2)?0:60;
  lightRate++;
  char j=((char)(lightRate%16));
  if(ls[multiplex[j]]<60) light(((int)(multiplex[j]%4))*100+ls[multiplex[j]]);
}  
  
void loop() {
  lights();
  while(Serial.available()) {
    lights();
    state(Serial.read());
  }
}
