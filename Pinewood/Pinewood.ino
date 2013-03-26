/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */

void setup() {                
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode( 8,OUTPUT);pinMode(3,OUTPUT);
  pinMode( 9,OUTPUT);pinMode(4,OUTPUT);
  pinMode(10,INPUT );pinMode(5,INPUT );
  pinMode(11,INPUT );pinMode(6,INPUT );
  pinMode(12,OUTPUT);pinMode(7,OUTPUT);
  digitalWrite( 8,HIGH);digitalWrite(3,HIGH);
  digitalWrite( 9,LOW );digitalWrite(4,LOW );
  digitalWrite(12,LOW );digitalWrite(7,LOW );
  Serial.begin(9600);
//  Serial.write(124);
//  Serial.write(128);
}

int wheelrev=0;
bool aft,oldaft=false;
bool fwd,oldfwd=false;
unsigned long t,old_t=0,t0=0;
float wheelRad=0.015; //wheel radius in m, 1.5cm
float wheelCirc=wheelRad*2*PI;
float dist=0;
float spd=0,maxspd=0;
bool checkSensor() {
  aft=(1023-analogRead(5))>900;
  fwd=(1023-analogRead(0))>900;
  if (fwd!=oldfwd) {
    t=micros();
    if(t0==0) t0=t;
    if(aft) {
      wheelrev+=(fwd?1:-1);
      dist=wheelrev*wheelCirc;
      if(old_t!=0) {
        spd=wheelCirc/((float)(t-old_t)/1e6);
        if(maxspd<spd) maxspd=spd;
      } else {
        old_t=t;
      }
    }
    if(wheelrev>999) wheelrev-=1000;
    if(wheelrev<0) wheelrev+=1000;
    oldaft=aft;
    oldfwd=fwd;
    old_t=t;
    return true;
  }
  return false;
}

void printCheck(char b) {
  Serial.write(b);
  checkSensor();
}

void printCheck(char* b) {
  while(*b) {
    printCheck(*b);
    b++;
  }
}

void printCheck(int b, int digits) {
  if(b< 100) printCheck(' '); else printCheck('0'+(b/ 100)%10);
  if(b<  10) printCheck(' '); else printCheck('0'+(b/  10)%10);
  printCheck('0'+b%10);
}
char buf[16];
void loop() {
  if (checkSensor()) {
    printCheck(254); 
    printCheck(128);
    printCheck("R");
    printCheck(wheelrev,4);
    printCheck(" T");
    dtostrf(((float)(t-t0))/1e6,5,2,buf);
    printCheck(buf);
    printCheck(254); 
    printCheck(192);
    printCheck("S");
    dtostrf(spd*2.23694,4,1,buf);
    printCheck(buf);
    printCheck(" M");
    dtostrf(maxspd*2.23694,4,1,buf);
    printCheck(buf);
    printCheck(" D");
    dtostrf(dist,5,2,buf);
    printCheck(buf);
  }
}
