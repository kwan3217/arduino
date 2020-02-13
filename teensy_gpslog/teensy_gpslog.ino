//Must have library includes be outside of an ifdef, else the Arduino preprocessor doesn't know to find them
#include <pit.h>
#include <SD.h>
#include <SPI.h>
#include <ccsds.h>
#include <gprmc.h>
#include <SerLCD.h>
#include <BMP180.h>
#include <ak8975.h>
#define USE_PIT
//#define USE_USB
#define USE_SD
#define USE_GPS
#ifdef USE_PIT
#define USE_PPS
#endif
//#define USE_BMP
//#define USE_MPU
#ifdef USE_MPU
//#define USE_AK //Only can get to the AK in an MPU9150 through the MPU
#endif
#define USE_LCD

#ifdef USE_PIT
PIT pit0(0);
#endif
#ifdef USE_SD
Sd2Card card;
SdVolume volume;
SdFile root;
File ouf;
CCSDS pkt;
#endif
#ifdef USE_GPS
bool gpsStatus=false;
HardwareSerial& SerialGPS=Serial2;
GPRMC gprmc;
#endif
#ifdef USE_LCD
SerLCD lcd;
HardwareSerial& SerialLCD=Serial4;
#endif
#ifdef USE_BMP
#endif
#ifdef USE_MPU
#include <mpu60x0.h>
MPU6050 mpu(Wire);
#ifdef USE_AK
AK8975 ak(Wire);
#endif
#endif
#ifdef USE_BMP
BMP180 bmp(Wire);
#endif
//SerialUSB is #defined in teensy3/pins_arduino.h . The following #undefs
//it so we can redefine it as a Stream reference.
#undef SerialUSB
#ifdef USE_USB
Stream& SerialUSB=Serial;
#endif

#ifdef USE_SD
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
#endif

#ifdef USE_SD
#ifdef USE_GPS
void dateTime(uint16_t* date, uint16_t* time) {
  // return date using FAT_DATE macro to format fields
  if(gprmc.count>0) {
    *date = FAT_DATE(gprmc.y, gprmc.m, gprmc.d);
    // return time using FAT_TIME macro to format fields
    *time = FAT_TIME(gprmc.h, gprmc.n, gprmc.s);
  } else {
    *date=FAT_DATE(2020,1,1);
    *time=FAT_TIME(3,2,17);
  }
}
#endif
int filenum=1;
#endif

#ifdef USE_PPS
volatile uint32_t tc_pps;
volatile bool intr_pps=false;

void pps() {
  tc_pps=pit0.TC();
  intr_pps=true;
}
#endif

#ifdef USE_MPU
volatile uint32_t tc_mpu;
volatile bool intr_mpu=false;

void mpuInt() {
  tc_mpu=pit0.TC();
  intr_mpu=true;
}
#endif

const int LED=13;
#ifdef USE_PPS
const int PPS=5;
#endif
#ifdef USE_MPU
const int MPUINT=17;
#endif

