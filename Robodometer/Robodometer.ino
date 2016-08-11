//Robodometer. See theory documentation at https://github.com/kwan3217/roboSim/wiki/Odometry

#include <WireSlaveReg.h>

struct State {
  /* 0x00      */  int32_t WC;
  /* 0x04      */ uint32_t DT;
  /* 0x08      */ uint32_t T0;
  /* 0x0C      */ uint32_t T1;
  /* 0x10      */ uint16_t ID;
  /* 0x12      */ uint16_t RST;
  /* 0x14,0x16 */ uint16_t A[2];
  /* 0x18,0x1A */ uint16_t A_min[2];
  /* 0x1C,0x1E */ uint16_t A_max[2];
  /* 0x20,0x22 */ uint16_t A_low[2];
  /* 0x24,0x26 */ uint16_t A_high[2];
  /* 0x28,0x29 */ uint8_t  A_lowfrac[2];
  /* 0x2A,0x2B */ uint8_t  A_highfrac[2];
  /* 0x2C,0x2E */ uint16_t servo_high[2];
  /* 0x30,0x32 */ uint16_t servo_total[2];
  /* 0x34      */ uint8_t  serial;
};

State state;
char* regs=(char*)&state;

bool black[2];
unsigned int sector;
int32_t wheelCount;
uint32_t T0[4]; //Time we entered the sector two clicks ago
uint32_t T1[4]; //Time we entered the sector last click

//Index: natural binary number
//Value: Gray code for that index
//Works as its own inverse IE grayCode[grayCode[i]]=i meaning it can be used to encode or decode
const int grayCode[4]={0,1,3,2};

void initServo(int i) {
  
}

void setup() {
  pinMode(A0,INPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  pinMode(A5,INPUT);
  pinMode(13,OUTPUT);
  digitalWrite(A2,LOW);
  digitalWrite(A1,HIGH);
  digitalWrite(13,LOW);
  digitalWrite(A4,1);
  digitalWrite(A5,1);
  state.ID=0x3217;
  for(int i=0;i<2;i++) {
    black[i]=false;
    state.A_min[i]=300;
    state.A_max[i]=300;
    state.A_low[i]=300;
    state.A_high[i]=300;
    state.A_lowfrac[i]=32;
    state.A_highfrac[i]=96;
    state.servo_high[i]=150;
    state.servo_total[i]=2000;
    initServo(i);
  }
  Serial.begin(115200);
  SlaveReg.onReceive(receiveEvent); // register event
  SlaveReg.onRequest(requestEvent); // register event
  SlaveReg.begin(0x55,regs);
/*
  Serial.print("WC: ");Serial.println((char*)&state.WC-regs,HEX);
  Serial.print("DT: ");Serial.println((char*)&state.DT-regs,HEX);
  Serial.print("T0: ");Serial.println((char*)&state.T0-regs,HEX);
  Serial.print("T1: ");Serial.println((char*)&state.T1-regs,HEX);
  Serial.print("ID: ");Serial.println((char*)&state.ID-regs,HEX);
  Serial.print("RST: ");Serial.println((char*)&state.RST-regs,HEX);
  Serial.print("A: ");Serial.println((char*)&state.A-regs,HEX);
  Serial.print("A_min: ");Serial.println((char*)&state.A_min-regs,HEX);
  Serial.print("A_max: ");Serial.println((char*)&state.A_max-regs,HEX);
  Serial.print("A_low: ");Serial.println((char*)&state.A_low-regs,HEX);
  Serial.print("A_high: ");Serial.println((char*)&state.A_high-regs,HEX);
  Serial.print("A_lowfrac: ");Serial.println((char*)&state.A_lowfrac-regs,HEX);
  Serial.print("A_highfrac: ");Serial.println((char*)&state.A_highfrac-regs,HEX);
  Serial.print("servo_high: ");Serial.println((char*)&state.servo_high-regs,HEX);
  Serial.print("servo_total: ");Serial.println((char*)&state.servo_total-regs,HEX);
  Serial.print("serial: ");Serial.println((char*)&state.serial-regs,HEX);
*/
}

void readSensors() {
  state.T0=micros();
  state.A[0]=analogRead(0);
  state.A[1]=analogRead(3);
  state.T1=micros();
}

void receiveEvent(int& regAddr) {
  Serial.print(regAddr,HEX);
  if(regAddr==0x34) {
    Serial.print(regs[regAddr]);         // print the character
  } else {
    regAddr++;
    if(regAddr==0x0C) regAddr=0;
  }
}

void requestEvent(int& regAddr) {
  Serial.print(regAddr,HEX);
  if(regAddr==0x34) {

  } else {
    regAddr++;
    if(regAddr==0x0C) regAddr=0;
  }
}

void figureThreshold(int i) {
  uint32_t range;
  range=state.A_max[i]-state.A_min[i];
  state.A_low [i]=range*state.A_lowfrac [i]/255+state.A_min[0];
  state.A_high[i]=range*state.A_highfrac[i]/255+state.A_min[0];
}

void figure() {
  for(int i=0;i<2;i++) {
    if(black[i] && state.A[i]<state.A_low[i]) {
      black[i]=false;
    } else if(!black[i] && state.A[i]>state.A_high[i]) {
      black[i]=true;
    }
    if(state.A_min[i]>state.A[i]) {
      state.A_min[i]=state.A[i];
      figureThreshold(i);
    }
    if(state.A_max[i]<state.A[i]) {
      state.A_max[i]=state.A[i];
      figureThreshold(i);
    }
  }
  digitalWrite(13,black[0]);
  int currentSector=grayCode[black[0]<<1 | black[1]];
  if(sector==3 && currentSector==0) {
    wheelCount++;
  } else if(sector==0 && currentSector==3) {
    wheelCount--;
  }
  if(sector!=currentSector) {
    T0[currentSector]=T1[currentSector];
    state.T0=T0[currentSector];
    T1[currentSector]=micros();
    state.T1=T1[currentSector];
    state.DT=T1[currentSector]-T0[currentSector];
  }
  sector=currentSector;
  state.WC=(wheelCount<<2)+sector;
}

void loop() {
//  readSensors();
//  figure();
  static uint16_t count=0;
  Serial.println(count++);
}

