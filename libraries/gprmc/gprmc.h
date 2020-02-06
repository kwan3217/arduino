#ifndef gprmc_h
#define gprmc_h

#include "Arduino.h"

typedef void (*State)(char);
extern signed long h,n,s,d,m,y;
extern State state;
extern int validgps;

#endif

