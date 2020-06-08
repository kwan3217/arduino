#ifndef packet_h
#define packet_h
#include "Arduino.h"
#include <Packet.h>
//#define DEBUG

/*uBlox packets are:
 2-byte sync marker 0xB5 0x62 (\mu b in CP437)
 1-byte class
 1-byte message id
 2-byte length
 payload
 fletcher16 checksum of class through payload

All values are little-endian.

We will treat a uBlox class/id pair as a 16-bit apid. Internally, the value will be treated as
the MSB being the class and the LSB being the id. The class ID 0x00 is not used, so we will
special-case that as if the class ID is zero, write it as 0x03, the lowest nonzero reserved
class.
*/

class uBlox: public Packet{
private:
  uint16_t current_apid;
  bool* docd;
  bool indoc;
  CharPrint docbuf; //If writing a doc packet, write it to this buffer
  CharPrint pktbuf; //Otherwise write it to this buffer
  int pkt_apid;
  int apidsize;
  bool writeDoc(uint16_t explicit_doc_apid, uint8_t field_type, uint16_t pos, const char* desc) override;
  bool writeDoc(uint8_t field_type, const char* desc) override {
    #ifdef DEBUG
    Serial.print("uBlox::writeDoc(");Serial.print(field_type);
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
    return writeDoc(doc_apid,field_type,(field_type==0?0:pktbuf.tell()),desc);
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
  uBlox(size_t Lapidsize=64, size_t Lpktsize=65536, size_t Ldocsize=512):docbuf(Ldocsize),pktbuf(Lpktsize),apidsize(Lapidsize) {
    docd=new bool[apidsize];
    for(int i=0;i<apidsize;i++) {
      docd[i]=false;
    }
  };
  virtual ~uBlox() {delete docd;};
  bool start(uint16_t apid, uint32_t TC=NULL_TC) override {
    return start(0xb5,0x62,apid,TC);
  };
  bool start(uint8_t start0, uint8_t start1, uint16_t apid, uint32_t TC=NULL_TC);
  bool finish(uint16_t apid) override;
  bool fill(char in) override;
  bool fillu16(uint16_t in) override;
  bool fillu32(uint32_t in) override;
  bool fillu64(uint64_t in) override;
  bool fillfp (float f) override;
  bool metaDoc() override;
};

#endif

