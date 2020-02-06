//Robodometer. See theory documentation at https://github.com/kwan3217/roboSim/wiki/Odometry

#include <WireSlaveReg.h>
#include <Servo.h>
#include "CircularBuffer.h"

struct tlmState {
  /* 0x00      */  int32_t WC;
  /* 0x04      */ uint32_t DT;
  /* 0x08      */ uint32_t T0;
  /* 0x0C      */ uint32_t cksum; //xor of WC, DT, T0, 0x32171836
  /* 0x10      */ uint32_t T1;
  /* 0x14      */ uint16_t ID;
  /* 0x16      */ uint16_t RST;
  /* 0x18,0x1A */ uint16_t A[2];
  /* 0x1C,0x1E */ uint16_t A_min[2];
  /* 0x20,0x22 */ uint16_t A_max[2];
  /* 0x24,0x26 */ uint16_t A_low[2];
  /* 0x28,0x2A */ uint16_t A_high[2];
  /* 0x2C,0x2D */ uint8_t  A_lowfrac[2];
  /* 0x2E,0x2F */ uint8_t  A_highfrac[2];
};

struct cmdState {
  /* 0x30      */ uint16_t steering;
  /* 0x32      */ uint16_t throttle;
  /* 0x34      */ uint16_t cksumServo; //xor of servo_high[1..0], servo_total[1..0], cksumServo
  /* 0x36      */ uint8_t  serial;
};

struct State {
  tlmState tlm;
  cmdState cmd;
};

const uint32_t cksumConst=0x32171836;
const uint16_t cksumServoConst=0x3217;

class Transaction {
private:
  CircularBuffer<bool,100> isMISO;
  CircularBuffer<uint32_t,100> time;
  CircularBuffer<int,100> addr;
  CircularBuffer<unsigned char,100> val;
public:
  bool push(bool LisMISO, uint32_t Ltime, int Laddr, unsigned char Lval) {isMISO.push(LisMISO);time.push(Ltime);addr.push(Laddr);return val.push(Lval);}
  void pop(bool& LisMISO, uint32_t& Ltime, int& Laddr, unsigned char& Lval) {LisMISO=isMISO.shift();Ltime=time.shift();Laddr=addr.shift();Lval=val.shift();}
  bool isEmpty() {return val.isEmpty();}
};

cmdState cmd;
tlmState tlm;
State regState;
unsigned char* regs=(unsigned char*)&regState;
const int serial_addr=&regState.cmd.serial-regs;
const int cmdcksum_addr=((unsigned char*)&regState.cmd.cksumServo)-regs;
Transaction trans;
Servo steering;
Servo throttle;
const int I2C_FLICKER_PIN=13;
const int E0_ECHO_PIN=9;
const int E1_ECHO_PIN=10;
const int SDA_PIN=A4;
const int SCL_PIN=A5;
const int E0_PIN=A6;
const int E1_PIN=A7;
const int STEERING_PIN=5;
const int THROTTLE_PIN=6;
int flicker0=0,flicker1=0;

bool black[2];
unsigned int sector;
int32_t wheelCount;
uint32_t T0[4]; //Time we entered the sector two clicks ago
uint32_t T1[4]; //Time we entered the sector last click

//Index: natural binary number
//Value: Gray code for that index
//Works as its own inverse IE grayCode[grayCode[i]]=i meaning it can be used to encode or decode
const int grayCode[4]={0,1,3,2};

void setup() {
  pinMode(E0_PIN,INPUT);
  pinMode(E1_PIN,INPUT);
  pinMode(SDA_PIN,INPUT);
  pinMode(SCL_PIN,INPUT);
  pinMode(I2C_FLICKER_PIN,OUTPUT);
  pinMode(E0_ECHO_PIN,OUTPUT);
  digitalWrite(E0_ECHO_PIN,flicker0);
  pinMode(E1_ECHO_PIN,OUTPUT);
  digitalWrite(E1_ECHO_PIN,flicker1);
  tlm.ID=0x3217;
  for(int i=0;i<2;i++) {
    black[i]=false;
    tlm.A_min[i]=900;
    tlm.A_max[i]=900;
    tlm.A_low[i]=900;
    tlm.A_high[i]=900;
    tlm.A_lowfrac[i]=32;
    tlm.A_highfrac[i]=96;
  }
  Serial.begin(115200);
  SlaveReg.onReceive(receiveEvent); // register event
  SlaveReg.onRequest(requestEvent); // register event
  SlaveReg.begin(0x55,(char*)regs);
  steering.attach(STEERING_PIN);
  throttle.attach(THROTTLE_PIN);
  cmd.steering=1500;
  steering.write(cmd.steering);
  cmd.throttle=1500;
  throttle.write(cmd.throttle);
}

void readSensors() {
  tlm.cksum=0xFFFFFFFF;
  tlm.T0=micros();
  tlm.A[0]=analogRead(E0_PIN);
  tlm.A[1]=analogRead(E1_PIN);
  tlm.T1=micros();
}