void setup() {
  #ifdef USE_PIT
  pit0.begin(60.0);
  #endif
  //Some pins on breadboard are used for external
  //connections - make sure that they are INPUT (=HiZ).
  //No signal touching one of these pins will be >3.3V,
  //and HiZ means no current flow
  //XNC - Not connected on peripheral, may use for routing
  //DNC - DO NOT use for routing, not used on peripheral but connected in an incompatible way
  //      If must be connected, check that Teensy natural use is compatible
  //
  //By default, turn every pin we can to input.
  //USB Micro-B end
  //Teensy GND       //GPS XNC 
  pinMode( 0,INPUT); //GPS XNC
  pinMode( 1,INPUT); //GPS XNC, GND jumper
  pinMode( 2,INPUT); //GPS XNC
  pinMode( 3,INPUT); //GPS XNC
  pinMode( 4,INPUT); //GPS XNC
  pinMode( 5,INPUT); //GPS PPS input - actually is an input
  pinMode( 6,INPUT); //GPS XNC
  pinMode( 7,INPUT); //GPS 3V3
  pinMode( 8,INPUT); //GPS GND
  pinMode( 9,INPUT); //GPS TX (Teensy RX2)
  pinMode(10,INPUT); //GPS RX (Teensy TX2)
  pinMode(11,INPUT); //GPS XNC
  pinMode(12,INPUT); //GPS XNC
  //Teensy 3.3V      //GPS XNC
  pinMode(24,INPUT); //GPS XNC
  pinMode(25,INPUT); //GPS XNC (Mounting hole)
  pinMode(26,INPUT); //GPS XNC (Mounting hole)
  pinMode(27,INPUT); 
  pinMode(28,INPUT);
  pinMode(29,INPUT);
  pinMode(30,INPUT);
  pinMode(31,INPUT); //          (Teensy RX4)
  pinMode(32,INPUT); //SerLCD RX (Teensy TX4)
  //MicroSD end

                     //11DoF GND
                     //11DoF VCC
  //USB Micro-B end  //11DoF XNC Acc Chip Select
  //Vin (5V USB)     //11DoF XNC Acc Interrupt
  //Analog GND       //11DoF DNC MISO, 0V OK
  //3.3V             //11DoF DNC MOSI, 3.3V OK
  pinMode(23,INPUT); //11DoF DNC SCK
  pinMode(22,INPUT); //11DoF XNC Gyro Interrupt
  pinMode(21,INPUT); //11DoF XNC Gyro Chip Select
  pinMode(20,INPUT); //11DoF DNC Compass Interrupt
  pinMode(19,INPUT); //11DoF SCL (Teensy SCL0)
  pinMode(18,INPUT); //11DoF SDA (Teensy SDA0)
  pinMode(17,INPUT); //MPU GND (Considering setting these to low and high to directly power MPU)
  pinMode(16,INPUT); //MPU VCC (set to HIGH when ready to turn on MPU)
  pinMode(15,INPUT); //MPU SDA
  pinMode(14,INPUT); //MPU SCL
  pinMode(13,OUTPUT);//MPU XNC AuxSDA (pin intentionally broken off to change from DNC to XNC)
  //GND              //MPU XNC AuxSCL (pin intentionally broken off)
  pinMode(A22,INPUT);//MPU DNC COUT
  pinMode(A21,INPUT);//MPU DNC CIN
  pinMode(39,INPUT); //MPU DNC AD0
  pinMode(38,INPUT); //MPU DNC FSYNC
  pinMode(37,INPUT); //MPU INT (actually used)
  pinMode(36,INPUT);
  pinMode(35,INPUT);
  pinMode(34,INPUT);
  pinMode(33,INPUT);
  //MicroSD end
 // Open serial communications and wait for port to open:
  #ifdef USE_USB
  //begin doesn't actually do anything for the USB serial port
  //SerialUSB.begin(9600);
  delay(300);
  #endif
  #ifdef USE_PPS
  attachInterrupt(digitalPinToInterrupt(PPS),pps,RISING);
  #endif
//  attachInterrupt(digitalPinToInterrupt(MPUINT),mpuInt,RISING);
  digitalWrite(LED,LOW);

  //SerLCD init
  #ifdef USE_LCD
  SerialLCD.begin(9600);
  lcd.begin(SerialLCD);  
  lcd.command(CLEAR_COMMAND);
  lcd.setCursor(0,0);
  lcd.print("Teensy Loginator");
  lcd.setCursor(0,1);
  lcd.print(__DATE__);
  #endif
  
  //Activate I2C
  Wire.begin();
  #ifdef USE_MPU
  //Activate MPU9150 inertial part. The MPU9150 is effectively
  //an MPU6050 inertial (3axis acc+3axis gyro) sensor.
  #ifdef USE_USB
  SerialUSB.print("Connecting to MPU9150 inertial... ");
  #endif
  mpu.begin(0, //gyro scale
            0, //acc scale
            3, //bandwidth setting
            9  //samplerate divisor
            );
  #ifdef USE_USB
  SerialUSB.print("Got ID 0x");
  SerialUSB.print(mpu.whoami(),HEX);
  SerialUSB.println(" (should be 0x68)");
  SerialUSB.flush();
  #endif
  #endif

  #ifdef USE_AK
  //Activate AK8975 magnetic sensor embedded in MPU9150
  #ifdef USE_USB
  SerialUSB.print("Connecting to AK8975...           ");
  #endif
  ak.begin();
  #ifdef USE_USB
  SerialUSB.print("Got ID 0x");
  SerialUSB.print(ak.whoami(),HEX);
  SerialUSB.println(" (should be 0x48)");
  SerialUSB.flush();
  #endif
  #endif
  
  #ifdef USE_BMP
  //Activate BMP180 temperature/pressure sensor
  #ifdef USE_USB
  SerialUSB.print("Connecting to BMP180...           ");
  #endif
  bmp.begin();
  #ifdef USE_USB
  SerialUSB.print("Got ID 0x");
  SerialUSB.print(bmp.whoami(),HEX);
  SerialUSB.println(" (should be 0x55)");
  SerialUSB.flush();
  #endif
  #endif

  #ifdef USE_SD
  #ifdef USE_GPS
  SdFile::dateTimeCallback(dateTime);
  #endif
  SD.begin(chipSelect);
  
  //Decide on the filename, but don't open the file yet
  num_filename(oufn+3,filenum);
  while (SD.exists(oufn)) {
    filenum++;
    num_filename(oufn+3,filenum);
  }

  //Open the file if needed
  ouf=SD.open(oufn, FILE_WRITE);
  #ifdef USE_USB
  if(ouf) {
    SerialUSB.print("Opened file ");
    SerialUSB.println(oufn);
  } else {
    SerialUSB.print("Couldn't open ");
    SerialUSB.println(oufn);
  }
  #endif

  if(ouf) {
    pkt.begin(ouf);
    pkt.script();
    pkt.metaDoc();
    pkt.metaDoc("Packets in this stream either may or may not have a secondary header, as indicated by the secondary header bit (byte 0, bit 4) in the primary header.");
    pkt.metaDoc("If the secondary header is present, it is a 32-bit unsigned timestamp (TC, for timer count).");
    pkt.metaDoc("This timestamp increments at 60MHz, and rolls over in 60 seconds, so from 0 to 3,599,999,999 (almost 3.6 billion) ticks.");
    pkt.metaDoc("There is no direct indication of rollover, but all packets are emitted at 1Hz or faster, so rollover can be detected by the timestamp decreasing.");

    #ifdef USE_MPU
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
    #endif

    #ifdef USE_BMP
     int16_t ac1,ac2,ac3; 
    uint16_t ac4,ac5,ac6;
     int16_t b1,b2,mb,mc,md;
    bmp.getCalibration(ac1,ac2,ac3,ac4,ac5,ac6,b1,b2,mb,mc,md);

    pkt.start(8,"BMP_cal");
    pkt.fill(bmp.whoami(),"whoami0x55");
    pkt.fill(bmp.getOSS(),"oss");
    pkt.filli16(ac1,"ac1");
    pkt.filli16(ac2,"ac2");
    pkt.filli16(ac3,"ac3");
    pkt.fillu16(ac4,"ac4");
    pkt.fillu16(ac5,"ac5");
    pkt.fillu16(ac6,"ac6");
    pkt.filli16(b1,"b1");
    pkt.filli16(b2,"b2");
    pkt.filli16(mb,"mb");
    pkt.filli16(mc,"mc");
    pkt.filli16(md,"md");
    pkt.finish(8);
    #endif
  }
  #endif

  #ifdef USE_GPS
  SerialGPS.begin(9600);
  #endif
  digitalWrite(LED,HIGH);
}

