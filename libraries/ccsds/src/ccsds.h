#ifndef packet_h
#define packet_h
#include "Arduino.h"

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
  Stream& buf;
  //Write a documentation packet for a single field of a packet
  virtual bool writeDoc(uint8_t type, const char* fieldName)=0;
  //Write a documentation packet for a whole packet
  virtual bool writeDoc(              const char*   pktName)=0;
private:
  //Write a packet of English self-documentation
  virtual bool metaDoc(const char text[]) {
    if(!start(apid_metadoc,"CCSDS self-documentation"))return false;
    if(!fill(text))return false;
    if(!finish(apid_metadoc))return false;
    return true;
  };
  virtual bool metaDoc0(const char text[]) {
    if(!start(apid_metadoc,"CCSDS self-documentation"))return false;
    if(!fill(text))return false;
    return true;
  }
  virtual bool metaDoc1(const char text[]) {
    if(!fill(text))return false;
    if(!finish(apid_metadoc))return false;
    return true;
  };
  virtual bool metaDocDec(const char text0[], int val, const char text1[]) {if(!metaDoc0(text0))return false;print(val,DEC);return metaDoc1(text1);};
  virtual bool metaDocHex(const char text0[], int val, int digits, const char text1[]) {if(!metaDoc0(text0))return false;print(val,HEX,digits);return metaDoc1(text1);};
  int doc_apid;
public:
  //These follow IDL types when possible just as a reference
  enum datatype {
    t_u8    = 1, ///< Field type is unsigned 8-bit int
    t_i16   = 2; ///< Field type is signed 16-bit int
    t_i32   = 3; ///< Field type is signed 32-bit int
    t_u16   =12; ///< Field type is unsigned 16-bit int
    t_u32   =13; ///< Field type is unsigend 32-bit int
    t_i64   =14; ///< Field type is signed 64-bit int
    t_u64   =15; ///< Field type is unsigend 64-bit int
    t_float = 4; ///< Field type is IEEE754 single-precision 32-bit float
    t_double= 5; ///< Field type is IEEE754 double-precision 64-bit float
    t_string= 7; ///< Field is a byte string of arbitrary length. No length info provided, it must be provided elsewhere. Intent is UTF-8 text with replace on error.
    t_binary=10; ///< Field is a byte string of arbitrary length. No length info provided, it must be provided elsewhere. We steal the pointer type from IDL
  }
  static const uint16_t apid_doc=1; ///< Apid of a documentation packet. DO NOT USE this for a normal packet, as it triggers certain special cases.
  static const uint16_t apid_metadoc=2; ///< Apid of a metadoc packet.
  //Abstract interface -- to be implemented by derived classes
  virtual bool start(uint16_t apid /**< [in] apid of packet to be started*/, 
                     uint32_t TC=0xFFFFFFFF /**< [in] Timestamp of packet to be started, written as the secondary header. If set to default value, the packet will not get a secondary header at all.*/
                    )=0; ///< Start a packet
  virtual bool finish(uint16_t tag /**< [in] apid of packet to finish, used to check that we are finishing the same kind of packet we started*/)=0; ///< Finish a packet.
  virtual bool fillu16(uint16_t in /**< [in] value to write in packet*/)=0; ///< Write a 16-bit unsigned integer to the packet
  virtual bool fillu32(uint32_t in /**< [in] value to write in packet*/)=0; ///< Write a 32-bit unsigned integer to the packet
  virtual bool fillu64(uint64_t in /**< [in] value to write in packet*/)=0; ///< Write a 64-bit unsigned integer to the packet
  virtual bool fillfp(fp f /**< [in] value to write in packet*/)=0; ///< Write a floating-point number to the packet
  /** Write the meta-documentation for this packet format. Should create a series of packets with string payloads, the contents of which document the packet structure in English. */
  virtual bool metaDoc() {return true;}; 

  //In general below, we won't use virtual until we have a use-case. Once a
  //derived class needs to override, make that one virtual.
  Packet(Stream &Lbuf):buf(Lbuf) {}; 
  /** Start and document a packet. This creates the documentation for the packet itself, aside from all the fields of the packet.*/     
  bool start(uint16_t apid /**< [in] apid of packet to be started*/,
             const char* pktName /**< [in] packet name. Should be unique across all packets, and should be a valid identifier -- something that matches regular expression [A-Za-z_][A-Za-z0-9_]* will work in most programming languages.*/,
             uint32_t TC=0xFFFFFFFF /**< [in] Timestamp of packet to be started, written as the secondary header. If set to default value, the packet will not get a secondary header at all.*/) {
    doc_apid=apid;
    if(!writeDoc(pktName))return false;
    if(!start(apid,TC))return false;
    return true;
  };
  virtual bool fill(char in) {buf.print(in);return true;};
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
  bool fillfp (fp          value /**<[in] value to write*/,               const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_float ,fieldName))return false;return fillfp (value    );}; ///<Write and document a floating-point value
  bool fill   (const char* value /**<[in] value to write*/, uint32_t len /**<[in] length of buffer to write*/, const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_binary,fieldName))return false;return fill   (value,len);}; ///<Write and document a binary buffer of arbitrary length.
  bool fill   (const char* value /**<[in] value to write*/,                                                    const char* fieldName /**< [in] Field name. Should be a valid identifier*/) {if(!writeDoc(t_string,fieldName))return false;return fill   (value    );}; ///<Write and document a null-terminated string.
};

inline bool Packet::fill(const char* in) {
  Debug.print("Packet::fill(in=\"");
  Debug.print(in);
  Debug.println("\")");
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  } 
  return true;
};

inline bool Packet::fill(const char* in, uint32_t length) {
  Debug.print("Packet::fill(in=\"");
  for(uint32_t i=0;i<length;i++) {
    Debug.print(in[i],16,2);
  }
  Debug.println("\")");
  for(uint32_t i=0;i<length;i++) {
    if(!fill(in[i])) return false;
  }
  return true;
};

template <int B=512, int S=64>
class CCSDS: public Packet{
private:
  uint16_t seq[S];
  uint16_t lock_apid;
  bool docd[S];
  char stashbuf[B];
  int stashlen;
  int stash_apid;
  bool writeDoc(uint8_t type, const char* fieldName) override;
  bool writeDoc(              const char*   pktName) override {return writeDoc(0,pktName);};
public:
  using Packet::metaDoc;
  using Packet::filli16;
  using Packet::filli32;
  using Packet::filli64;
  using Packet::fillu16;
  using Packet::fillu32;
  using Packet::fillu64;
  using Packet::fill;
  using Packet::start;
  CCSDS(Circular &Lbuf, uint16_t* Lseq=nullptr, bool *Ldocd=nullptr, char* Lstashbuf=nullptr):Packet(Lbuf),seq(Lseq),lock_apid(0),docd(Ldocd),stashbuf(Lstashbuf),stashlen(0),stash_apid(0) {};
  bool start(uint16_t apid, uint32_t TC=0xFFFFFFFF) override;
  bool finish(uint16_t tag) override;
  bool fill(char in) override;
  bool fillu16(uint16_t in) override;
  bool fillu32(uint32_t in) override;
  bool fillu64(uint64_t in) override;
  bool fillfp (fp f) override;
  bool metaDoc() override;
};

#endif

