/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,INPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,INPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,INPUT);
  digitalWrite(4,HIGH);
  digitalWrite(7,HIGH);
  digitalWrite(10,HIGH);
}

// the loop routine runs over and over again forever:
void loop() {
  if(digitalRead(4)) {
    for(int i=0;i<3;i++) {
      digitalWrite(3, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);
      digitalWrite(3, LOW);   // turn the LED on (HIGH is the voltage level)
      delay(100);
    }
    digitalWrite(3, HIGH);   // turn the LED on (HIGH is the voltage level)
    for(;;);
  }
  if(digitalRead(7)) {
    for(int i=0;i<3;i++) {
      digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);
      digitalWrite(6, LOW);   // turn the LED on (HIGH is the voltage level)
      delay(100);
    }
    digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
    for(;;);
  }
  if(digitalRead(10)) {
    for(int i=0;i<3;i++) {
      digitalWrite(9, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);
      digitalWrite(9, LOW);   // turn the LED on (HIGH is the voltage level)
      delay(100);
    }
    digitalWrite(9, HIGH);   // turn the LED on (HIGH is the voltage level)
    for(;;);
  }
}
