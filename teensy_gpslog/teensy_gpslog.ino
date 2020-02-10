#include <ccsds.h>
#include <gprmc.h>
#include <SD.h>
#include <SPI.h>
#include <mpu60x0.h>
#include <ak8975.h>
#include <SerLCD.h>
#include <pit.h>
#include <BMP180.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
MPU6050 mpu(Wire);
AK8975 ak(Wire);
PIT pit0(0);
SerLCD lcd;
BMP180 bmp(Wire);

File ouf;
CCSDS pkt;

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

volatile uint32_t tc_pps;
volatile bool intr_pps=false;

void pps() {
  tc_pps=pit0.TC();
  intr_pps=true;
}

volatile uint32_t tc_mpu;
volatile bool intr_mpu=false;

void mpuInt() {
  tc_mpu=pit0.TC();
  intr_mpu=true;
}

const int LED=13;
const int PPS=11;
const int MPUINT=17;

void setup() {
 // Open serial communications and wait for port to open:
  pit0.begin(60.0);
  Serial.begin(9600);
  delay(300);
  pinMode(LED   ,OUTPUT);
  pinMode(PPS   ,INPUT); 
  pinMode(MPUINT,INPUT); 
  attachInterrupt(digitalPinToInterrupt(PPS),pps,RISING);
//  attachInterrupt(digitalPinToInterrupt(MPUINT),mpuInt,RISING);
  digitalWrite(LED,LOW);

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
  mpu.begin(0, //gyro scale
            0, //acc scale
            3, //bandwidth setting
            9  //samplerate divisor
            );
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

  //Activate BMP180 temperature/pressure sensor
  Serial.print("Connecting to BMP180...           ");
  bmp.begin(0);
  Serial.print("Got ID 0x");
  Serial.print(bmp.whoami(),HEX);
  Serial.println(" (should be 0x55)");
  Serial.flush();

  SdFile::dateTimeCallback(dateTime);
  SD.begin(chipSelect);

  //Decide on the filename, but don't open the file yet
  num_filename(oufn+3,filenum);
  while (SD.exists(oufn)) {
    filenum++;
    num_filename(oufn+3,filenum);
  }

  //Open the file if needed
  ouf=SD.open(oufn, FILE_WRITE);
  if(ouf) {
    Serial.print("Opened file ");
    Serial.println(oufn);
  } else {
    Serial.print("Couldn't open ");
    Serial.println(oufn);
  }

  if(ouf) {
    pkt.begin(ouf);
    pkt.script();
    pkt.metaDoc();
    pkt.metaDoc("Packets in this stream either may or may not have a secondary header, as indicated by the secondary header bit (byte 0, bit 4) in the primary header.");
    pkt.metaDoc("If the secondary header is present, it is a 32-bit unsigned timestamp (TC, for timer count).");
    pkt.metaDoc("This timestamp increments at 60MHz, and rolls over in 60 seconds, so from 0 to 3,599,999,999 (almost 3.6 billion) ticks.");
    pkt.metaDoc("There is no direct indication of rollover, but all packets are emitted at 1Hz or faster, so rollover can be detected by the timestamp decreasing.");
    uint8_t        gyro_scale_raw, acc_scale_raw, bandwidth_raw, smplrt_div;
    float          gyro_scale, gyro_bw, acc_scale, acc_bw, sample_rate;
    mpu.get_config(gyro_scale_raw, acc_scale_raw, bandwidth_raw, smplrt_div,
                   gyro_scale, gyro_bw, acc_scale, acc_bw, sample_rate);
    pkt.start(7,"MPU_config");
    pkt.fill(gyro_scale_raw,"gyro_scale_raw");
    pkt.fill(acc_scale_raw ,"acc_scale_raw");
    pkt.fill(bandwidth_raw ,"bandwdith_raw");
    pkt.fill(smplrt_div    ,"smplrt_div");
    pkt.fillfp(gyro_scale   ,"gyro_scale");
    pkt.fillfp(gyro_bw      ,"gyro_bw");
    pkt.fillfp(acc_scale    ,"acc_scale_raw");
    pkt.fillfp(acc_bw       ,"acc_bw");
    pkt.fillfp(sample_rate  ,"sample_rate");
    pkt.finish(7);

    pkt.start(8,"BMP_cal");
    pkt.fill(gyro_scale_raw,"gyro_scale_raw");
    pkt.fill(acc_scale_raw ,"acc_scale_raw");
    pkt.fill(bandwidth_raw ,"bandwdith_raw");
    pkt.fill(smplrt_div    ,"smplrt_div");
    pkt.fillfp(gyro_scale   ,"gyro_scale");
    pkt.fillfp(gyro_bw      ,"gyro_bw");
    pkt.fillfp(acc_scale    ,"acc_scale_raw");
    pkt.fillfp(acc_bw       ,"acc_bw");
    pkt.fillfp(sample_rate  ,"sample_rate");
    pkt.finish(7);
  }
  Serial1.setTX(26);
  Serial1.setRX(27);
  Serial1.begin(9600);
  digitalWrite(LED,HIGH);
}

