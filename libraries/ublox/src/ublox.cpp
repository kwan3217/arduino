#include "ublox.h"
#include "Arduino.h"

bool uBlox::writeDoc(uint16_t explicit_doc_apid, uint8_t field_type, uint16_t pos, const char* desc) {
  #ifdef DEBUG
  Serial.print("uBlox::writeDoc(explicit_doc_apid=0x");Serial.print(explicit_doc_apid,HEX);
                 Serial.print(",field_type=0x");Serial.print(field_type,HEX);
                 Serial.print(",pos=0x");Serial.print(pos,HEX);
                 Serial.print(",desc=\"");Serial.print(desc);Serial.println("\");");
  #endif
  if(!start(apid_doc)) return false;
  //uint16_t packet APID being described
  //Split the apid into class and message id, and handle the default class
  uint8_t cls=(explicit_doc_apid >> 8) & 0xFF;
  uint8_t id =(explicit_doc_apid >> 0) & 0xFF;
  if(cls==0) cls=0x03;

  if(!fill(cls)) return false;
  if(!fill(id )) return false;
  //uint16_t position in the packet of the field being described, zero if the whole packet is being named
  if(!fillu16(pos)) return false;
  //uint8_t type of the field
  if(!fill(field_type)) return false;
  //string field description
  if(!fill(desc)) return false;
  if(!finish(apid_doc)) return false;
  return true;
}

bool uBlox::start(uint8_t start0, uint8_t start1, uint16_t apid, uint32_t TC) {
  #ifdef DEBUG
  Serial.print("uBlox::start(start0=");Serial.print(start0,HEX);
              Serial.print(",start1=");Serial.print(start1,HEX);
              Serial.print(",apid=");Serial.print(apid,HEX);
              Serial.print(",TC=");Serial.print(TC,HEX);
  Serial.println(")");
  #endif
  //Split the apid into class and message id, and handle the default class
  uint8_t cls=(apid >> 8) & 0xFF;
  uint8_t id =(apid >> 0) & 0xFF;
  if(cls==0) cls=0x03;
  indoc=(apid==apid_doc);
  #ifdef DEBUG
  Serial.print("apid_doc=");Serial.println(apid_doc);
  Serial.print("indoc=");Serial.println(indoc);
  #endif
  CharPrint& buf=indoc?docbuf:pktbuf;
  buf.clear();
  if(!indoc) {
    if(current_apid>0) return false;
    current_apid=apid;
  }
  if(!fill(start0)) return false;
  if(!fill(start1)) return false;
  if(!fill(cls)) return false;
  if(!fill(id)) return false;
  if(!fillu16(0xDEAD)) return false; //Reserve space in the packet for length
  if(TC!=NULL_TC) {
    //Secondary header: count of subseconds since beginning of minute
    if(!fillu32(TC,"TC")) return false;
  }
  return true;
}

bool uBlox::finish(uint16_t apid) {
  #ifdef DEBUG
  Serial.print("uBlox::finish(0x");Serial.print(apid,HEX);
  Serial.print("), indoc=");Serial.println(indoc);
  #endif
  CharPrint& buf=indoc?docbuf:pktbuf;
  size_t len=buf.tell()-6;
  buf[4]=(len >> 0) & 0xFF;
  buf[5]=(len >> 8) & 0xFF;
  uint8_t CK_A=0;
  uint8_t CK_B=0;
  for(auto i=2;i<buf.tell();i++) {
    CK_A+=reinterpret_cast<uint8_t&>(buf[i]);
    CK_B+=CK_A;
  }
  if(!fill(CK_A)) return false;
  if(!fill(CK_B)) return false;
  buf.drain(ouf);
  if(indoc && apid==apid_doc) {
    indoc=false;
    return true;
  } else {
    if(current_apid==apid) {
      if(apid<apidsize) docd[apid]=true;
      current_apid=0;
      return true;
    } else {
      return false;
    }
  }    
}

bool uBlox::fill(char c) {
  #ifdef DEBUG
  Serial.print("uBlox::fill(");
  if((c<0x20)||(c>0x7E)) {
    Serial.print("(char)0x");Serial.print(c,HEX);
  } else {
    Serial.print("'");Serial.print(c);Serial.print("'");
  }
  Serial.print("), indoc=");Serial.println(indoc);
  #endif
  CharPrint& buf=indoc?docbuf:pktbuf;
  buf.print(c);
  return true;
}

//Fill in Little-endian order as specified by uBlox 32.3.2
bool uBlox::fillu16(uint16_t in) {
  if(!fill((char)((in >> 0) & 0xFF))) return false;
  if(!fill((char)((in >> 8) & 0xFF))) return false;
  return true;
}

bool uBlox::fillu32(uint32_t in) {
  if(!fill((char)((in >>  0) & 0xFF))) return false;
  if(!fill((char)((in >>  8) & 0xFF))) return false;
  if(!fill((char)((in >> 16) & 0xFF))) return false;
  if(!fill((char)((in >> 24) & 0xFF))) return false; 
  return true;
}

bool uBlox::fillu64(uint64_t in) {
  if(!fill((char)((in >>  0) & 0xFF))) return false;
  if(!fill((char)((in >>  8) & 0xFF))) return false;
  if(!fill((char)((in >> 16) & 0xFF))) return false;
  if(!fill((char)((in >> 24) & 0xFF))) return false;
  if(!fill((char)((in >> 32) & 0xFF))) return false;
  if(!fill((char)((in >> 40) & 0xFF))) return false;
  if(!fill((char)((in >> 48) & 0xFF))) return false;
  if(!fill((char)((in >> 56) & 0xFF))) return false;
  return true;
}

bool uBlox::fillfp(float f) {
  //The uBlox spec requires big-endian notation
  //for all header fields, and we follow that lead
  //for all payload fields, including floating-point.
  //The in-memory form is little-endian, so we
  //do a reinterpret_cast from float to uint32_t
  //and write that int in big-endian.
  return fillu32(reinterpret_cast<uint32_t&>(f));
}

bool uBlox::metaDoc() {
  #ifdef DEBUG
  Serial.println("uBlox::metaDoc()");
  #endif
  if(!script()) return false;
  if(!writeDoc(apid_doc,0,0,"packetdoc")) return false;
  if(!writeDoc(apid_doc,t_u8,6,"descClass")) return false;
  if(!writeDoc(apid_doc,t_u8,7,"descId")) return false;
  if(!writeDoc(apid_doc,t_u16,8,"descPos")) return false;
  if(!writeDoc(apid_doc,t_u8,10,"descType")) return false;
  if(!writeDoc(apid_doc,t_string,11,"descName")) return false;
  if(!writeDoc(0x2323,0,0,"Script")) return false;
  if(!writeDoc(0x2323,6,t_string,"script")) return false;
  return true;
}

bool uBlox::script() {
  if(!start('#','#',0x2323)) return false;
  if(!fill(
"Run this with:\n"
"#bash <filename>.SDS\n"
"python3 - $0 << EOF\n"
"EOF\n"
"exit\n"
)) return false;
  if(!finish(0x2323)) return false;
  return true;
}
          


