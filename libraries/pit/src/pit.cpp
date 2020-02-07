#include "pit.h"

PIT::fvoid PIT::isr[4]={nullptr,nullptr,nullptr,nullptr};

//Each of these runs the user ISR if set (not nullptr), then
//acknowledges the interrupt by clearing the PIT_TFLGn bit 
//by writing a 1.
void pit0_isr(void) {if(PIT::isr[0]) PIT::isr[0]();PIT::TFLG(0)=1;}
void pit1_isr(void) {if(PIT::isr[1]) PIT::isr[1]();PIT::TFLG(1)=1;}
void pit2_isr(void) {if(PIT::isr[2]) PIT::isr[2]();PIT::TFLG(2)=1;}
void pit3_isr(void) {if(PIT::isr[3]) PIT::isr[3]();PIT::TFLG(3)=1;}