char buf[512];
size_t bufptr=0;

int statusLED() {
  int result=HIGH;
  #ifdef USE_MPU
  if(!mpuStatus)result=LOW;
  #endif
  #ifdef USE_GPS
  if(!gpsStatus)result=LOW;
  #endif
  #ifdef USE_AK
  if(!magStatus)result=LOW;
  #endif
  #ifdef USE_SD
  if(!ouf)result=LOW;
  #endif
  return result;
}

#ifdef USE_GPS
void readGPS() {
  int incomingByte;
  while(SerialGPS.available() > 0) {
    incomingByte = SerialGPS.read();
    buf[bufptr]=incomingByte;
    bufptr++;
    if((bufptr>=sizeof(buf))||(incomingByte==10)) {
      buf[bufptr]=0;
      bufptr=0;
      #ifdef USE_USB
      SerialUSB.print(buf);
      #endif
      #ifdef USE_SD
      if(ouf) {
        #ifdef USE_PIT
        pkt.start(5,"NMEA",pit0.TC());
        #else
        pkt.start(5,"NMEA");
        #endif
        pkt.fill(buf,"NMEA");
        pkt.finish(5);
      }
      #endif
      gpsStatus=true;
    }
    gprmc.process(incomingByte);
  }
  digitalWrite(LED,statusLED());
}
#endif

