const int PICPOWER=12;
const int ICSPDAT=13;
const int ICSPCLK=2;
const int spd=50;

void sendMCHP() {
  pinMode(ICSPDAT,OUTPUT);
  const char entryCode[]="MCHP";
  unsigned long entry;
  for(int i=0;i<4;i++) {
    entry=((entry << 8) | (int)entryCode[i]);
  }
  Serial.print("Sending program entry code:");
  Serial.println(entry,HEX);
  for(int i=0;i<33;i++) {
    digitalWrite(ICSPDAT,entry & 1);
    digitalWrite(ICSPCLK,1);
    delay(spd);
    digitalWrite(ICSPCLK,0);
    delay(spd);
    entry=entry >> 1;
  }
}

void sendCmd(unsigned char cmd, unsigned int data) {
  Serial.print("Cmd: ");
  Serial.print(cmd,HEX);
  Serial.print(" Data: ");
  Serial.println(data,HEX);
  pinMode(ICSPDAT,OUTPUT);
  for(int i=0;i<6;i++) {
    digitalWrite(ICSPDAT,cmd & 1);
    digitalWrite(ICSPCLK,1);
    delay(spd);
    digitalWrite(ICSPCLK,0);
    delay(spd);
    cmd=cmd >> 1;
  }
  //14 bits of data, shifted such that the start and stop bits are in place
  data=(data << 1) & 0x7FFF;
  for(int i=0;i<16;i++) {
    digitalWrite(ICSPDAT,data & 1);
    digitalWrite(ICSPCLK,1);
    delay(spd);
    digitalWrite(ICSPCLK,0);
    delay(spd);
    data=data >> 1;
  }
}

unsigned int readCmd(unsigned char cmd) {
  Serial.print("Cmd: ");
  Serial.println(cmd,HEX);
  pinMode(ICSPDAT,OUTPUT);
  for(int i=0;i<6;i++) {
    digitalWrite(ICSPDAT,cmd & 1);
    digitalWrite(ICSPCLK,1);
    delay(spd);
    digitalWrite(ICSPCLK,0);
    delay(spd);
    cmd=cmd >> 1;
  }
  pinMode(ICSPDAT,INPUT);
  //14 bits of data, shifted such that the start and stop bits are in place
  unsigned int data=0;
  for(int i=0;i<16;i++) {
    digitalWrite(ICSPCLK,1);
    delay(spd);
    digitalWrite(ICSPCLK,0);
    data=(data >> 1) & (digitalRead(ICSPDAT) << 15);
    delay(spd);
  }
  return data;
}

void sendCmd(unsigned char cmd) {
  Serial.print("Cmd: ");
  Serial.println(cmd,HEX);
  pinMode(ICSPDAT,OUTPUT);
  for(int i=0;i<6;i++) {
    digitalWrite(ICSPDAT,cmd & 1);
    digitalWrite(ICSPCLK,1);
    delay(spd);
    digitalWrite(ICSPCLK,0);
    delay(spd);
    cmd=cmd >> 1;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(ICSPDAT,OUTPUT);
  digitalWrite(ICSPDAT,0);
  pinMode(ICSPCLK,OUTPUT);
  digitalWrite(ICSPCLK,0);
  pinMode(PICPOWER,OUTPUT);
  digitalWrite(PICPOWER,0);
  Serial.println("PIC power off");
  delay(1000);
  digitalWrite(PICPOWER,1);
  delay(1000);
  Serial.println("PIC power on");
  sendMCHP();
}

const unsigned char LOAD_CFG=0x00;
const unsigned char LOAD_PGM=0x02;
const unsigned char LOAD_DAT=0x03;
const unsigned char READ_PGM=0x04;
const unsigned char READ_DAT=0x05;
const unsigned char INC_ADDR=0x06;
const unsigned char RST_ADDR=0x16;
const unsigned char BEGIN_INT_PGM=0x08;
const unsigned char BEGIN_EXT_PGM=0x18;
const unsigned char END_EXT_PGM=0x0A;
const unsigned char ERASE_PGM=0x09;
const unsigned char ERASE_DAT=0x0B;
const unsigned char ERASE_ROW=0x11;


void loop() {
  sendCmd(RST_ADDR);
  sendCmd(LOAD_CFG,0);
  for(unsigned int addr=0x8000;addr<0x81FF;addr++) {
    int dat=readCmd(READ_PGM);
    Serial.print("Addr 0x");
    Serial.print(addr,HEX);
    Serial.print(": ");
    Serial.println(dat,HEX);
    sendCmd(INC_ADDR);
  }
}
