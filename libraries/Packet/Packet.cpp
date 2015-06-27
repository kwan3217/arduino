#include "Packet.h"

bool Packet::fill(const char* in) {
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  } 
  return true;
}

bool Packet::fill(const char* in, uint16_t length) {
  for(uint16_t i=0;i<length;i++) {
    if(!fill(in[i])) return false;
  }
  return true;
}

bool CCSDS::start(uint16_t apid, uint16_t* seq, uint32_t TC) {
  const uint16_t Ver=0;  //0 for standard CCSDS telecommand according to CCSDS 102.0-B-5 11/2000
  const uint16_t Type=0; //0 for telemetry, 1 for command
  uint16_t Sec=(TC!=0xFFFFFFFFU)?1:0;  //Presence of secondary header
  const uint16_t Grp=0x03;  //Grouping flags - 3->not in a group of packets
  if(!fill16(((apid & 0x7FF) | (Sec & 0x01) << 11 | (Type & 0x01) << 12 | (Ver & 0x07) << 13))) return false;
  uint16_t seq_=0;
  if(seq) seq_=seq[apid-seqOffset];
  if(!fill16(((seq_ & 0x3FFF) | (Grp & 0x03) << 14))) return false;
  if(!fill16(0xDEAD)) return false; //Reserve space in the packet for length
  if(Sec) {
    //Secondary header: count of microseconds since beginning of minute
    if(!fill32(TC)) return false;
  }
  if(seq) seq[apid]=(seq[apid]+1)& 0x3FFF;
  return true;
}

bool CCSDS::finish() {
  uint16_t len=buf.unreadylen()-7;
  buf.pokeMid(4,(len >> 8) & 0xFF);
  buf.pokeMid(5,(len >> 0) & 0xFF);
  buf.mark();
  return true;
}

//Fill in Big-endian order as specified by CCSDS 102.0-B-5, 1.6a
bool CCSDS::fill16(uint16_t in) {
  return buf.fill16BE(in);
}

bool CCSDS::fill32(uint32_t in) {
  return buf.fill32BE(in);
}

bool CCSDS::fill64(uint64_t in) {
  if(!buf.fill32BE((uint32_t)((in >> 32) & 0xFFFFFFFFU))) return false;
  if(!buf.fill32BE((uint32_t)((in >>  0) & 0xFFFFFFFFU))) return false;
  return true;
}


