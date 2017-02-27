//PCharlie.cpp
#include "PCharlie.h"

bool PCharlieVerbose=false;
#define A 0
#define B 1
#define C 2
#define D 3

//ATMega PORTbn addresses of all P charlieplex signals
                                 //1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16
static const signed char PB[]={-1, B, D, B, B, B, D, D, D, D, C, C, C, C, C, C, B};
static const signed char PN[]={-1, 3, 4, 2, 1, 0, 7, 6, 5, 3, 5, 4, 3, 2, 1, 0, 5};

static const signed char P[]=
                      //  1 2  3 4 5 6 7 8 9 10 11 12 13 14 15 16
                     {-1,11,4,10,9,8,7,6,5,3,A5,A4,A3,A2,A1,A0,13};
static const signed char KK1[][60]=
                          // x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  
                           {{ 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  //00x
                             11, 12, 13, 14, 15,  1,  2,  3,  4,  5,  //01x
                              6,  7,  8,  9, 10, 11, 12, 13, 14,  1,  //02x
                              2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  //03x
                             12, 13,  1,  2,  3,  4,  5,  6,  7,  8,  //04x
                              9, 10, 11, 12,  1,  2,  3,  4,  5,  6}, //05x
                         {    7,  8,  9, 10, 11,  1,  2,  3,  4,  5,  //10x
                              6,  7,  8,  9, 10,  1,  2,  3,  4,  5,  //11x
                              6,  7,  8,  9,  1,  2,  3,  4,  5,  6,  //12x
                              7,  8,  1,  2,  3,  4,  5,  6,  7,  1,  //13x
                              2,  3,  4,  5,  6,  1,  2,  3,  4,  5,  //14x
                              1,  2,  3,  4,  1,  2,  3,  1,  2,  1}};//15x
static const signed char AA1[][60]=
                          // x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  
                           {{ 2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  //00x
                             12, 13, 14, 15, 16,  3,  4,  5,  6,  7,  //01x
                              8,  9, 10, 11, 12, 13, 14, 15, 16,  4,  //02x
                              5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  //03x
                             15, 16,  5,  6,  7,  8,  9, 10, 11, 12,  //04x
                             13, 14, 15, 16,  6,  7,  8,  9, 10, 11}, //05x
                            {12, 13, 14, 15, 16,  7,  8,  9, 10, 11,  //10x
                             12, 13, 14, 15, 16,  8,  9, 10, 11, 12,  //11x
                             13, 14, 15, 16,  9, 10, 11, 12, 13, 14,  //12x
                             15, 16, 10, 11, 12, 13, 14, 15, 16, 11,  //13x
                             12, 13, 14, 15, 16, 12, 13, 14, 15, 16,  //14x
                             13, 14, 15, 16, 14, 15, 16, 15, 16, 16}};//15x

signed char pinA,pinK;

void allDark() {
  pinA=-1;pinK=-1;
  for(int i=1;i<=16;i++) pinMode(P[i],INPUT);
}  

void dark() {
  if(pinA==0) return;
  pinMode(pinA,INPUT);
  pinMode(pinK,INPUT);
  pinA=-1;pinK=-1;
}

void light(int x) {
  int h=x / 100;
  int m=x % 100;
  int dir=h / 2;
  int bank=h % 2;
  dark();
  int PA=AA1[bank][m];
  pinA=P[PA];
  int PK=KK1[bank][m];
  pinK=P[PK];
  pinMode(pinA,OUTPUT); digitalWrite(pinA,dir?LOW:HIGH);
  pinMode(pinK,OUTPUT); digitalWrite(pinK,dir?HIGH:LOW);
  if(PCharlieVerbose) {
    Serial.print("Dir ");
    Serial.print(dir,DEC);
    Serial.print(" Bank ");
    Serial.println(bank,DEC);
    Serial.print("Setting P");
    Serial.print(PA,DEC);
    Serial.print(pinA>=A0?" (A":" (D");
    Serial.print(pinA>=A0?(pinA-A0):pinA,DEC);
    Serial.print(") to ");
    Serial.println(dir?"lo":"HI");
    Serial.print("Setting P");
    Serial.print(PK,DEC);
    Serial.print(pinK>=A0?" (A":" (D");
    Serial.print(pinK>=A0?(pinK-A0):pinK,DEC);
    Serial.print(") to ");
    Serial.println(dir?"HI":"lo");
  }
}  


