//Stub Arduino - minimal required to run an Arduino sketch on a host
#include <cstdio>
#include "Arduino.h"

void StubSerial::print  (      char  c) {printf("%c"  ,c);}
void StubSerial::print  (const char* s) {printf("%s"  ,s);}
void StubSerial::println(      char  c) {printf("%c\n",c);}
void StubSerial::println(const char* s) {printf("%s\n",s);}
void StubSerial::print(int i, int radix) {
  if(radix==DEC) {
    printf("%d");
  } else {
    printf("%x");
  }
}
void StubSerial::println(int i, int radix) {
  print(i,radix);
  printf("\n");
}

StubSerial Serial;