#ifdef USE_PPS
void readPPS() {
  #ifdef USE_SD
  if(ouf && intr_pps) {
    pkt.start(6,"PPS",tc_pps);
    pkt.finish(6);
    intr_pps=false;
  }
  #endif
}
#endif

#ifdef USE_MPU
int acc=0;
bool mpuStatus=false;
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
    } else {
      SerialUSB.print("Bad MPU: ax=");
      SerialUSB.println(ax,HEX);
    }
    digitalWrite(LED,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}
#endif

#ifdef USE_BMP
int32_t UT;
int32_t UP;
int32_t T;
int32_t P;
void readBMP() {
  static elapsedMillis phase=0;
  const uint16_t period=100;
  if(phase>=period && bmp.noBlockTakeMeasurement()) {
    uint32_t tc=pit0.TC();
    phase=0;
    UT=bmp.getTemperatureRaw();
    UP=bmp.getPressureRaw();
    T=bmp.getTemperature();
    P=bmp.getPressure();
    #ifdef SD
    if(ouf) {
      if(verbose) {      
        pkt.start(10,"BMP_verbose",tc);
         int16_t ac1,ac2,ac3; 
        uint16_t ac4,ac5,ac6;
         int16_t b1,b2,mb,mc,md;
        bmp.getCalibration(ac1,ac2,ac3,ac4,ac5,ac6,b1,b2,mb,mc,md);
        pkt.filli16(ac1,"ac1");
        pkt.filli16(ac2,"ac2");
        pkt.filli16(ac3,"ac3");
        pkt.fillu16(ac4,"ac4");
        pkt.fillu16(ac5,"ac5");
        pkt.fillu16(ac6,"ac6");
        pkt.filli16(b1,"b1");
        pkt.filli16(b2,"b2");
        pkt.filli16(mb,"mb");
        pkt.filli16(mc,"mc");
        pkt.filli16(md,"md");
        pkt.filli32(UT,"UT");
        pkt.fill(bmp.getOSS(),"oss");
        pkt.filli32(UP,"UP");
         int32_t x1_0,x2_0;
         int32_t b5;
         int32_t b6;
         int32_t x1_1,x2_1,x3_1;
         int32_t b3;
         int32_t x1_2,x2_2,x3_2;
        uint32_t b4;
        uint32_t b7;
         int32_t p_0;
         int32_t x1_3a,x1_3b,x2_3;
         int32_t p_1;
        bmp.getIntermediate(x1_0,x2_0,b5,b6,x1_1,x2_1,x3_1,b3,x1_2,x2_2,x3_2,b4,b7,p_0,x1_3a,x1_3b,x2_3,p_1);
        pkt.filli32(x1_0,"x1_0");
        pkt.filli32(x2_0,"x2_0");
        pkt.filli32(T,"T");
        pkt.filli32(b6,"b6");
        pkt.filli32(x1_1,"x1_1");
        pkt.filli32(x2_1,"x2_1");
        pkt.filli32(x3_1,"x3_1");
        pkt.filli32(b3,"b3");
        pkt.filli32(x1_2,"x1_2");
        pkt.filli32(x2_2,"x2_2");
        pkt.filli32(x3_2,"x3_2");
        pkt.fillu32(b4,"b4");
        pkt.fillu32(b7,"b7");
        pkt.filli32(p_0,"p_0");
        pkt.filli32(x1_3a,"x1_3a");
        pkt.filli32(x1_3b,"x1_3b");
        pkt.filli32(x2_3,"x2_3");
        pkt.filli32(p_1,"p_1");
        pkt.finish(10);
      }
      
      pkt.start(9,"BMP",tc);
      pkt.filli32(UT,"UT");
      pkt.filli32(UP,"UP");
      pkt.filli32(T,"T");
      pkt.filli32(P,"P");
      pkt.finish(9);
    }
    #endif
    digitalWrite(LED,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}
#endif

#ifdef USE_AK
int mag=0;
bool magStatus=false;
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
        }
        mag++;
      } else {
        SerialUSB.print("Bad mag");
      }
    }
    digitalWrite(LED,(mpuStatus&&gpsStatus&&magStatus&&ouf)?HIGH:LOW);
  }
}
#endif

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

