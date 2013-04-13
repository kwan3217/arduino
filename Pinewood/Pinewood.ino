/*
  Pinewood
  
  Uses a pair of IR sensors to measure the turns of a wheel of a pinewood
  derby car and thereby measure its speed and acceleration curve
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
  Serial.write(124);
  Serial.write(128);
}

int wheelrev=0;
bool aft,oldaft=false;
bool fwd,oldfwd=false;
unsigned long t,old_t=0,t0=0;
float wheelRad=0.015; //wheel radius in m, 1.5cm
float wheelCirc=wheelRad*2*PI;
float dist=0;
float spd=0,maxspd=0;
long check=0;

int wheelTurnMS[200];

//returns 0 for no motion, +1 for one forward turn, -1 for one backward turn
//forward is defined as the aft sensor white and the fwd sensor changing from black to white
//backward is defined as the aft sensor white and the fwd sensor changing from black to white
int checkSensor() {
  int result=0;
//  check++;
  aft=(1023-analogRead(5))>900;
  fwd=(1023-analogRead(0))>900;
  if (fwd!=oldfwd) {
    t=micros();
    if(t0==0) t0=t;
    if(aft) {
      result=(fwd?1:-1);
      wheelrev+=result;
      if(fwd) wheelTurnMS[wheelrev]=(t-t0)/1000;
      dist=wheelrev*wheelCirc;
      if(old_t!=0) {
        spd=wheelCirc/(((float)(t-old_t))/1e6);
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
    return result;
  }
  return result;
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
  int result=checkSensor();
  if (result==1) {
    printCheck(254); 
    printCheck(1);
    printCheck("R");
    printCheck(wheelrev,4);
    printCheck("T");
    dtostrf(((float)(t-t0))/1e6,5,2,buf);
    printCheck(buf);
//    sprintf(buf,"%d",check);
//    printCheck(buf);
    printCheck(254); 
    printCheck(192);
    printCheck("S");
    dtostrf(spd*2.23694,4,1,buf); //Print speed in mph
    printCheck(buf);
    printCheck("M");
    dtostrf(maxspd*2.23694,4,1,buf); //Print max speed in mph
    printCheck(buf);
    printCheck("D");
    dtostrf(dist/0.3048,5,2,buf); //Print distance in feet
    printCheck(buf);
  } else if(result==-1) {
    int i=wheelrev+1;
    if(i<0)i=199;
    if(i>199)i=i-200;
    printCheck(254); 
    printCheck(1);
    printCheck("RM");
    printCheck(i,4);
    printCheck("T");
    dtostrf(((float)(wheelTurnMS[i]))/1e3,6,3,buf);
    printCheck(buf);
  }
}