void receiveEvent(int& regAddr) {
  //Take note of receive
  flicker0=1-flicker0;
  digitalWrite(I2C_FLICKER_PIN,flicker0);
  trans.push(false,micros(),regAddr,regs[regAddr]);
  if(regAddr==cmdcksum_addr+1) {
    int16_t cksum_calc=regState.cmd.steering ^ regState.cmd.throttle ^ cksumServoConst;
    if(regState.cmd.cksumServo==cksum_calc) {
      cmd=regState.cmd;
      steering.write(cmd.steering);
      throttle.write(cmd.throttle);
    } else {
      Serial.print("Cmd check bad, is ");
      Serial.print(regState.cmd.cksumServo,HEX);
      Serial.print(" Should be ");
      Serial.print(cksum_calc,HEX);
    }
  }
  regAddr++;
  if(regAddr==0x10) regAddr=0;
}

void requestEvent(int& regAddr) {
  //Take note of request
  flicker1=1-flicker1;
  digitalWrite(I2C_FLICKER_PIN,flicker1);
  trans.push(true,micros(),regAddr,regs[regAddr]);
  
  regAddr++;
  if(regAddr==0x10) {
    regState.tlm=tlm;
    regState.tlm.cksum=tlm.WC ^ tlm.DT ^ tlm.T0 ^ cksumConst;
    regAddr=0;
  }
}

void figureThreshold(int i) {
  uint32_t range;
  range=tlm.A_max[i]-tlm.A_min[i];
  tlm.A_low [i]=range*tlm.A_lowfrac [i]/255+tlm.A_min[0];
  tlm.A_high[i]=range*tlm.A_highfrac[i]/255+tlm.A_min[0];
}

void figure() {
  for(int i=0;i<2;i++) {
    if(black[i] && tlm.A[i]<tlm.A_low[i]) {
      black[i]=false;
    } else if(!black[i] && tlm.A[i]>tlm.A_high[i]) {
      black[i]=true;
    }
    if(tlm.A_min[i]>tlm.A[i]) {
      tlm.A_min[i]=tlm.A[i];
      figureThreshold(i);
    }
    if(tlm.A_max[i]<tlm.A[i]) {
      tlm.A_max[i]=tlm.A[i];
      figureThreshold(i);
    }
  }
  digitalWrite(E0_ECHO_PIN,black[0]);
  digitalWrite(E1_ECHO_PIN,black[1]);
  int currentSector=grayCode[black[0]<<1 | black[1]];
  if(sector==3 && currentSector==0) {
    wheelCount++;
  } else if(sector==0 && currentSector==3) {
    wheelCount--;
  }
  if(sector!=currentSector) {
    T0[currentSector]=T1[currentSector];
    T1[currentSector]=micros();
    tlm.DT=T1[currentSector]-T0[currentSector];
    tlm.WC=(wheelCount<<2)+sector;
  }
  sector=currentSector;
  tlm.WC=(wheelCount<<2)+sector;
}

const long sec=1000000;

long t []={0*sec,2*sec,4*sec,6*sec,8*sec};  // Time that this command goes into effect in micros
long ss[]={ 1500, 1500, 1500, 1500, 1500};  // Raw steering command
long tt[]={ 1500, 1400, 1400, 1400, 1500};  // Raw throttle command

void drive() {
  int effectCmd=0;
  static int oldEffectCmd=0;
  long m=micros();
  for(int i=0;i<sizeof(t)/sizeof(t[0]);i++) {
    if(t[i]<m) effectCmd=i;
  }
  if(effectCmd!=oldEffectCmd) {
    Serial.print(" m="        );Serial.print(m        );
    Serial.print(" effectCmd=");Serial.print(effectCmd);
    Serial.print(" t[" );Serial.print(effectCmd);Serial.print("]=");Serial.print(t [effectCmd]);
    Serial.print(" ss[");Serial.print(effectCmd);Serial.print("]=");Serial.print(ss[effectCmd]);
    Serial.print(" tt[");Serial.print(effectCmd);Serial.print("]=");Serial.print(tt[effectCmd]);
    Serial.println();
    steering.write(ss[effectCmd]);
    throttle.write(tt[effectCmd]);
    oldEffectCmd=effectCmd;
  }
}

void loop() {
  readSensors();
  figure();
//  drive();
/*
  while(!trans.isEmpty()) {
    bool isMISO;
    uint32_t time;
    int addr;
    unsigned char val;
    trans.pop(isMISO,time,addr,val);
    Serial.print("M");
    Serial.print(isMISO?"<":">");
    Serial.print("S");
    if(addr<0x10) Serial.print('0');
    Serial.print(addr,HEX);
    Serial.print(',');
    if(val<0x10) Serial.print('0');
    Serial.println(val,HEX);
  }
  */
}

