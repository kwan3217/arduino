#ifndef packet_h
#define packet_h
#include "Arduino.h"
#include <Packet.h>
//#define DEBUG

class CCSDS: public Packet{
private:
  uint16_t* seq;
  uint16_t current_apid;
  bool* docd;
  bool indoc;
  CharPrint docbuf; //If writing a doc packet, write it to this buffer
  CharPrint pktbuf; //Otherwise write it to this buffer
  int pkt_apid;
  int apidsize;
  bool writeDoc(uint16_t explicit_doc_apid, uint8_t type, uint16_t pos, const char* desc) override;
  bool writeDoc(uint8_t type, const char* desc) override {
    #ifdef DEBUG
    Serial.print("CCSDS::writeDoc(");Serial.print(type);
    if(desc) {
      Serial.print(",\"");Serial.print(desc);Serial.println("\")");
    } else {
      Serial.println(",nullptr)");
    }
    #endif
    if(!desc) {
      #ifdef DEBUG
      Serial.print("No documentation string, returning true");
      #endif
      return true;
    }
    if(docd[doc_apid]) {
      #ifdef DEBUG
      Serial.print("Apid ");Serial.print(doc_apid);Serial.print("already documented, returning true");
      #endif
      return true;
    }
    return writeDoc(doc_apid, type,(type==0?0:pktbuf.tell()),desc);
  };
  bool writeDoc(              const char*   pktName) override {return writeDoc(0,pktName);};
  bool script();
public:
  using Packet::metaDoc;
  using Packet::filli16;
  using Packet::filli32;
  using Packet::filli64;
  using Packet::fillu16;
  using Packet::fillu32;
  using Packet::fillu64;
  using Packet::fillfp;
  using Packet::fill;
  using Packet::start;
  CCSDS(size_t Lapidsize=64, size_t Lpktsize=65536, size_t Ldocsize=512):current_apid(0),docbuf(Ldocsize),pktbuf(Lpktsize),apidsize(Lapidsize) {
    docd=new bool[apidsize];seq=new uint16_t[apidsize];
    for(int i=0;i<apidsize;i++) {
      docd[i]=false;
      seq[i]=0;
    }
  };
  virtual ~CCSDS() {delete docd;delete seq;};
  bool start(uint16_t apid, uint32_t TC=0xFFFFFFFF) override {
    uint16_t this_seq=0;
    if(apid<apidsize) {
      this_seq=seq[apid];
    }
    if(apid<apidsize) seq[apid]=(seq[apid]+1)& 0x3FFF;
    return start(apid,0,0,0x03,this_seq,TC);
  };
  bool start(uint16_t apid, uint8_t Ver, uint8_t Type, uint8_t Grp, uint16_t Seq, uint32_t TC=0xFFFFFFFF); /**< [in] Grouping flags, used by packet parser to decide what packet handler to use. 0x00-0x02 is dump, 0x03 is CSV. */

  bool finish(uint16_t apid) override;
  bool fill(char in) override;
  bool fillu16(uint16_t in) override;
  bool fillu32(uint32_t in) override;
  bool fillu64(uint64_t in) override;
  bool fillfp (float f) override;
  bool metaDoc() override;
};

#endif

