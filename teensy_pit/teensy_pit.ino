//volatile uint32_t& PIT_LTMR64H=*((uint32_t*)0x40037000+0xE0);
//volatile uint32_t& PIT_LTMR64L=*((uint32_t*)0x40037000+0xE4);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(300);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  SIM_SCGC6|=SIM_SCGC6_PIT;
  PIT_MCR&=~PIT_MCR_MDIS;
  uint32_t pit_mcr=PIT_MCR;
  digitalWrite(13,HIGH);
  Serial.print("PIT_MCR:     ");Serial.println(pit_mcr,HEX);
//  Serial.print("PIT_LTMR64H: ");Serial.print(PIT_LTMR64H,HEX);
//  Serial.print("PIT_LTMR64L: ");Serial.print(PIT_LTMR64L,HEX);
  
  Serial.print("PIT_LDVAL0:  ");Serial.println(PIT_LDVAL0,HEX);
  Serial.print("PIT_CVAL0:   ");Serial.println(PIT_CVAL0,HEX);
  Serial.print("PIT_TCTRL0:  ");Serial.println(PIT_TCTRL0,HEX);

  Serial.print("PIT_LDVAL1:  ");Serial.println(PIT_LDVAL1,HEX);
  Serial.print("PIT_CVAL1:   ");Serial.println(PIT_CVAL1,HEX);
  Serial.print("PIT_TCTRL1:  ");Serial.println(PIT_TCTRL1,HEX);

  Serial.print("PIT_LDVAL2:  ");Serial.println(PIT_LDVAL2,HEX);
  Serial.print("PIT_CVAL2:   ");Serial.println(PIT_CVAL2,HEX);
  Serial.print("PIT_TCTRL2:  ");Serial.println(PIT_TCTRL2,HEX);

  Serial.print("PIT_LDVAL3:  ");Serial.println(PIT_LDVAL3,HEX);
  Serial.print("PIT_CVAL3:   ");Serial.println(PIT_CVAL3,HEX);
  Serial.print("PIT_TCTRL3:  ");Serial.println(PIT_TCTRL3,HEX);

}

void loop() {
  // put your main code here, to run repeatedly:
}
