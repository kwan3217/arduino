/*
  Pinewood
  
  Uses a pair of IR sensors to measure the turns of a wheel of a pinewood
  derby car and thereby measure its speed and acceleration curve
 */

void setup() {                
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  //VCC - powers LED and puts voltage across phototransistor
  pinMode(13,OUTPUT);digitalWrite(13,HIGH);pinMode(A3,OUTPUT);digitalWrite(A3,HIGH);
  //GND - for LED and phototransistor 
  pinMode(A0,OUTPUT);digitalWrite(A0,LOW );pinMode(A4,OUTPUT);digitalWrite(A4,LOW );
  //AIN - photodiode readout
  pinMode(A1,INPUT );                      pinMode(A5,INPUT );
  //Blank terminal - must be input, which is most HiZ - effectively disconnected
  pinMode(A2,INPUT );                      pinMode(A6,INPUT );
  Serial.begin(115200);
}

void loop() {
  long t=micros();
  int aft=analogRead(A1);
  int fwd=analogRead(A5);
  Serial.print("\nT: ");
  Serial.print(t);
  Serial.print(" Aft: ");
  Serial.print(aft);
  Serial.print(" Fwd: ");
  Serial.print(fwd);
}
