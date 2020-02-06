#include "mpu60x0.h"
//#include <Serial.h>

static const int gyroBandwidth[]={ 256, 188,  98,  42,  20,  10,   5,  -1};
static const int  accBandwidth[]={ 260, 184,  94,  44,  21,  10,   5,  -1};
static const int gyroSamplekHz[]={8000,1000,1000,1000,1000,1000,1000,8000};

/** Start up an MPU60x0. This handles both bus types, and presumes that the bus is already initialized. It wakes up the device,
  and sets the clock source to the X gyro oscillator. It then sets external sync to none, and sets up the low-pass filter.
  Next it sets the gyro config register, which turns off gyro self-testing and sets the gyro scale.
  Finally it sets the acc config register, which turns off acc self-testing and sets the acc scale.

@param gyro_scale Full scale range of the gyro is set to &plusmn;250*2^gyro_scale &deg;/s. May be 0-3 (&plusmn;250-2000&deg;/s)
@param acc_scale Full scale range of the acc is set to &plusmn;2*2^acc_scale g. May be 0-3 (&plusmn;2-16g)
*/
bool MPU60x0::begin(uint8_t gyro_scale, uint8_t acc_scale, uint8_t bandwidth, uint8_t sample_rate) {
//  Serial.print("MPU60x0::begin(");Serial.print(gyro_scale,DEC);Serial.print(",");Serial.print(acc_scale,DEC);Serial.println(")");
  //Wake the part up, Set up the clock source (x gyro, 1)
  write(0x6B,(0 << 7) | (0 << 6) | (0 << 5) | (0x01 << 0));
  delay(50);
  //Do anything necessary to init the part. Bus is available at this point.
  //Set External sync (none, 0x00) and Low Pass Filter 
  bandwidth&=0x07;
  uint8_t lpf_setup=(0x00<<3) | (bandwidth<<0);
//  Serial.print("Sync/LPF config: 0x");Serial.println(lpf_setup,HEX,2);
//  Serial.print("Gyro bandwidth:   ");Serial.print(gyroBandwidth[bandwidth]);Serial.println("Hz");
//  Serial.print("Acc  bandwidth:   ");Serial.print( accBandwidth[bandwidth]);Serial.println("Hz");
//  Serial.print("Gyro sample rate: ");Serial.print(gyroSamplekHz[bandwidth]);Serial.println("Hz");
  write(0x1A,lpf_setup);
  //Turn off all gyro self-test and set the gyro scale
  uint8_t gyro_config=(0 << 7) | (0 << 6) | (0 << 5) | ((gyro_scale & 0x03) << 3);
//  Serial.print("Gyro config: 0x");Serial.println(gyro_config,HEX,2);
//  Serial.print("Gyro full scale: +/-");Serial.print(250*(1<<gyro_scale));Serial.println("deg/s");
  write(0x1B,gyro_config);
  //Turn off all acc  self-test and set the acc  scale
  uint8_t acc_config =(0 << 7) | (0 << 6) | (0 << 5) | ((acc_scale  & 0x03) << 3);
//  Serial.print("Acc  config: 0x");Serial.println(acc_config,HEX,2);
//  Serial.print("Acc  full scale: +/-");Serial.print(2*(1<<acc_scale));Serial.println("g");
  write(0x1C,acc_config);
  //Set the sample rate
//  Serial.print("Sample rate: ");Serial.print(sample_rate,DEC);Serial.print("gyro sample/acc sample");
//  Serial.print("Actual sample rate: ");Serial.print(gyroSamplekHz[bandwidth]/(1+sample_rate));Serial.println("Hz");
  write(0x19,sample_rate);
  //Set up I2C passthrough, so that the magnetic sensor on a 9150 can be accessed. Also set int levels
  uint8_t int1_config=(0 << 7) | //0 - INT line is active high
                      (0 << 6) | //0 - INT line is push-pull (no pullup needed, can't attach another int line and do a wired and)
                      (0 << 5) | //0 - LATCH_INT_EN is disabled, int line emits a 50us pulse. This is detectable by the capture line but doesn't need any action to reset
                      (1 << 4) | //1 - INT_RD_CLEAR on any read. If you want int status, read that register first on receiving an int
                      (0 << 3) | //0 - Frame Sync input - designed to trigger a read in sync with an external event, like a video frame capture. Not used in any Kwan design yet
                      (0 << 2) | //0 - Frame sync interrupt enabled - when set, a pulse on the FSYNC input to the 60x0 will cause an interrupt on the INT line to the host
                      (1 << 1) ; //1 - I2C_BYPASS_EN - bypass enable. When set to 1, the auxillary bus (including the mag sensor on the 9150) is just tied to the main
                                 //                    bus so that the host can see the auxillary sensor(s) directly.
//  Serial.print("Int1 config: 0x");Serial.println(int1_config,HEX,2);
  write(0x37,int1_config);
//  Serial.print("User ctrl: 0x");Serial.println(read(0x6B),HEX,2);
  //In order for passthrough to work, the 9150 I2C bus master must be disabled, but this is default, so we don't do anything with it
  return true;
}

/** Read 1 byte from the sensor
@param  address register address to read from
*/
unsigned char MPU6050::read(uint8_t address) {
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();

  port.requestFrom(ADDRESS, 1);
  return port.read();
}

/** Read a 16-bit integer in big-endian format from the sensor
@param  address register address to read from
@return A 16 byte value. MSB will be from address, LSB will be from address+1
*/
int16_t MPU6050::read16(uint8_t address) {

  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();

  port.requestFrom(ADDRESS, 2);
  int msb, lsb;
  msb = port.read();
  lsb = port.read();
  return msb<<8 | lsb;
}

/** Write to a register
@param  address register address to write to
@param data value to write to the register
*/
void MPU6050::write(uint8_t address, uint8_t data) {
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.write(data);
  port.endTransmission();
}

/** Read the sensor data values. This isn't optimized for any particular bus, and therefore doesn't do burst
    mode, but this may be overridden to allow burst mode in a particular bus.
@param ax Accelerometer x readout
@param ay Accelerometer y readout
@param az Accelerometer z readout
@param gx Gyroscope x readout
@param gy Gyroscope y readout
@param gz Gyroscope z readout
@param t  Thermometer readout
*/
bool MPU60x0::read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t) {
  ax=read16(0x3B);
  ay=read16(0x3D);
  az=read16(0x3F);
  t =read16(0x41);
  gx=read16(0x43);
  gy=read16(0x45);
  gz=read16(0x47);
  return true;
}

/** Read the sensor values with I2C burst mode
@param ax Accelerometer x readout
@param ay Accelerometer y readout
@param az Accelerometer z readout
@param gx Gyroscope x readout
@param gy Gyroscope y readout
@param gz Gyroscope z readout
@param t  Thermometer readout
*/
bool MPU6050::read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t) {
  int msb, lsb;
  port.beginTransmission(ADDRESS);
  port.write(0x3B);
  port.endTransmission();

  port.requestFrom(ADDRESS, 14);
  msb = port.read();
  lsb = port.read();
  ax= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  ay= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  az= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  t= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  gx= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  gy= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  gz= msb<<8 | lsb;
  return true;
}

