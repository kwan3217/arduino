//Program Robodometer. Software which resides in the microcontroller 
//which manages the readout of an optical wheel encoder.
//
//This is a simple I2C slave. The system is presented as an address space
//of R/W registers, as follows
//
// Address   Name   format   Range
//    0        A0     U16     [0,1023]                   Analog Read 0
//    1        A1     U16     [0,1023]                   Analog Read 4
//    2        TS     U32     [0,0xFFFF'FFFF]            Time at start of read
//    3        TP     U32     [0,0xFFFF'FFFF]            Time at end of read
//    4        W*     I32     [-0x8000'0000-0x7FFF'FFFF] Count of wheel quarter-rotations
//    5        TW     U32     [0,0xFFFF'FFFF]            Time of last encoder click
//    6        DT     U32     [0,0xFFFF'FFFF]            Time between last two encoder clicks, proportional to inverse of speed
//    7        RST    U8      -                          Write to reset the integer part of the wheel rotation counter
// W is a bit special. The lower two bits indicate the current quadrant
//(in normal binary, not Gray code) and may start at any number in range [0,3]
//while the higher bits indicate the number of rotations since the encoder was
//reset.
// Wire protocol is byte-little-endian. Signed numbers use two's complement form.
//Hookup is with a pair of servo connections, as follows
//Encoder 0 - A0 signal
//            A1 power
//            A2 ground A2
//               power  A3 
//               signal A4 - Encoder 1
//
//There is also a pure digital GPIO interface - pins D7 and D8 represent the
//digital value of encoder A0 and A4 respectively. These pins are used because
//they have no alternate function, either I2C, SPI, UART, or analog.
#include <Wire.h>

uint32_t regs[]={0,0,0,0};

void setup() {
  pinMode(A0,INPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,INPUT);
  pinMode( 7,OUTPUT);
  pinMode( 8,OUTPUT);
  digitalWrite(A2,LOW);
  digitalWrite(A1,HIGH);
  digitalWrite(A3,HIGH);
  digitalWrite( 7,LOW);
  digitalWrite( 8,HIGH);
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


