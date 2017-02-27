#include <PCharlie.h>

#include "DST.h"
#include "RMC.h"

volatile signed long t=0,u=0;

volatile int ppsCount=0;
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
    char in=Serial.read();
    if(process(in)) toLocalTime();
  }
}
