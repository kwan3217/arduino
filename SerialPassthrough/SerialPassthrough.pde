void setup() {
  pinMode(12,INPUT);
  pinMode(13,OUTPUT);
  delay(1000);
  Serial.begin(9600);
  delay(1000);
  Serial.println("$PMTK314,1,1,1,1,1,5,1,1,1,1,1,1,0,1,1,1,1,1,1*2C");
  delay(10000);
  digitalWrite(13,HIGH);
  Serial.end();
  pinMode(1,INPUT);
  pinMode(0,INPUT);
}

int last12=0;

void loop() {
  int this12=digitalRead(12);
  if(this12!=last12) {
    last12=this12;
    digitalWrite(13,this12);
  }
  
}
