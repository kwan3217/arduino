#ifndef packet_h
#define packet_h
#include "Arduino.h"
//#define DEBUG

const char dchar[]="0123456789ABCDEF";

inline void atoi_digits(char* buf, unsigned int val, unsigned int digits, unsigned int radix=10, char pad='0') {
  unsigned int powcheck=1;
  for(unsigned int i=0;i<digits-1;i++)powcheck*=radix;
  for(unsigned int i=0;i<digits;i++) {
    unsigned int d=val/powcheck;
    val%=powcheck;
    if(d==0) {
      buf[i]=pad;
    } else {
      buf[i]=dchar[d];
      pad=dchar[0];
    }
    powcheck/=radix;
  }
}

class CharPrint:public Print {
private:
  char* buf;
  size_t ptr;
  size_t size;
public:
  using Print::print;
  using Print::println;
  CharPrint(size_t Lsize):ptr(0),size(Lsize) {buf=(char*)malloc(size);};
  virtual ~CharPrint() {free(buf);};
  virtual size_t write(uint8_t c) {
    #ifdef DEBUG
    Serial.print("CharPrint::write(this=");Serial.print((uint32_t)this,HEX);Serial.print(",");
    Serial.print("\"\\x");
    Serial.print(dchar[c/16]);
    Serial.print(dchar[c%16]);
    Serial.println("\")");
    #endif
    if(ptr<size) {
      #ifdef DEBUG
      Serial.print("ptr(");Serial.print(ptr);Serial.print(")<");
      Serial.print("size(");Serial.print(size);Serial.println(")");
      Serial.print("&buf=");Serial.println((uint32_t)buf,HEX);
      Serial.print("buf before=\"");
      for(size_t i=0;i<ptr;i++) {
        Serial.print("\\x");
        Serial.print(dchar[buf[i]/16]);
        Serial.print(dchar[buf[i]%16]);
      }
      Serial.println("\"");
      Serial.print("ptr before=");Serial.println(ptr);
      #endif
      buf[ptr]=c;
      ptr++;
      #ifdef DEBUG
      Serial.print("buf after=\"");
      for(size_t i=0;i<ptr;i++) {
        Serial.print("\\x");
        Serial.print(dchar[buf[i]/16]);
        Serial.print(dchar[buf[i]%16]);
      }
      Serial.println("\"");
      Serial.print("ptr after=");Serial.println(ptr);
      #endif
      return 1;
    } 
    return 0;
  };
  size_t drain(Print* target) {
    #ifdef DEBUG
    Serial.print("drain(target)");
    #endif
    size_t result=target->write(buf,ptr);
    #ifdef DEBUG
    Serial.print("target.write(\"");
    for(size_t i=0;i<ptr;i++) {
      Serial.print("\\x");
      Serial.print(dchar[buf[i]/16]);
      Serial.print(dchar[buf[i]%16]);
    }
    Serial.print("\",");
    Serial.print(ptr,DEC);
    Serial.print(")=");
    Serial.println(result,DEC);
    #endif
    ptr=0;
    return result;
  };
  size_t tell() {return ptr;};
  size_t seek(size_t Lptr) {if(Lptr<size) ptr=Lptr;return ptr;};
  void clear() {ptr=0;};
  char& operator[](size_t pos) {return buf[pos];};
};

class Packet {
  //Implement a packet writer. A packet is a single unit of data intended to be atomic,
  //IE either the whole packet arrives perfectly or none of it does. Packets can have
  //a variable length. Packets can have different types, called apid (pronounced
  //App-I-D, plural Apids) after the concept in the CCSDS Space Packet Protocol. 
  //Conventionally a packet represents a single instance of a structure, and all 
  //packets with the same apid have the same structure, with fields of the same type.

  //This packet writer supports machine-readable self-documentation. Before the first
  //instance of each packet apid, a number of documentation packets appear which 
  //completely describe the packet. The packet stream may also include a first packet
  //which contains a script which can parse the packet stream.

  //There are two kinds of documentation packets, doc and metadoc. 

  //Doc packets describe each field in a machine-readable manner, so that a packet
  //parser can parse other packets based on doc packets. Naturally, doc packets
  //must come before each packet that is being documented, and here is where you
  //have to be careful. "A beginning is the time for taking the most delicate 
  //care that the balances are correct." It is not possible to machine-read the doc 
  //packets with doc packets, because there is a boostrap problem. The parser will
  //have to have the definition of a doc packet pre-available, probably encoded
  //directly into its code. 

  //Metadoc packets are intended to be human-readable through whatever the raw 
  //format of the packet is, and as near the beginning of the packet stream as
  //possible. The intent is to describe the general packet structure and 
  //documentation packets in English, so that anyone who comes across the packet
  //stream will be able to bootstrap a packet reader. Therefore, metadoc packets
  //are always one field with plain text. Metadoc packets can be documented 
  //themselves, so that a parser can at least read around them. 