char buf[512];
size_t bufptr=0;
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
      }
      buf[bufptr]=0;
      bufptr=0;
      //Serial.print(buf);
      if(ouf) {
        pkt.start(5,"NMEA",pit0.TC());
        pkt.fill(buf,"NMEA");
        pkt.finish(5);
      }
      gpsStatus=true;
    }
  }
  digitalWrite(LED,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
}

void readPPS() {
  if(ouf && intr_pps) {
    pkt.start(6,"PPS",tc_pps);
    pkt.finish(6);
    intr_pps=false;
  }
}

void readMPU() {
  static unsigned long oldMillis=0;
  const int period=10;
  unsigned long newMillis=millis();
  if(newMillis>=oldMillis+period) {
    oldMillis+=period;
    int16_t ax,ay,az,gx,gy,gz,t;
    uint32_t tc=pit0.TC();
    mpu.read(ax,ay,az,gx,gy,gz,t);
    mpuStatus=ax!=0 && ax!=-1;
    if(mpuStatus) {
      if(ouf) {
        pkt.start(3,"MPUinertial",tc);
        pkt.filli16(ax,"ax");
        pkt.filli16(ay,"ay");
        pkt.filli16(az,"az");
        pkt.filli16(gx,"gx");
        pkt.filli16(gy,"gy");
        pkt.filli16(gz,"gz");
        pkt.filli16(t, "t" );
        pkt.finish(3);
      }
      acc++;
    }
    //printPKWNF(Serial,newMillis,ax,ay,az,gx,gy,gz,t);
    digitalWrite(LED,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}

void readBMP() {
  static unsigned long oldMillis=0;
  const int period=100;
  unsigned long newMillis=millis();
  if(newMillis>=oldMillis+period) {
    oldMillis+=period;
    int16_t ax,ay,az,gx,gy,gz,t;
    uint32_t tc=pit0.TC();
    mpu.read(ax,ay,az,gx,gy,gz,t);
    mpuStatus=ax!=0 && ax!=-1;
    if(mpuStatus) {
      if(ouf) {
        pkt.start(3,"MPUinertial",tc);
        pkt.filli16(ax,"ax");
        pkt.filli16(ay,"ay");
        pkt.filli16(az,"az");
        pkt.filli16(gx,"gx");
        pkt.filli16(gy,"gy");
        pkt.filli16(gz,"gz");
        pkt.filli16(t, "t" );
        pkt.finish(3);
      }
      acc++;
    }
    //printPKWNF(Serial,newMillis,ax,ay,az,gx,gy,gz,t);
    digitalWrite(LED,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}

void readAK() {
  static unsigned long oldMillis=0;
  const int period=100;
  unsigned long newMillis=millis();
  if(newMillis>=oldMillis+period) {
    if(ak.noblockMeasurement()) {
      uint32_t tc=pit0.TC();
      oldMillis+=period; //Only do this after we have a measurement, so that if the measurement isn't ready yet, we check immediately next time around
      int16_t bx,by,bz;
      uint8_t st1,st2;
      ak.read(bx,by,bz,st1,st2);
      magStatus=(bx!=0 && bx!=-1);
      if(magStatus) {
        if(ouf) {
          pkt.start(4,"MPUmagnetic",tc);
          pkt.filli16(bx,"bx");
          pkt.filli16(by,"by");
          pkt.filli16(bz,"bz");
          pkt.fill(st1, "st1" );
          pkt.fill(st2, "st2" );
          pkt.finish(4);
          //printPKWNK(ouf,newMillis,bx,by,bz,st1,st2);
        }
        mag++;
      } else {
        //Serial.print("Bad mag");
      }
      //printPKWNK(Serial,newMillis,bx,by,bz,st1,st2);
    }
    digitalWrite(LED,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}

void print(Print& ouf, int val, int digits, char pad='\0') {
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
  static int page=0;
  static int maxPages=2;
  static int phase=0;
  static int maxPhase=5;
  static bool first=true;
  static int oldFilenum;
  static int oldGPS;
  int dispfilenum=ouf?filenum:0;
  if(first || (oldFilenum!=dispfilenum) || (oldGPS!=validgps)) {
    switch(page) {
      case 0:
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
        break;
      case 1:
        lcd.setCursor(0,0);
        lcd.print("ouf:  ");
        print(lcd,dispfilenum,10);
        lcd.print("x");
        lcd.setCursor(0,1);
        lcd.print("tell: ");
        print(lcd,ouf?ouf.position():0,10);
        lcd.print("x");
        break;
    }
    oldFilenum=dispfilenum;
    oldGPS=validgps;
    first=false;
    phase++;
    if(phase>=maxPhase) {
      phase=0;
      page++;
      if(page>=maxPages) {
        page=0;
      }
    }
    if(ouf)ouf.flush();
  }
}

void loop(void) {
  readGPS(); 
  readPPS();
  readMPU();
  readAK();
  readBMP();
  updateDisplay();
}
