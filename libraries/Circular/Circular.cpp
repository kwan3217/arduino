#include "Circular.h"

bool Circular::isFull() {
  return (head+1)%N==tail;
}

bool Circular::isEmpty() {
  return head==tail;
}

//returns true if character written, false 
//if character could not be written
bool Circular::fill(char in) {
  if(!isFull()) {
    buf[head]=in;
    head=(head+1)%N;
    return true;
  }
  return false;
}

char Circular::get() {
  if(isEmpty()) return 0;
  char result=buf[tail];
  tail=(tail+1)%N;
  return result;
}

bool Circular::fill(const char* in) {
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  }
  return true;
}

bool Circular::fill(const char* in, uint16_t len) {
  for(uint16_t i=0;i<len;i++) {
    if(!fill(in[i])) return false;;
  }
  return true;
}

char Circular::peekMid(uint16_t pos) {
  pos+=mid;
  pos%=N;
  return buf[pos];
}

char Circular::peekHead(uint16_t pos) {
  pos+=head;
  pos%=N;
  return buf[pos];
}

char Circular::peekTail(uint16_t pos) {
  pos+=tail;
  pos%=N;
  return buf[pos];
}

void Circular::pokeMid(uint16_t pos, char poke) {
  pos+=mid;
  pos%=N;
  buf[pos]=poke;
}

void Circular::pokeHead(uint16_t pos, char poke) {
  pos+=head;
  pos%=N;
  buf[pos]=poke;
}

void Circular::pokeTail(uint16_t pos, char poke) {
  pos+=tail;
  pos%=N;
  buf[pos]=poke;
}

void Circular::mark() {
  mid=head;
}

uint16_t Circular::unreadylen() {
  uint16_t h=head;
  uint16_t m=mid;
  if(h<m) h+=N;
  return h-m;
}

uint16_t Circular::readylen() {
  uint16_t m=mid;
  uint16_t t=tail;
  if(m<t) m+=N;
  return m-t;
}

bool Circular::drain(Circular& to) {
  while(readylen()>0) {
    if(to.isFull()) {
      to.empty();
      empty();
      return false;
    }
    to.fill(get());
  }
  to.mark();
  return true;
}

void Circular::empty() {
  head=0;
  tail=0;
  mid=0;
}

bool Circular::fill16BE(uint16_t in) {
  if(!fill((char)((in >> 8) & 0xFF))) return false;
  if(!fill((char)((in >> 0) & 0xFF))) return false;
  return true;
}
bool Circular::fill32BE(uint32_t in) {
  if(!fill((char)((in >> 24) & 0xFF))) return false;
  if(!fill((char)((in >> 16) & 0xFF))) return false;
  if(!fill((char)((in >>  8) & 0xFF))) return false;
  if(!fill((char)((in >>  0) & 0xFF))) return false;
  return true;
};
bool Circular::fill16LE(uint16_t in) {
  if(!fill((char)((in >> 0) & 0xFF))) return false;
  if(!fill((char)((in >> 8) & 0xFF))) return false;
}
bool Circular::fill32LE(uint32_t in) {
  if(!fill((char)((in >>  0) & 0xFF))) return false;
  if(!fill((char)((in >>  8) & 0xFF))) return false;
  if(!fill((char)((in >> 16) & 0xFF))) return false;
  if(!fill((char)((in >> 24) & 0xFF))) return false;
}

