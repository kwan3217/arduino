class Packet {
protected: 
  Circular& buf;
  bool writeDoc(int type, const char* fieldName) {};
  bool writeDoc(          const char*   pktName) {};

  virtual bool metaDoc(const char text[]) {};
  virtual bool metaDoc(const char fmtString[], int value) {};
public:
  //Abstract interface -- to be implemented by derived classes
  virtual bool start(uint16_t apid, uint32_t TC=0xFFFFFFFF)=0;
  virtual bool finish(uint16_t tag)=0;
  virtual bool fillu16(uint16_t in)=0;
  virtual bool fillu32(uint32_t in)=0;
  virtual bool fillu64(uint64_t in)=0;
  virtual bool fillfp(fp f)=0;
  virtual bool metaDoc()=0;

  //In general below, we won't use virtual until we have a use-case. Once a
  //derived class needs to override, make that one virtual.
  Packet(Circular &Lbuf):buf(Lbuf) {};
  bool start(uint16_t apid, const char* pktName, uint32_t TC=0xFFFFFFFF) {pktApid=apid;if(!writeDoc(pktName))return false;return start(apid,TC);};
  bool fill(char in) {return buf.fill(in);};
  //The only reason I can think to override these is if we are not doing twos-complement.
  bool filli16(uint16_t in) {return fillu16((uint16_t)in);};
  bool filli32(uint32_t in) {return fillu32((uint32_t)in);};
  bool filli64(uint64_t in) {return fillu64((uint64_t)in);};
  //Might need to override these for packets where data needs escape sequences. 
  //Leave non-virtual for now.
  bool fill(const char*    value) {};
  bool fill(const char*    value, uint32_t length) {};
  //These use the rest of the interface to document each field as needed
  bool fill(      char     value,               const char* fieldName) {if(!writeDoc(t_u8    ,fieldName))return false;return fill   (value    );};
  bool filli16( int16_t    value,               const char* fieldName) {if(!writeDoc(t_i16   ,fieldName))return false;return filli16(value    );};
  bool filli32( int32_t    value,               const char* fieldName) {if(!writeDoc(t_i32   ,fieldName))return false;return filli32(value    );};
  bool filli64( int64_t    value,               const char* fieldName) {if(!writeDoc(t_i64   ,fieldName))return false;return filli64(value    );};
  bool fillu16(uint16_t    value,               const char* fieldName) {if(!writeDoc(t_u16   ,fieldName))return false;return fillu16(value    );};
  bool fillu32(uint32_t    value,               const char* fieldName) {if(!writeDoc(t_u32   ,fieldName))return false;return fillu32(value    );};
  bool fillu64(uint64_t    value,               const char* fieldName) {if(!writeDoc(t_u64   ,fieldName))return false;return fillu64(value    );};
  bool fillfp (fp          value,               const char* fieldName) {if(!writeDoc(t_float ,fieldName))return false;return fillfp (value    );};
  bool fill   (const char* value, uint32_t len, const char* fieldName) {if(!writeDoc(t_binary,fieldName))return false;return fill   (value,len);};
  bool fill   (const char* value,               const char* fieldName) {if(!writeDoc(t_string,fieldName))return false;return fill   (value    );};
};

class CCSDS: public Packet{
private:
  uint16_t *seq;
  uint16_t lock_apid;
public:
  virtual bool metaDoc(const char text[]) {Packet::metaDoc(text);};
  CCSDS(Circular &Lbuf, uint16_t* Lseq=nullptr):Packet(Lbuf),seq(Lseq),lock_apid(0) {};
  virtual bool start(uint16_t apid, uint32_t TC=0xFFFFFFFF) override;
  virtual bool finish(uint16_t tag) override;
  virtual bool fillu16(uint16_t in) override;
  virtual bool fillu32(uint32_t in) override;
  virtual bool fillu64(uint64_t in) override;
  virtual bool fillfp (fp f);
  virtual bool metaDoc();
};

bool CCSDS::metaDoc() {
  metaDoc("Hey there");
}

int main() {
  CCSDS pkt;
  pkt.metaDoc();
}
