//Program Kyoko. Software which resides in the microcontroller in the 
//"engine room" of the helicopter "Poison 3" of Project Kyoko.
//
//This is a simple I2C slave. The system is presented as an address space
//of R/W registers, as follows
//
// Address   Name   format   Range
//    0        A0     U16     [0,1023]        Analog Read 0
//    1        A1     U16     [0,1023]        Analog Read 4
//    2        TS     U32     [0,0xFFFF'FFFF] Time at start of read
//    3        TP     U32     [0,0xFFFF'FFFF] Time at end of read
// Wire protocol is byte-little-endian
//Hookup is with a pair of servo connections, as follows
//Encoder 0 - A0 signal
//            A1 power
//            A2 ground A2
//               power  A3 
//               signal A4 - Encoder 1
#include <Wire.h>

uint32_t regs[]={0,0,0,0};

void setup() {
  pinMode(A0,INPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,INPUT);
  digitalWrite(A2,LOW);
  digitalWrite(A1,HIGH);
  digitalWrite(A3,HIGH);
  Serial.begin(115200);
  Serial.println("Tstart,A0,A1,Tstop");
}

void loop() {
  regs[2]=micros();
  regs[0]=analogRead(0);
  regs[1]=analogRead(4);
  regs[3]=micros();
  Serial.print(regs[2]);
  Serial.print(',');
  Serial.print(regs[0]);
  Serial.print(',');
  Serial.print(regs[1]);
  Serial.print(',');
  Serial.println(regs[3]);
}


