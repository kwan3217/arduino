/*
  Jeopardy - uses three Big Red Buttons to run a game-show type game. 
  When the program is reset, it watches for which of the three buttons
  is pressed first. That one then has its light blinked a few times,
  then stays on. Buttons are reset by the reset switch on the Arduino.
 */
 

void setup() {                
  // initialize the digital pin as an output.
  pinMode(3,OUTPUT); //Button 1 light
  pinMode(4,INPUT);  //Button 1 switch
  pinMode(6,OUTPUT); //Button 2 light
  pinMode(7,INPUT);  //Button 2 switch
  pinMode(9,OUTPUT); //Button 3 light
  pinMode(10,INPUT); //Button 3 switch
  digitalWrite(4,HIGH);
  digitalWrite(7,HIGH);
  digitalWrite(10,HIGH);
}

void loop() {
  //Check each button in turn. If the button is pressed, blink the light, then leav it on.
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