  //So we have doc packets documented in English in the metadoc packets, and 
  //metadoc packets documented machine-readably in doc packets. Yo Dawg.... We 
  //can have a few doc packets before the first metadoc packet, as long as there
  //is readable text in the first few lines of a stream dump.

  //Note that metadoc is most important just for describing the packet structure
  //in general and doc packets in particular, not necessarily any particular 
  //data in the file.

  //My wild fantasy is that if this file were found engraved on metal plates
  //or stone tablets 1000 years in the future, nothing else would be necessary
  //to decode it. Reading the packet file will require the ability to decode
  //ASCII data, read English, and perhaps execute a Python 3.6 script. 

  //Although not a part of the code of this library, it would be helpful if the 
  //encoding program embedded a copy of its source code in the packet stream.
  //Decoding that source code would require the availablity
protected: 
  int doc_apid;
  Print* ouf;
  //Write a documentation packet for a single field of a packet
  virtual bool writeDoc(uint8_t type, const char* fieldName)=0;
  //Write a documentation packet for a whole packet
  virtual bool writeDoc(              const char*   pktName)=0;
  //Write a packet of English self-documentation
  virtual bool metaDoc(const char text[]) {
    #ifdef DEBUG
    Serial.print("Packet::metaDoc(\"");
    Serial.print(text);
    Serial.println("\")");
    #endif
    if(!start(apid_metadoc,"CCSDS self-documentation"))return false;
    if(!fill(text,"Self-documentation text"))return false;
    if(!finish(apid_metadoc))return false;
    return true;
  };
  virtual bool metaDoc(const char text0[], int val, int radix, int digits, const char text1[]) {
    char buf[16];buf[digits]=0;atoi_digits(buf,val,digits,radix,'0');
    #ifdef DEBUG
    Serial.print("Packet::metaDocDec(\"");
    Serial.print(text0);
    Serial.print("\",");
    Serial.print(buf);
    Serial.print(",");
    Serial.print(radix);
    Serial.print(",");
    Serial.print(digits);
    Serial.print(",\"");
    Serial.print(text1);
    Serial.println(",\")");
    #endif
    if(!start(apid_metadoc,"CCSDS_selfdoc")) {
      #ifdef DEBUG
      Serial.println("Packet::metaDoc(char[],int,int,int,char[])=0 //start() falied");
      #endif
      return false;
    }
    if(!fill(text0)) {
      #ifdef DEBUG
      Serial.println("Packet::metaDoc(char[],int,int,int,char[])=0 //fill(text0) falied");
      #endif
      return false;
    }
    if(!fill(buf,digits)) {
      #ifdef DEBUG
      Serial.println("Packet::metaDoc1(char[],int,int,int,char[])=0 //fill(buf,digits) failed");
      #endif
      return false;
    }
    if(!fill(text1)) {
      #ifdef DEBUG
      Serial.println("Packet::metaDoc(char[],int,int,int,char[])=0 //fill(text1) falied");
      #endif
      return false;
    }
    if(!finish(apid_metadoc)) {
      #ifdef DEBUG
      Serial.println("Packet::metaDoc1()=0 //finish(apid_metadoc) failed");
      #endif
      return false;
    }
    #ifdef DEBUG
    Serial.println("Packet::metaDoc()=1 //success");
    #endif
    return true;
  };
public:
  //These follow IDL types when possible just as a reference
  static const uint8_t t_u8    = 1; ///< Field type is unsigned 8-bit int
  static const uint8_t t_i16   = 2; ///< Field type is signed 16-bit int
  static const uint8_t t_i32   = 3; ///< Field type is signed 32-bit int
  static const uint8_t t_u16   =12; ///< Field type is unsigned 16-bit int
  static const uint8_t t_u32   =13; ///< Field type is unsigend 32-bit int
  static const uint8_t t_i64   =14; ///< Field type is signed 64-bit int
  static const uint8_t t_u64   =15; ///< Field type is unsigend 64-bit int
  static const uint8_t t_float = 4; ///< Field type is IEEE754 single-precision 32-bit float
  static const uint8_t t_double= 5; ///< Field type is IEEE754 double-precision 64-bit float
  static const uint8_t t_string= 7; ///< Field is a byte string of arbitrary length. No length info provided, it must be provided elsewhere. Intent is UTF-8 text with replace on error.
  static const uint8_t t_binary=10; ///< Field is a byte string of arbitrary length. No length info provided, it must be provided elsewhere. We steal the pointer type from IDL
  static const uint16_t apid_doc=1; ///< Apid of a documentation packet. DO NOT USE this for a normal packet, as it triggers certain special cases.
  static const uint16_t apid_metadoc=2; ///< Apid of a metadoc packet.
  void begin(Print& Louf) {ouf=&Louf;};
  //Abstract interface -- to be implemented by derived classes
  virtual bool start(uint16_t apid /**< [in] apid of packet to be started*/, 
                     uint32_t TC=0xFFFFFFFF /**< [in] Timestamp of packet to be started, written as the secondary header. If set to default value, the packet will not get a secondary header at all.*/
                    )=0; ///< Start a packet
  virtual bool finish(uint16_t tag /**< [in] apid of packet to finish, used to check that we are finishing the same kind of packet we started*/)=0; ///< Finish a packet.
  virtual bool fillu16(uint16_t in /**< [in] value to write in packet*/)=0; ///< Write a 16-bit unsigned integer to the packet
  virtual bool fillu32(uint32_t in /**< [in] value to write in packet*/)=0; ///< Write a 32-bit unsigned integer to the packet
  virtual bool fillu64(uint64_t in /**< [in] value to write in packet*/)=0; ///< Write a 64-bit unsigned integer to the packet
  virtual bool fillfp (float    in /**< [in] value to write in packet*/)=0; ///< Write a floating-point number to the packet
  /** Write the meta-documentation for this packet format. Should create a series of packets with string payloads, the contents of which document the packet structure in English. */
  virtual bool metaDoc() {return true;}; 

