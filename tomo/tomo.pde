//Program Tomo. Software which resides in the microcontroller in the 
//"engine room" of the ground vehicle Yukari
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
#include <Servo.h>
 
Servo drive,steer;  // create servo object to control a servo
              // a maximum of eight servo objects can be created
 
int pos = 0;    // variable to store the servo position
int driveScale=20;
int steerScale=60;
 
void setup()
{
  Wire.begin(0x55);                 // join i2c bus 
  Wire.onReceive(receiveEvent);  // register event
  drive.attach(3,1000,2000);  // attaches the servo on pin 9 to the servo object
  steer.attach(9,1000,2000);  // attaches the servo on pin 9 to the servo object
  drive.write(90);
  steer.write(90);
}
 
 
unsigned char regs[]={0,0,0,0};
int ptr=0;
int blink=0;

void loop() {
  spinMotors();
  delay(20);
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
  }
}

void spinDrive(int spd) {
  drive.write(90-spd);
}

void spinSteer(int angle) {
  steer.write(90+angle);
}

void spinMotors() {
  spinDrive((((int)regs[0]))-((int)(regs[1])));
  spinSteer((((int)regs[2]))-((int)(regs[3])));
}


