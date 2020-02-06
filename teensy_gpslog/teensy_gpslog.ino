#include <gprmc.h>
#include <SD.h>
#include <SPI.h>
#include <mpu60x0.h>
#include <ak8975.h>
#include <SerLCD.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
MPU6050 mpu(Wire);
AK8975 ak(Wire);
SerLCD lcd;
// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// Teensy audio board: pin 10
// Teensy 3.5 & 3.6 on-board: BUILTIN_SDCARD
// Wiz820+SD board: pin 4
// Teensy 2.0: pin 0
// Teensy++ 2.0: pin 20
const int chipSelect = BUILTIN_SDCARD;

void num_filename(char* fn, int num) {
  const int digits=5;
  int mod0=1;
  for(int i=0;i<digits;i++) {
    int digit=num/mod0%10;
    fn[digits-i-1]=digit+'0';
    mod0*=10;
  }
}

File ouf;
char oufn[]="LOG00000.TXT";

void dateTime(uint16_t* date, uint16_t* time) {
  // return date using FAT_DATE macro to format fields
  if(validgps>0) {
    *date = FAT_DATE(y, m, d);
    // return time using FAT_TIME macro to format fields
    *time = FAT_TIME(h, n, s);
  } else {
    *date=FAT_DATE(2020,1,1);
    *time=FAT_TIME(3,2,17);
  }
}

int filenum=1;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial1.setTX(26);
  Serial1.setRX(27);
  Serial1.begin(9600);
  delay(300);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

  //SerLCD init
  Serial2.begin(9600);
  lcd.begin(Serial2);  
  lcd.command(CLEAR_COMMAND);
  lcd.setCursor(0,0);
  lcd.print("Teensy Loginator");
  
  //Activate I2C
  Wire.begin();
  
  //Activate MPU9150 inertial part. The MPU9150 is effectively
  //an MPU6050 inertial (3axis acc+3axis gyro) sensor.
  Serial.print("Connecting to MPU9150 inertial... ");
  mpu.begin();
  Serial.print("Got ID 0x");
  Serial.print(mpu.whoami(),HEX);
  Serial.println(" (should be 0x68)");
  Serial.flush();
  //Activate AK8975 magnetic sensor embedded in MPU9150
  Serial.print("Connecting to AK8975...           ");
  ak.begin();
  Serial.print("Got ID 0x");
  Serial.print(ak.whoami(),HEX);
  Serial.println(" (should be 0x48)");
  Serial.flush();

  SdFile::dateTimeCallback(dateTime);
  SD.begin(chipSelect);

  //Decide on the filename, but don't open the file yet
  num_filename(oufn+3,filenum);
  while (SD.exists(oufn)) {
    filenum++;
    num_filename(oufn+3,filenum);
  }
}

char buf[512];
int bufptr=0;
int acc=0;
int mag=0;
bool mpuStatus=false;
bool magStatus=false;
bool gpsStatus=false;
void readGPS() {
  int incomingByte;
  while(Serial1.available() > 0) {
    incomingByte = Serial1.read();
    state(incomingByte);
    buf[bufptr]=incomingByte;
    bufptr++;
    if((bufptr>=sizeof(buf))||(incomingByte==10)) {
      if(!ouf && validgps>0) {
        //Open the file if needed
        ouf=SD.open(oufn, FILE_WRITE);
        if(!ouf) {
          Serial.print("Couldn't open ");
          Serial.println(oufn);
        } else {
          digitalWrite(13,HIGH);
        }
      }
      buf[bufptr]=0;
      bufptr=0;
      //Serial.print(buf);
      if(ouf) {
        ouf.print(buf);
        ouf.flush();
      }
      gpsStatus=true;
      digitalWrite(13,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
    }
  }
}

void printPKWNF(Print& ouf, unsigned long ts, int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz, int16_t t) {
  ouf.print("$PKWNF,");
  ouf.print(ts);ouf.print(",");
  ouf.print(ax);ouf.print(",");
  ouf.print(ay);ouf.print(",");
  ouf.print(az);ouf.print(",");
  ouf.print(gx);ouf.print(",");
  ouf.print(gy);ouf.print(",");
  ouf.print(gz);ouf.print(",");
  ouf.print(t);ouf.println("*");
  ouf.flush();  
}

void printPKWNK(Print& ouf, unsigned long ts, int16_t bx, int16_t by, int16_t bz, uint8_t st1, uint8_t st2) {
  ouf.print("$PKWNK,");
  ouf.print(ts);ouf.print(",");
  ouf.print(bx);ouf.print(",");
  ouf.print(by);ouf.print(",");
  ouf.print(bz);ouf.print(",");
  ouf.print(st1);ouf.print(",");
  ouf.print(st2);ouf.println("*");
  ouf.flush();  
}

void readMPU() {
  static unsigned long oldMillis=0;
  const int period=10;
  unsigned long newMillis=millis();
  if(newMillis>=oldMillis+period) {
    oldMillis+=period;
    int16_t ax,ay,az,gx,gy,gz,t;
    mpu.read(ax,ay,az,gx,gy,gz,t);
    mpuStatus=ax!=0 && ax!=-1;
    if(mpuStatus) {
      if(ouf) {
        printPKWNF(ouf,newMillis,ax,ay,az,gx,gy,gz,t);
      }
      acc++;
    } else {
      //Serial.print("Bad mpu");
    }
    //printPKWNF(Serial,newMillis,ax,ay,az,gx,gy,gz,t);
    digitalWrite(13,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}

void readAK() {
  static unsigned long oldMillis=0;
  const int period=100;
  unsigned long newMillis=millis();
  if(newMillis>=oldMillis+period) {
    if(ak.noblockMeasurement()) {
      oldMillis+=period; //Only do this after we have a measurement, so that if the measurement isn't ready yet, we check immediately next time around
      int16_t bx,by,bz;
      uint8_t st1,st2;
      ak.read(bx,by,bz,st1,st2);
      magStatus=(bx!=0 && bx!=-1);
      if(magStatus) {
        if(ouf) {
          printPKWNK(ouf,newMillis,bx,by,bz,st1,st2);
        }
        mag++;
      } else {
        //Serial.print("Bad mag");
      }
      //printPKWNK(Serial,newMillis,bx,by,bz,st1,st2);
    }
    digitalWrite(13,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}

void print(SerLCD& ouf, int val, int digits, char pad='\0') {
  int powcheck=1;
  for(int i=0;i<digits-1;i++)powcheck*=10;
  int mod=powcheck*10;
  if(pad==0) pad=(val>=mod)?'0':' ';
  for(int i=0;i<digits-1;i++) {
    if(val%mod<powcheck) ouf.print(pad);
    powcheck/=10;
  }
  ouf.print(val%mod,DEC);
}

void updateDisplay() {
  static bool first=true;
  static int oldFilenum;
  static int oldGPS;
  int dispfilenum=ouf?filenum:0;
  if(first || (oldFilenum!=dispfilenum) || (oldGPS!=validgps)) {
    lcd.setCursor(0,0);
    lcd.print("ouf:");
    print(lcd,dispfilenum,3);
    lcd.print(" GPS:");
    print(lcd,validgps,4);
    lcd.setCursor(0,1);
    lcd.print("mag:");
    print(lcd,mag,3);
    lcd.print(" acc:");
    print(lcd,acc,4);
    oldFilenum=dispfilenum;
    oldGPS=validgps;
    first=false;
  }
}

void loop(void) {
  readGPS(); 
  readMPU();
  readAK();
  updateDisplay();
}
