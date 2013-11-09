#include <PCharlie.h>

//Hours the local time zone is behind UTC - positive for all US time zones
//No auto adjust for daylight savings time
const int tz=7;

volatile unsigned long h=0,m=0,s=0,t=0,u=0;
unsigned long gh=0,gm=0,gs=0;
//which of the 60 lights in 5 virtual banks is on
//bank 0=hour, 1=minute, 2=second, 3=third, 4=PPS indicator (re-use of hour)
//if ls[x]>60, then no lights in bank x will be on
volatile int ls[5];         
//Each time the lights are switched, we use this array
//to tell us which bank to light this time. If a bank
//appears more often, it will be brighter
                            //0 1 2 3 4 5 6 7 8 9 A B C D E F
static const int multiplex[]={2,3,1,3,3,2,3,3,0,3,1,3,4,3,2,3};
volatile unsigned long last_micro=0;
volatile unsigned long lightRate=0;

#define MILLION 1000000

void setup() {
  Serial.begin(9600);
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

void incClock() {
  while(u>=MILLION) {
    s++;
    if(s==60) {
      Serial.print("lightRate: ");
      Serial.println(lightRate,DEC);
    }
    lightRate=0;
    
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
  ls[0]=(h*5+m/12)%60;
  ls[1]=m;
  ls[2]=s;
  ls[3]=t;
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
  if(in>='0' && in<='9') {
    gh=(in-'0')*10;
    state=expectHour1;
    return;
  }
  state=expectDollar;
}

void expectHour1(char in) {
  if(in>='0' && in<='9') {
    gh+=(in-'0');
    state=expectMinute0;
    return;
  }
  state=expectDollar;
}

void expectMinute0(char in) {
  if(in>='0' && in<='9') {
    gm=(in-'0')*10;
    state=expectMinute1;
    return;
  }
  state=expectDollar;
}

void expectMinute1(char in) {
  if(in>='0' && in<='9') {
    gm+=(in-'0');
    state=expectSecond0;
    return;
  }
  state=expectDollar;
}

void expectSecond0(char in) {
  if(in>='0' && in<='9') {
    gs=(in-'0')*10;
    state=expectSecond1;
    return;
  }
  state=expectDollar;
}

void expectSecond1(char in) {
  if(in>='0' & in<='9') {
    gs+=(in-'0');
    //Validate GPS time
    if(gh>=0 && gh<24 && gm>=0 && gm<60 && gs>=0 && gs<60) {
      //Time is valid
      h=(gh+24-tz)%12;m=gm;s=gs;
    } else {
      Serial.print("Problem with GPS time: ");
      if(gh<10) Serial.print("0");
      Serial.print(gh,DEC);
      Serial.print(":");
      if(gm<10) Serial.print("0");
      Serial.print(gm,DEC);
      Serial.print(":");
      if(gs<10) Serial.print("0");
      Serial.println(gs,DEC);
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
