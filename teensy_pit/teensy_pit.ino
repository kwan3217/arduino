#include <pit.h>

//volatile uint32_t& PIT_LTMR64H=*((uint32_t*)0x40037000+0xE0);
//volatile uint32_t& PIT_LTMR64L=*((uint32_t*)0x40037000+0xE4);

PIT pit0=PIT(0);

void setup() {
  //Start up pit as early as possible
  pit0.begin(5.0);
  pit0.attachInterrupt(tock);
  Serial.begin(115200);
  delay(300);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  Serial.print("PIT_MCR:   0x");Serial.println(PIT_MCR,HEX);
//  Serial.print("PIT_LTMR64H: ");Serial.print(PIT_LTMR64H,HEX);
//  Serial.print("PIT_LTMR64L: ");Serial.print(PIT_LTMR64L,HEX);
  
  Serial.print("PIT_LDVAL0:  (0x");Serial.print((uint32_t)&PIT_LDVAL0,HEX);Serial.print(") ");Serial.println(PIT_LDVAL0);
  Serial.print("PIT_CVAL0:   (0x");Serial.print((uint32_t)&PIT_CVAL0 ,HEX);Serial.print(") ");Serial.println(PIT_CVAL0);
  Serial.print("PIT_TCTRL0:  (0x");Serial.print((uint32_t)&PIT_TCTRL0,HEX);Serial.print(") 0x");Serial.println(PIT_TCTRL0,HEX);
  Serial.print("PIT_TFLG0:   (0x");Serial.print((uint32_t)&PIT_TFLG0 ,HEX);Serial.print(") 0x");Serial.println(PIT_TFLG0,HEX);
  Serial.println();
  Serial.print("PIT_LDVAL1:  (0x");Serial.print((uint32_t)&PIT_LDVAL1,HEX);Serial.print(") ");Serial.println(PIT_LDVAL1);
  Serial.print("PIT_CVAL1:   (0x");Serial.print((uint32_t)&PIT_CVAL1 ,HEX);Serial.print(") ");Serial.println(PIT_CVAL1);
  Serial.print("PIT_TCTRL1:  (0x");Serial.print((uint32_t)&PIT_TCTRL1,HEX);Serial.print(") 0x");Serial.println(PIT_TCTRL1,HEX);
  Serial.print("PIT_TFLG1:   (0x");Serial.print((uint32_t)&PIT_TFLG1 ,HEX);Serial.print(") 0x");Serial.println(PIT_TFLG1,HEX);
  Serial.println();
  Serial.print("PIT_LDVAL2:  (0x");Serial.print((uint32_t)&PIT_LDVAL2,HEX);Serial.print(") ");Serial.println(PIT_LDVAL2);
  Serial.print("PIT_CVAL2:   (0x");Serial.print((uint32_t)&PIT_CVAL2 ,HEX);Serial.print(") ");Serial.println(PIT_CVAL2);
  Serial.print("PIT_TCTRL2:  (0x");Serial.print((uint32_t)&PIT_TCTRL2,HEX);Serial.print(") 0x");Serial.println(PIT_TCTRL2,HEX);
  Serial.print("PIT_TFLG2:   (0x");Serial.print((uint32_t)&PIT_TFLG2 ,HEX);Serial.print(") 0x");Serial.println(PIT_TFLG2,HEX);
  Serial.println();
  Serial.print("PIT_LDVAL3:  (0x");Serial.print((uint32_t)&PIT_LDVAL3,HEX);Serial.print(") ");Serial.println(PIT_LDVAL3);
  Serial.print("PIT_CVAL3:   (0x");Serial.print((uint32_t)&PIT_CVAL3 ,HEX);Serial.print(") ");Serial.println(PIT_CVAL3);
  Serial.print("PIT_TCTRL3:  (0x");Serial.print((uint32_t)&PIT_TCTRL3,HEX);Serial.print(") 0x");Serial.println(PIT_TCTRL3,HEX);
  Serial.print("PIT_TFLG3:   (0x");Serial.print((uint32_t)&PIT_TFLG3 ,HEX);Serial.print(") 0x");Serial.println(PIT_TFLG3,HEX);
  Serial.println();
  Serial.print("Nominal PIT frequency: ");Serial.println(PIT::Hz());
  Serial.println();
  //Serial.print("pit0.LDVAL(): (0x");Serial.print((uint32_t)&pit0.LDVAL(),HEX);Serial.print(") ");Serial.println(pit0.LDVAL());
  //Serial.print("pit0.CVAL (): (0x");Serial.print((uint32_t)&pit0.CVAL (),HEX);Serial.print(") ");Serial.println(pit0.CVAL ());
  //Serial.print("pit0.TCTRL(): (0x");Serial.print((uint32_t)&pit0.TCTRL(),HEX);Serial.print(") ");Serial.println(pit0.TCTRL());
  //Serial.print("pit0.TFLG (): (0x");Serial.print((uint32_t)&pit0.TFLG (),HEX);Serial.print(") ");Serial.println(pit0.TFLG ());
}

volatile bool tockFlag=false;
void tock() {
  digitalWrite(13,HIGH);
  tockFlag=true;
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  auto u=micros();
  auto ticks=(float)(pit0.TC())/(float)(PIT::Hz());
  Serial.print("micros: ");
  Serial.println(u);
  Serial.print("ticks:  ");
  Serial.print(tockFlag);
  Serial.println(ticks);
  if(tockFlag) {
    tockFlag=false;
    Serial.println("Tock!");
  }
}
