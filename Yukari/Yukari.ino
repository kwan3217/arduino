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
//#include <Wire.h>
#include <Servo.h>
 
Servo drive,steer;  // create servo object to control a servo
              // a maximum of eight servo objects can be created
 
int pos = 0;    // variable to store the servo position
int driveScale=20;
int steerScale=60;
int steerAngle=0; 
int steerDir=5;

void setup()
{
//  Wire.begin(0x55);                 // join i2c bus 
//  Wire.onReceive(receiveEvent);  // register event
  drive.attach(3,1000,2000);  
  steer.attach(9,1000,2000);  
  drive.write(90);
  steer.write(90);
  delay(5000);
}
 
 
int ptr=0;
int blink=0;

void loop() {
  spinDrive(12);
  spinSteer(0);
  delay(2500);
  spinSteer(900);
  delay(1000);
  spinSteer(0);
  delay(500);
  spinDrive(0);
  for(;;);
}

void spinDrive(int spd) {
  drive.write(90-spd);
}

void spinSteer(int angle) {
  steer.write(90+angle/10);
}


