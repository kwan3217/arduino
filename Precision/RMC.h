//RMC.h
#ifndef RMC_h
#define RMC_h

//GPS date and time variables, valid ONLY during that
//one magic moment where process() returns true.
extern signed long gy,gm,gd,gh,gn,gs;
//Number of valid GPS sentences read
extern volatile int validgps;

//Interface to RMC parser. Pass in a character received
//from the serial port. If the function returns true,
//the time variables are valid UNTIL the next time
//this function is called.
bool process(char in);

#endif
