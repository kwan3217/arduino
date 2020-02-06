#include "packet.h"

bool Packet::fill(const char* in) {
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  }
  return true;
}

bool Packet::fill(const char* in, uint32_t length) {
  for(uint32_t i=0;i<length;i++) {
    if(!fill(in[i])) return false;
  }
  return true;
}

void Packet::forget() {
  buf->forget();
}


