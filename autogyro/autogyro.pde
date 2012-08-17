//Program Kyoko. Software which resides in the microcontroller in the 
//"engine room" of the helicopter "Poison 3" of Project Arisa.
//
//This is a simple I2C slave. The system is presented as an address space
//of R/W registers, as follows
//
// Address   Motor   format   Range
//    0        --     U16     [0,65535]   Used as a version indicator, ignores writes
//    1        M1     U16     [0,255]     Rotor A (top) motor
//    2        M2     U16     [0,255]     Rotor B (bottom) motor
//    3        MT     I16     [-255,255]  Tail rotor motor
//    4      UART     string  Whatever is written here goes out the Arduino's UART
//    5      UART     U16     Number of bytes in UART Rx buffer
//    6      UART     string  Read the Arduino's RX UART
// Wire protocol is little-endian (I don't know what the chip really is)
#include <Wire.h>

void setup() {
  pinMode(3,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(9,OUTPUT);
  spinTail(0);
  spinB(0);
  spinA(0);
}

void loop() {
  spinTail(0);
  spinB(0);
  spinA(0);
  delay(10000);
  
  for(int i=0;i<256;i++) {
    spinA(i);
    spinB(i);
    delay(100);
    digitalWrite(13,(i/10) & 1);
  }
  delay(1000);
  for(int i=255;i>=0;i--) {
    spinA(i);
    spinB(i);
    delay(10);
    digitalWrite(13,(i/10) & 1);
  }
  digitalWrite(13,LOW);
  for(;;) delay(5000);
}

void spinA(int spd) {
  analogWrite(3,spd);
}

void spinB(int spd) {
  analogWrite(9,spd);
}

void spinTail(int spd) {
  if(spd==0) {
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
  } else if (spd>0) {
    digitalWrite(5,LOW);
    analogWrite(6,spd);
  } else {
    analogWrite(5,-spd);
    digitalWrite(6,LOW);
  }
}


