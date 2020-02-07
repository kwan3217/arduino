#ifndef pit_h
#define pit_h
#include "Arduino.h"

class PIT {
private:
  int channel;
  static uint32_t volatile& LDVAL(const int channel) {return *((uint32_t*)(0x4003'7100+channel*0x10+0x0));};
  static uint32_t volatile& CVAL (const int channel) {return *((uint32_t*)(0x4003'7100+channel*0x10+0x4));};
  static uint32_t volatile& TCTRL(const int channel) {return *((uint32_t*)(0x4003'7100+channel*0x10+0x8));};
  static uint32_t volatile& TFLG (const int channel) {return *((uint32_t*)(0x4003'7100+channel*0x10+0xC));};
  uint32_t volatile& LDVAL() {return LDVAL(channel);};
  uint32_t volatile& CVAL () {return CVAL (channel);};
  uint32_t volatile& TCTRL() {return TCTRL(channel);};
  uint32_t volatile& TFLG () {return TFLG (channel);};
  typedef void (*fvoid)(void);
  static fvoid isr[4];
  friend void pit0_isr(void);
  friend void pit1_isr(void);
  friend void pit2_isr(void);
  friend void pit3_isr(void);
public:
  PIT(int Lchannel):channel(Lchannel) {};
  void set_period(uint32_t Lperiod) {
    //Timer will count from LDVAL to 0, then after 0, go back to LDVAL, so set LDVAL to period-1
    LDVAL()=Lperiod-1; 
  };
  void stop() {
    TCTRL()&=~PIT_TCTRL_TEN;
  }
  void start() {
    TCTRL()|= PIT_TCTRL_TEN;
  }
  void begin(uint32_t Lperiod) {
    //Turn on PIT block power and enable this channel.

    //Make sure PIT block power is on -- all accesses to PIT registers will fail
    //and crash the program before this.
    SIM_SCGC6|=SIM_SCGC6_PIT;

    //Enable the PITs in general
    PIT_MCR&=~PIT_MCR_MDIS;

    //Explicilty write 0 to all control bits
    TCTRL()=0;
    set_period(Lperiod);
    //Enable the timer count
    start();
  };
  void begin(float period) {
    begin((uint32_t)(period*Hz()));
  }
  static uint32_t Hz() {int OUTDIV2=(SIM_CLKDIV1>>24) & 0x0F;return F_CPU/(OUTDIV2+1);};
  uint32_t TC() {
    //Turn the natural countdown behavior into a count-up clock as used with previous
    //Loginators. Note that if you change the period, the count-up will be wrong until
    //the timer rolls over or is stop()ped and re start()ed. This
    //will return a value between 0 and period-1, inclusive (IE if you set the period 
    //to 100, this will count from 0 to 99)
    return LDVAL()-CVAL();
  }
  void attachInterrupt(fvoid Lisr) {isr[channel]=Lisr;   TCTRL()|= PIT_TCTRL_TIE;};
  void detachInterrupt(          ) {isr[channel]=nullptr;TCTRL()&=~PIT_TCTRL_TIE;};
};

#endif