const int maxPages=4
#ifdef USE_BMP
+1
#endif
;
const int maxPhase=5;
void doSlow() {
  static elapsedMillis t;
  static int page=0;
  static int phase=0;
  static bool first=true;
  #ifdef USE_SD
  int dispfilenum=(ouf)?filenum:0;
  #endif
  bool doit=first;
  if(t>=1000) {
    doit=true;
    t=0;
  }
  float f;
  if(doit) {
    #ifdef USE_LCD
    switch(page) {
      case 0:
        lcd.setCursor(0,0);
        #ifdef USE_SD
        lcd.print("ouf:");
        print(lcd,dispfilenum,3);
        #endif
        #ifdef USE_GPS
        lcd.print(" GPS:");
        print(lcd,gprmc.count,4);
        #endif
        lcd.setCursor(0,1);
        #ifdef USE_AK
        lcd.print("mag:");
        print(lcd,mag,3);
        #endif
        #ifdef USE_MPU
        lcd.print(" acc:");
        print(lcd,acc,4);
        #endif
        break;
      case 1:
        #ifdef USE_SD
        lcd.setCursor(0,0);
        lcd.print("ouf:  ");
        print(lcd,dispfilenum,10);
        lcd.print("x");
        lcd.setCursor(0,1);
        lcd.print("tell: ");
        print(lcd,ouf?ouf.position():0,10);
        lcd.print("x");
        #endif
        break;
      case 2:
        #ifdef USE_GPS
        lcd.setCursor(0,0);
        lcd.print("date: ");
        print(lcd,gprmc.y,4);
        lcd.print("-");
        print(lcd,gprmc.m,2,'0');
        lcd.print("-");
        print(lcd,gprmc.d,2,'0');
        lcd.setCursor(0,1);
        lcd.print("time:   ");
        print(lcd,gprmc.h,2,'0');
        lcd.print(":");
        print(lcd,gprmc.n,2,'0');
        lcd.print(":");
        print(lcd,gprmc.s,2,'0');
        #endif
        break;
      case 3:
        #ifdef USE_GPS
        lcd.setCursor(0,0);
        lcd.print("lat:");
        lcd.print(gprmc.valid?"A":"V");
        lcd.print(gprmc.lat<0?"S":"N");
        f=gprmc.lat>0?gprmc.lat:-gprmc.lat;
        print(lcd,int(f),3);
        lcd.print('.');
        print(lcd,int((f-int(f))*1000000+0.5),6,'0');
        lcd.setCursor(0,1);
        lcd.print("lon: ");
        lcd.print(gprmc.lon<0?"W":"E");
        f=gprmc.lon>0?gprmc.lon:-gprmc.lon;
        print(lcd,int(f),3);
        lcd.print('.');
        print(lcd,int((f-int(f))*1000000+0.5),6,'0');
        #endif
        break;
      case 4:
        #ifdef USE_BMP
        lcd.setCursor(0,0);
        lcd.print("UT:");
        print(lcd,UT,5);
        lcd.print(" T:");
        print(lcd,T,5);
        lcd.setCursor(0,1);
        lcd.print("UP:");
        print(lcd,UP,5);
        lcd.print(" P:");
        print(lcd,P,5);
        #endif
        break;
      
    }
    #endif
    #ifdef USE_SD
    if(ouf)ouf.flush();
    #endif
    first=false;
    phase++;
    if(phase>=maxPhase) {
      phase=0;
      page++;
      if(page>=maxPages) {
        page=0;
      }
    }
  }
}

void loop(void) {
  doSlow();
  #ifdef USE_GPS
  readGPS(); 
  #endif
  #ifdef USE_PPS
  readPPS();
  #endif
  #ifdef USE_MPU
  readMPU();
  #ifdef USE_AK  
  readAK();
  #endif
  #endif
  #ifdef USE_BMP
  readBMP();
  #endif
}
