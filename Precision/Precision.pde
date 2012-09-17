#include <PCharlie.h>

void setup() {
  Serial.begin(9600);  
  allDark();
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH);
//Activate the 20k pullup on the PPS line. With no GPS,
//this means that the pin will not respond to things like
//touching the PPS pins. With a GPS, the line will follow
//whatever the GPS puts on it.
  pinMode(2,INPUT);
  digitalWrite(2,HIGH); 
  attachInterrupt(0,pps,RISING);
}

volatile unsigned long h=0,m=0,s=0,t=0,u=0;
unsigned long gh=0,gm=0,gs=0;
volatile unsigned long last_micro=0,this_micro;

#define MILLION 1000000

void incClock() {
  while(u>=MILLION) {
    s++;
    u-=MILLION;
  }
  t=u*60/MILLION;
  while(s>=60) {
    m++;
    s-=60;
  }
  while(m>=60) {
    h++;
    m-=60;
  }
  while(h>=24) {
    h-=24;
  }
}

void pps() {
  //Otherwise the next update_clock will credit time before the tick to the next update.
  //micros() doesn't update during an interrupt but we don't need it to.
  last_micro=micros();
//If the PPS comes in in the first half of the second 0<=u<500000, presume that the local clock
//has passed the top of the second on its own and incremented itself. Otherwise, presume that
//the local clock has not passed the top of the second, and therefore needs to be incremented.
  if(u>(MILLION/2)) {
    u=MILLION;
    incClock();
  }
  u=0;  
}

void update_clock() {
  this_micro=micros();
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
  incClock();
  last_micro=this_micro;
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
  state=expectDollar;
}

void expectHour0(char in) {
  if(in>='0' & in<='9') {
    gh=(in-'0')*10;
    state=expectHour1;
    return;
  }
  state=expectDollar;
}

void expectHour1(char in) {
  if(in>='0' & in<='9') {
    gh+=(in-'0');
    state=expectMinute0;
    return;
  }
  state=expectDollar;
}

void expectMinute0(char in) {
  if(in>='0' & in<='9') {
    gm=(in-'0')*10;
    state=expectMinute1;
    return;
  }
  state=expectDollar;
}

void expectMinute1(char in) {
  if(in>='0' & in<='9') {
    gm+=(in-'0');
    state=expectSecond0;
    return;
  }
  state=expectDollar;
}

void expectSecond0(char in) {
  if(in>='0' & in<='9') {
    gs=(in-'0')*10;
    state=expectSecond1;
    return;
  }
  state=expectDollar;
}

const int tz=6;

void expectSecond1(char in) {
  if(in>='0' & in<='9') {
    gs+=(in-'0');
    h=(gh+24-tz)%12;m=gm;s=gs;
  }
  state=expectDollar;
}

volatile unsigned long lightRate=0;


#define SWITCH

void lights() {
  update_clock();
  long j=(u/333)%16;
  #ifdef SWITCH
  switch(j) {
    case 0:
      light(000+(h*5+m/12)%60); //hour hand
      break;
    case 1:
    case 2:
    case 9:
      light(100+m);             //minute hand
      break;
    case 3:
    case 4:
    case 11:
      light(200+s);             //second hand
      break;
    default:
      light(300+t); //else dark();             //third hand
      break;
  }
  #else
  if(j==0 || j==8) {
    light(000+(h*5+m/12)%60); //hour hand
  } else if (j==1 || j==2 || j==9) {
    light(100+m);             //minute hand
  } else if (j==3 || j==4 || j==11) {
    light(200+s);             //second hand
  } else {
    light(300+t); //else dark();             //third hand
  }
  #endif
}  
  
void loop() {
  lights();
  while(Serial.available()) {
    lights();
    state(Serial.read());
  }
}