  //In general below, we won't use virtual until we have a use-case. Once a
  //derived class needs to override, make that one virtual.
  /** Start and document a packet. This creates the documentation for the packet itself, aside from all the fields of the packet.*/     
  bool start(uint16_t apid /**< [in] apid of packet to be started*/,
             const char* pktName /**< [in] packet name. Should be unique across all packets, and should be a valid identifier -- something that matches regular expression [A-Za-z_][A-Za-z0-9_]* will work in most programming languages.*/,
             uint32_t TC=0xFFFFFFFF /**< [in] Timestamp of packet to be started, written as the secondary header. If set to default value, the packet will not get a secondary header at all.*/,
             uint8_t  Grp=0x03) {
    #ifdef DEBUG
    Serial.print("Packet::start(");Serial.print(apid);Serial.print(",\"");Serial.print(pktName);Serial.print("\",");Serial.print(TC,HEX);Serial.println(")");
    #endif
    doc_apid=apid;
    if(!writeDoc(pktName))return false;
    if(!start(apid,TC))return false;
    return true;
  };
  virtual bool fill(char in) {ouf->print(in);return true;};
  //The only reason I can think to override these is if we are not doing twos-complement.
  bool filli16(uint16_t in) {return fillu16((uint16_t)in);};///< Write a 16-bit signed integer to the packet. Default implementation casts the value to an unsigned int and writes that.
  bool filli32(uint32_t in) {return fillu32((uint32_t)in);};///< Write a 32-bit signed integer to the packet. Default implementation casts the value to an unsigned int and writes that.
  bool filli64(uint64_t in) {return fillu64((uint64_t)in);};///< Write a 32-bit signed integer to the packet. Default implementation casts the value to an unsigned int and writes that.
  //Might need to override these for packets where data needs escape sequences. 
  //Leave non-virtual for now.
  bool fill(const char*    value);                  ///< Write a null-terminated string.            Note that this does nothing to record the length itself -- you may need to write the length separately.
  bool fill(const char*    value, uint32_t length); ///< Write a binary buffer of arbitrary length. Note that this does nothing to record the length itself -- you may need to write the length separately.
  //These use the rest of the interface to document each field as needed
  bool fill(      char     value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_u8    ,fieldName))return false;return fill   (value    );}; ///<Write and document an 8-bit value
  bool filli16( int16_t    value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_i16   ,fieldName))return false;return filli16(value    );}; ///<Write and document a signed 16-bit value
  bool filli32( int32_t    value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_i32   ,fieldName))return false;return filli32(value    );}; ///<Write and document a signed 32-bit value
  bool filli64( int64_t    value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_i64   ,fieldName))return false;return filli64(value    );}; ///<Write and document a signed 64-bit value
  bool fillu16(uint16_t    value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_u16   ,fieldName))return false;return fillu16(value    );}; ///<Write and document an unsigned 16-bit value
  bool fillu32(uint32_t    value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_u32   ,fieldName))return false;return fillu32(value    );}; ///<Write and document an unsigned 32-bit value
  bool fillu64(uint64_t    value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_u64   ,fieldName))return false;return fillu64(value    );}; ///<Write and document an unsigned 64-bit value
  bool fillfp (float        value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_float ,fieldName))return false;return fillfp (value    );}; ///<Write and document a floating-point value
  bool fill   (const char* value /**<[in] value to write*/, uint32_t len /**<[in] length of buffer to write*/, const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_binary,fieldName))return false;return fill   (value,len);}; ///<Write and document a binary buffer of arbitrary length.
  bool fill   (const char* value /**<[in] value to write*/,                                                    const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_string,fieldName))return false;return fill   (value    );}; ///<Write and document a null-terminated string.
};

inline bool Packet::fill(const char* in) {
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  } 
  return true;
};

inline bool Packet::fill(const char* in, uint32_t length) {
  for(uint32_t i=0;i<length;i++) {
    if(!fill(in[i])) return false;
  }
  return true;
};

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
  bool writeDoc(uint16_t apid, uint8_t type, uint16_t pos, const char* desc);
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
  bool script();
};

#endif

