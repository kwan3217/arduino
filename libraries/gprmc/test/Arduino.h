//Stub Arduino which provides a Serial class which just prints to stdout
#ifndef Arduino_h
#define Arduino_h

const int DEC=10;
const int HEX=16;
const int BIN=2;

class StubSerial {
public:
  void print(      char c);
  void print(const char* s);
  void print(int i,int radix=DEC);
  void println(      char  c);
  void println(const char* s);
  void println(int i,int radix=DEC);
  
};

extern StubSerial Serial;

#endif
