//Program Kyoko. Software which resides in the microcontroller in the 
//"engine room" of the helicopter "Poison 3" of Project Kyoko.
//
//This is a simple I2C slave. The system is presented as an address space
//of R/W registers, as follows
//
// Address   Motor   format   Range
//    0        --     U16     [0,65535]   Used as a version indicator, ignores writes
//    1        M1     U16     [0,255]
//    2        M2     U16     [0,255]
//    3        MT     I16     [-255,255]
//    4      UART     string  Whatever is written here goes out the Arduino's UART
//    5      UART     U16     Number of bytes in UART Rx buffer
//    6      UART     string  Read the Arduino's RX UART
// Wire protocol is little-endian (I don't know what the chip really is)
#include <Wire.h>

#include "WProgram.h"
void setup();
void loop();
void receiveEvent(int howMany);
void spinMotors();
short regs[4];
int ptr=0;

void setup() {
  Wire.begin(0x55);                 // join i2c bus with address #4
  Wire.onReceive(receiveEvent);  // register event
//  Wire.onRequest(requestEvent);  // register the other event
  Serial.begin(9600);            // start serial for output
  pinMode(3,OUTPUT);
  pinMode(11,OUTPUT);
}

void loop() {
  //We are the for loop, that don't do anything...
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  if(howMany==1) {
    ptr=0;
  } else {
    ptr=Wire.receive();
    if(ptr>0 && ptr<4) {
      while(0 < Wire.available()) { 
        regs[ptr]=Wire.receive();
        regs[ptr]|=Wire.receive()<<8;
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
}

void spinMotors() {
  analogWrite(10,regs[1]);
  analogWrite(9,regs[2]);
  if(regs[3]==0) {
    digitalWrite(3,LOW);
    digitalWrite(11,LOW);
  } else if (regs[3]>0) {
    digitalWrite(3,LOW);
    analogWrite(11,regs[3]);
  } else {
    digitalWrite(11,LOW);
    analogWrite(3,-regs[3]);
  }
}



int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

