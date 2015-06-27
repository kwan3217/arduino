#ifndef Packet_h
#define Packet_h
#include <Arduino.h>
#include <Circular.h>

class Packet {
protected: 
  Circular& buf;
public:
  Packet(Circular &Lbuf):buf(Lbuf) {};
  virtual bool start(uint16_t apid, uint16_t* seq=0, uint32_t TC=0xFFFFFFFF)=0;
  virtual bool finish()=0;
  bool fill(char in) {return buf.fill(in);};
  virtual bool fill16(uint16_t in)=0;
  virtual bool fill32(uint32_t in)=0;
  virtual bool fill64(uint64_t in)=0;
  virtual bool fill(const char* in);
  virtual bool fill(const char* in, uint16_t length);
};

class CCSDS: public Packet{
private:
  uint16_t seqOffset;
public:
  CCSDS(Circular &Lbuf, uint16_t LseqOffset):Packet(Lbuf),seqOffset(LseqOffset) {};
  virtual bool start(uint16_t apid, uint16_t* seq=0, uint32_t TC=0xFFFFFFFF);
  virtual bool finish();
  virtual bool fill16(uint16_t in);
  virtual bool fill32(uint32_t in);
  virtual bool fill64(uint64_t in);
};

#endif

