//Program Kyoko. Software which resides in the microcontroller in the 
//"engine room" of the helicopter "Poison 3" of Project Kyoko.
//
//This is a simple I2C slave. The system is presented as an address space
//of R/W registers, as follows
//
// Address   Motor   format   Range
//    0        --     U16     [0,65535]   Used as a version indicator, ignores writes
//    1        MA     U16     [0,255]
//    2        MB     U16     [0,255]
//    3        MT     I16     [-255,255]
//    4      UART     string  Whatever is written here goes out the Arduino's UART (Not active)
//    5      UART     U16     Number of bytes in UART Rx buffer                    (Not active)
//    6      UART     string  Read the Arduino's RX UART                           (Not active)
// Wire protocol is little-endian (I don't know what the chip really is)
#include <Wire.h>

unsigned char regs[]={0,0,0,0};
int ptr=0;
int blink=0;

void setup() {
  Wire.begin(0x55);                 // join i2c bus 
  Wire.onReceive(receiveEvent);  // register event
  pinMode(3,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(13,OUTPUT);
  spinTail(0);
  spinB(0);
  spinA(0);
}

void loop() {
  //We are the for loop, that don't do anything...
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  digitalWrite(13,blink);
  blink=1-blink;

  ptr=Wire.receive();
  if(ptr>0 && ptr<4) {
    ptr=0;
    while(0 < Wire.available()) { 
      regs[ptr]=Wire.receive();
      ptr++;
      if(ptr==4) ptr=3;
    }
    spinMotors();
  } else if(ptr==4) {
    while(0 < Wire.available()) { // loop through all but the last
      char c = Wire.receive();   // receive byte as a character
      Serial.print(c);           // print the character
    }
  } else {
    while(0 < Wire.available()) { // loop through all but the last
      char junk = Wire.receive();   // receive byte as a character
    }
  }
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

void spinMotors() {
  spinA(regs[0]);
  spinB(regs[1]);
  spinTail((((int)regs[2]))-((int)(regs[3])));
}


