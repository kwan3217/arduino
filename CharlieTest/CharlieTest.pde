void setup() {
  Serial.begin(9600);  
}
         //  1 2  3 4 5 6 7 8 9 10 11 12 13 14 15 16
int P[]={-1,A6,4,10,9,8,7,6,5,3,A5,A4,A3,A2,A1,A0,A7};
             
void light1xx(int x, int dir) {
  for(int i=1;i<=16;i++) pinMode(P[i],INPUT);
  Serial.print("Setting P");
  Serial.print(x,DEC);
  Serial.print(P[x]<A0?" (D":" (A");
  Serial.print(P[x]<A0?P[x]:(P[x]-A0),DEC);
  Serial.print(") to ");
  Serial.println(dir?"HI":"lo");
  pinMode(P[x],OUTPUT); digitalWrite(P[x],dir?HIGH:LOW);
}  

void loop() {
  Serial.println("Enter a signal number: ");
  int i=0, b=0;
  while(b!='h' & b!='l') {
    while(!Serial.available());b=Serial.read();
    if(b>='0' and b<='9') i=i*10+(b-'0');
    Serial.println(b,DEC);
  }
  Serial.print("Switching on light ");
  Serial.println(i,DEC);
  light1xx(i,b=='h');
}
