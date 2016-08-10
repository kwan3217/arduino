#include <WireSlaveReg.h>

//Robodometer. See theory documentation at https://github.com/kwan3217/Yukari4Python/wiki/Odometry

struct State {
  /* 0x00      */ int32_t  WC __attribute((packed));
  /* 0x04      */ uint32_t DT __attribute((packed));
  /* 0x08      */ uint32_t T0 __attribute((packed));
  /* 0x0C      */ uint32_t T1 __attribute((packed));
  /* 0x10      */ uint16_t ID __attribute((packed));
  /* 0x12      */ uint16_t RST __attribute((packed));
  /* 0x14,0x16 */ uint16_t A[2] __attribute((packed));
  /* 0x18,0x1A */ uint16_t A_min[2] __attribute((packed));
  /* 0x1C,0x1E */ uint16_t A_max[2] __attribute((packed));
  /* 0x20,0x22 */ uint16_t A_low[2] __attribute((packed));
  /* 0x24,0x26 */ uint16_t A_high[2] __attribute((packed));
  /* 0x28,0x29 */ uint8_t  A_lowfrac[2] __attribute((packed));
  /* 0x2A,0x2B */ uint8_t  A_highfrac[2] __attribute((packed));
  /* 0x2C,0x2E */ uint16_t servo_high[2] __attribute((packed));
  /* 0x30,0x32 */ uint16_t servo_total[2] __attribute((packed));
};

State state;

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
  pinMode(A3,OUTPUT);
  pinMode(A4,INPUT);
  pinMode(13,OUTPUT);
  digitalWrite(A2,LOW);
  digitalWrite(A1,HIGH);
  digitalWrite(A3,HIGH);
  digitalWrite(13,LOW);
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
  Serial.begin(38400);
  Serial.println("Tstart,A0,A0min,A0max,A0low,A0high,black0,A1,A1min,A1max,A1low,A1high,black1,sector");
}

void readSensors() {
  state.T0=micros();
  state.A[0]=analogRead(0);
  state.A[1]=analogRead(4);
  state.T1=micros();
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
  sector=currentSector;
  state.WC=(wheelCount<<2)+sector;
}

void printState() {
  Serial.print(state.T0);
  Serial.print(',');
  for(int i=0;i<2;i++) {
    Serial.print(state.A[i]);
    Serial.print(',');
    Serial.print(state.A_min[i]);
    Serial.print(',');
    Serial.print(state.A_max[i]);
    Serial.print(',');
    Serial.print(state.A_low[i]);
    Serial.print(',');
    Serial.print(state.A_high[i]);
    Serial.print(',');
    Serial.print(black[i]);
    Serial.print(',');
  }
  Serial.print(wheelCount);
  Serial.print(',');
  Serial.print(sector);
  Serial.print(',');
  Serial.print(state.WC);
  Serial.println();
}

void loop() {
  readSensors();
  figure();
  printState();
}

