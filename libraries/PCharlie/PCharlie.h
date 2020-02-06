//PCharlie.h
#ifndef PCHARLIE_H
#define PCHARLIE_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

extern bool PCharlieVerbose;
void allDark(void);
void dark(void);
void light(int);

#endif
