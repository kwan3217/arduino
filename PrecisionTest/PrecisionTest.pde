#define VERBOSE
#include <PCharlie.h>

void setup() {
  Serial.begin(9600);  
}

int oldi=0;

void loop() {
  Serial.println("Enter a light number: ");
  int i=0, b=0;
  while(b!=',' && b!=' ') {
    while(!Serial.available());b=Serial.read();
    if(b>='0' and b<='9') i=i*10+(b-'0');
    if(b==' ') i=oldi+1;
    if(i%100>59) i=((i/100)+1)*100;
    if(i>=400) i=i % 100;
    Serial.println(b,DEC);
  }
  Serial.print("Switching on light ");
  Serial.println(i,DEC);
  light(i);
  oldi=i;
}
