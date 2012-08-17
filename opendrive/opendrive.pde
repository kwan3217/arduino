// Sweep
// by BARRAGAN <http://barraganstudio.com>
// This example code is in the public domain.


#include <Servo.h>
 
Servo drive,steer;  // create servo object to control a servo
              // a maximum of eight servo objects can be created
 
int pos = 0;    // variable to store the servo position
 
void setup()
{
  drive.attach(3,1000,2000);  // attaches the servo on pin 9 to the servo object
  steer.attach(9,1000,2000);  // attaches the servo on pin 9 to the servo object
  drive.write(90);
  steer.write(90);
}
 
 
void loop() {
  delay(3000);
  drive.write(80);
  delay(2000);
  drive.write(80);
  steer.write(175);
  delay(1000);
  steer.write(90);
  drive.write(80);
  delay(2000);
  drive.write(90);
  for(;;);
}

