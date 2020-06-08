#include "ccsds.h"
#include "Arduino.h"

bool CCSDS::writeDoc(uint16_t apid, uint8_t type, uint16_t pos, const char* desc) {
  if(!start(apid_doc)) {
    #ifdef DEBUG
    Serial.print("Failed start(");Serial.print(apid_doc);Serial.print("), returning false");
    #endif
    return false;
  }
  //uint16_t packet APID being described
  if(!fillu16(apid)) {
    #ifdef DEBUG
    Serial.print("Failed fillu16(doc_apid=");Serial.print(apid);Serial.print("), returning false");
    #endif
    return false;
  }
  //uint16_t position in the packet of the field being described, zero if the whole packet is being named
  if(!fillu16(pos))  {
    #ifdef DEBUG
    Serial.print("Failed fillu16(pos=");Serial.print(pos);Serial.print("), returning false");
    #endif
    return false;
  }
  //uint8_t type of the field
  if(!fill(type)) {
    #ifdef DEBUG
    Serial.print("Failed fill(type=");Serial.print(type,DEC);Serial.print("), returning false");
    #endif
    return false;
  }
  //string field description
  if(!fill(desc)) {
    #ifdef DEBUG
    Serial.print("Failed fill(desc=\"");Serial.print(desc);Serial.print("\"), returning false");
    #endif
    return false;
  }
  if(!finish(apid_doc)) {
    #ifdef DEBUG
    Serial.print("Failed finish(apid_doc=");Serial.print(apid_doc);Serial.print("), returning false");
    #endif
    return false;
  }
  return true;
}

bool CCSDS::start(uint16_t apid, uint8_t Ver, uint8_t Type, uint8_t Grp, uint16_t Seq, uint32_t TC) {
  #ifdef DEBUG
  Serial.print("CCSDS::start(this=");Serial.print((uint32_t)this,HEX);Serial.print(",apid=");Serial.print(apid);Serial.print(",");Serial.print(TC,HEX);Serial.println(")");
  #endif
  indoc=(apid==apid_doc);
  #ifdef DEBUG
  Serial.print("apid=");Serial.println(apid);
  Serial.print("apid_doc=");Serial.println(apid_doc);
  Serial.print("indoc=");Serial.println(indoc);
  #endif
  CharPrint& buf=indoc?docbuf:pktbuf;
  #ifdef DEBUG
  Serial.print("docbuf=");Serial.println((uint32_t)&docbuf,HEX);
  Serial.print("pktbuf=");Serial.println((uint32_t)&pktbuf,HEX);
  Serial.print("&buf=");Serial.println((uint32_t)&buf,HEX);
  #endif
  buf.clear();
  if(!indoc) {
    if(current_apid>0) return false;
    current_apid=apid;
  }
  int Sec=(TC!=0xFFFFFFFFU)?1:0;  
  //const int Grp=0x03;  //Grouping flags - 3->not in a group of packets
                 //data       len        lowbit
  uint16_t word=(((apid & ((1<<11)-1)) <<  0) | 
                 ((Sec  & ((1<< 1)-1)) << 11) | 
                 ((Type & ((1<< 1)-1)) << 12) | 
                 ((Ver  & ((1<< 3)-1)) << 13));
  if(!fillu16(word)) return false;
        //data       len        lowbit
  word=(((Seq & ((1<<14)-1)) <<  0) | 
        ((Grp & ((1<< 2)-1)) << 14));
  if(!fillu16(word)) return false;
  word=0xDEAD;
  if(!fillu16(word)) return false; //Reserve space in the packet for length
  if(Sec) {
    //Secondary header: count of subseconds since beginning of minute
    if(!fillu32(TC,"TC")) return false;
  }
  return true;
}

bool CCSDS::finish(uint16_t apid) {
  CharPrint& buf=indoc?docbuf:pktbuf;
  size_t len=buf.tell()-7;
  buf[4]=(len >> 8) & 0xFF;
  buf[5]=(len >> 0) & 0xFF;
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

bool CCSDS::fill(char c) {
  CharPrint& buf=indoc?docbuf:pktbuf;
  buf.print(c);
  return true;
}

//Fill in Big-endian order as specified by CCSDS 102.0-B-5, 1.6a
bool CCSDS::fillu16(uint16_t in) {
  if(!fill((char)((in >> 8) & 0xFF))) return false;
  if(!fill((char)((in >> 0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fillu32(uint32_t in) {
  if(!fill((char)((in >> 24) & 0xFF))) return false;
  if(!fill((char)((in >> 16) & 0xFF))) return false;
  if(!fill((char)((in >>  8) & 0xFF))) return false;
  if(!fill((char)((in >>  0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fillu64(uint64_t in) {
  if(!fill((char)((in >> 56) & 0xFF))) return false;
  if(!fill((char)((in >> 48) & 0xFF))) return false;
  if(!fill((char)((in >> 40) & 0xFF))) return false;
  if(!fill((char)((in >> 32) & 0xFF))) return false;
  if(!fill((char)((in >> 24) & 0xFF))) return false;
  if(!fill((char)((in >> 16) & 0xFF))) return false;
  if(!fill((char)((in >>  8) & 0xFF))) return false;
  if(!fill((char)((in >>  0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fillfp(float f) {
  //The CCSDS spec requires big-endian notation
  //for all header fields, and we follow that lead
  //for all payload fields, including floating-point.
  //The in-memory form is little-endian, so we
  //do a reinterpret_cast from float to uint32_t
  //and write that int in big-endian.
  return fillu32(reinterpret_cast<uint32_t&>(f));
}

bool CCSDS::metaDoc() {
  #ifdef DEBUG
  Serial.println("CCSDS::metaDoc()");
  #endif
  if(!script()) return false;
  if(!metaDoc("This file contains CCSDS packets as described in CCSDS 133.0-B-1 with updates."))return false;
  if(!metaDoc("https://public.ccsds.org/Pubs/133x0b1c2.pdf"))return false;
  if(!metaDoc("There are no padding bytes or sync markers between packets."))return false;
  if(!metaDoc("All packets are an integer number of whole bytes."))return false;
  if(!metaDoc("Each packet starts with a six-byte header which holds the packet "
		   "type (APID), sequence number and length."))return false;
  if(!metaDoc("All numbers in a CCSDS packet are stored Big-Endian."))return false;
  if(!metaDoc("Packets start with a 16-bit unsigned integer, upper 5 bits are "
		   "version, type, and secondary header presence."))return false;
  if(!metaDoc("Lower 11 bits are the APID."))return false;
  if(!metaDoc("Next is a 16-bit number with the upper 2 bits set (flags indicating unfragmented packet) "
		   "and the lower 14 bits are a sequence number which increments for each APID and "
		   "wraps in 14 bits."))return false;
  if(!metaDoc("Next 16-bit number is length of packet minus 7 since every packet "
		   "has a 6-byte header and must have at least 1 byte of payload."))return false;
  /*
  if(!metaDocDec("Packets of apid ",Apids::parse," encapsulates a python 3 script which can decode this stream into "
                   "separate CSV streams for each apid. The apid and length are correct for this "
                   "type of packet, but other flags may not be."))return false;
  */
  if(!metaDoc("Packets of apid ",apid_metadoc,DEC,1," contain this English meta-documentation. "))return false;
  if(!metaDoc("Packets of apid ",apid_doc,DEC,1," describe the detailed format of the other packets."))return false;
  if(!metaDoc("In those packets, the first field in the payload is a 16-bit number, the "
		  "packet APID being described."))return false;
  if(!metaDoc("The second field in the payload is a 16-bit number, the position in the "
		  "packet of the field being described."))return false;
  if(!metaDoc("This value is zero if the whole packet is being named."))return false;
  if(!metaDoc("Otherwise the position is a zero-based count of bytes from the beginning "
		  "of the packet, and will never be less than six."))return false;
  if(!metaDoc("This value will be unreliable for fields written after a variable length"
		  "string or binary, but usually such fields are the last field in a packet."))return false;
  if(!metaDoc("For packets with multiple variable-length fields, the position written "
		  "follows from the first packet of this APID to be written"))return false;
  if(!metaDoc("The third field in the payload is an 8-bit number, the type of the field"
		  "from the following table: "))return false;
  if(!metaDoc("0x",0       ,HEX,2,": This name describes the whole packet, not any one field"))return false;
  if(!metaDoc("0x",t_u8    ,HEX,2,": t_u8 (8-bit unsigned integer)"))return false;
  if(!metaDoc("0x",t_i16   ,HEX,2,": t_i16 (16-bit signed integer)"))return false;
  if(!metaDoc("0x",t_i32   ,HEX,2,": t_i32 (32-bit signed integer)"))return false;
  if(!metaDoc("0x",t_u16   ,HEX,2,": t_u16 (16-bit unsigned integer)"))return false;
  if(!metaDoc("0x",t_u32   ,HEX,2,": t_u32 (32-bit unsigned integer)"))return false;
  if(!metaDoc("0x",t_float ,HEX,2,": t_float (32-bit IEEE-754 floating point)"))return false;
  if(!metaDoc("0x",t_double,HEX,2,": t_double (64-bit IEEE-754 floating point)"))return false;
  if(!metaDoc("0x",t_string,HEX,2,": t_string (UTF-8 text)"))return false;
  if(!metaDoc("0x",t_binary,HEX,2,": t_binary (unformatted data dump)"))return false;
  if(!metaDoc("The fourth field is a UTF-8 text string with the name of the field."))return false;
  if(!metaDoc("For all strings and binary data, no length information is included."))return false;
  if(!metaDoc("If the string or binary is the last such field in the packet, its "
		  "length can be deduced from the packet length."))return false;
  if(!metaDoc("Otherwise the packet will need some indication of the length."))return false;
  if(!metaDoc("UTF-8 text might have a null-termination, but binary data will usually need"
		  "a prefix length, which is described in its own field."))return false;
  if(!metaDoc("UTF-8 length descriptions, if present, will be of the number of bytes, not code points."))return false;
  if(!metaDoc("Normally text will be ASCII, meaning that the high bit will be cleared."))return false;
  if(!metaDoc("UTF-8 is chosen merely to give a concrete interpretation to set upper-bits."))return false;
  if(!metaDoc("Be prepared for errors in any string -- use a protocol like error='replace' in Python."))return false;
  if(!metaDoc("Although the Space Packet Protocol allows it, this packet writer does "
		  "not leave gaps between fields -- IE all fields are contiguous"))return false;
  if(!metaDoc("Although the Space Packet Protocol allows fields on non-byte boundaries, "
		  "this packet writer always writes whole-byte fields."))return false;
  if(!metaDoc("This packet stream may contain \"dump\" packets. The payload of all these packets "
          "concatenated together in order form a TAR stream compressed with the ZPAQ "
          "algorithm."))return false;
  if(!metaDoc("That TAR stream contains the source code for the program used to write this "
          "packet stream, along with all other files the authors felt necessary to "
          "make a \"self-documenting robot\"."))return false;
  if(!writeDoc(apid_metadoc,0,0,"metadoc")) return false;
  if(!writeDoc(apid_metadoc,t_string,6,"metadoc")) return false;
  if(!writeDoc(apid_doc,0,0,"packetdoc")) return false;
  if(!writeDoc(apid_doc,t_u16,6,"descApid")) return false;
  if(!writeDoc(apid_doc,t_u16,8,"descPos")) return false;
  if(!writeDoc(apid_doc,t_u8,10,"descType")) return false;
  if(!writeDoc(apid_doc,t_string,11,"descName")) return false;
  return true;
}

bool CCSDS::script() {
  if(!start(0x323,1,0,0,0x2323)) return false;
  if(!fill(
"#Run this with: \n"
"#bash <filename>.SDS\n"
"python3 - $0 << EOF\n"
"import struct\n"
"import os.path\n"
"import sys\n"
"from collections import OrderedDict\n"
"if __name__ == '__main__':\n"
"    t_u8=1\n"
"    t_i16=2\n"
"    t_i32=3\n"
"    t_float=4\n"
"    t_double=5\n"
"    t_string=7\n"
"    t_bin=0x0A\n"
"    t_u16=0x0C\n"
"    t_u32=0x0D\n"
"                #0  1    2    3    4    5   6  7   8  9  A   B   C    D\n"
"                #   u8  i16  i32  f32  f64    str       bin    u16   u32\n"
"    lookupType=['','B','>h','>i','>f','>d','','s','','','c','','>H','>I']\n"
"    lookupSize=[ 0, 1,   2,   4,   4,   8,  0, 0,  0, 0, 0,  0,  2,   4 ]\n"
"    infn=sys.argv[1]\n"
"    base=os.path.dirname(infn)\n"
"    if base=='':\n"
"        base='.'\n"
"    base+='/'+'.'.join((os.path.basename(infn).split('.'))[:-1])+'/'\n"
"    try:\n"
"        os.mkdir(base)\n"
"    except FileExistsError:\n"
"        pass #This is OK\n"
"    with open(infn,'rb') as inf:\n"
"        done=False\n"
"        pktdesc=OrderedDict() #Dictionary of packet descriptions keyed on apid (numeric)\n"
"        #Preload apid 0x001 (field description bootstrap)\n"
"        apidfirst=OrderedDict()\n"
"        def csv(pkt):\n"
"            oufn=base+'/'+pktdesc[pkt['apid']]['name']+'.csv'\n"
"            if pkt['apid'] not in apidfirst:\n"
"                with open(oufn,'w') as ouf:\n"
"                    first = True\n"
"                    apidfirst[pkt['apid']]=False\n"
"                    for k,v in pkt.items():\n"
"                        if k!='apid':\n"
"                            print(('' if first else ',')+k,file=ouf,end='')\n"
"                            first=False\n"
"                    print('',file=ouf)\n"
"            with open(oufn,'a') as ouf:\n"
"                first=True\n"
"                for k,v in pkt.items():\n"
"                    if k!='apid':\n"
"                        last=str(v)\n"
"                        print(('' if first else ',')+last,file=ouf,end='')\n"
"                        first=False\n"
"                if last[-1]!='\\n':\n"
"                    print('',file=ouf) #Only put an end of line if there wasn't one in the data already\n"
"        def dump(pkt):\n"
"            oufn=base+'/'+pktdesc[pkt['apid']]['name']+'.dump'\n"
"            with open(oufn,'ab' if pkt['apid'] in apidfirst else 'wb') as ouf:\n"
"                ouf.write(pkt['DumpData'])\n"
"            apidfirst[pkt['apid']]=False\n"
"        def addField(pkt):\n"
"            '''\n"
"            Add a field description to a packet. If the packet describes a whole packet, check if the packet description\n"
"            exists yet. If it does, just change the name, otherwise create it fresh.\n"
"            '''\n"
"            print(pkt)\n"
"            if pkt['descType']==0:\n"
"                if pkt['descApid'] in pktdesc:\n"
"                    pktdesc[pkt['descApid']]['name']=pkt['descName']\n"
"                else:\n"
"                    pktdesc[pkt['descApid']]={'name':pkt['descName'],'handler':csv if pkt['descName']!='Dump' else dump,'fields':OrderedDict()}\n"
"            else:\n"
"                pktdesc[pkt['descApid']]['fields'][pkt['descName']]={'pos':pkt['descPos'],'type':pkt['descType']}\n"
"        pktdesc[1]={'name':  'PacketDescription',\n"
"                     'handler':addField,\n"
"                     'fields':OrderedDict({'descApid':{'pos': 6,'type':t_u16},\n"
"                               'descPos': {'pos': 8,'type':t_u16},\n"
"                               'descType':{'pos':10,'type':t_u8},\n"
"                               'descName':{'pos':11,'type':t_string}})}\n"
"        pktdesc[803]={'name':  'Script',\n"
"                     'handler':dump,\n"
"                     'fields':OrderedDict({'DumpData':{'pos': 6,'type':t_bin}})}\n"
"        while not done:\n"
"            header=inf.read(6)\n"
"            if len(header)<6:\n"
"                done=True\n"
"                continue\n"
"            apid=struct.unpack('>H',header[0:2])[0] & ((1<<11)-1)\n"
"            pktlen=struct.unpack('>H',header[4:6])[0]+1\n"
"            body=inf.read(pktlen)\n"
"            pkt={'apid':apid}\n"
"            for k,v in pktdesc[apid]['fields'].items():\n"
"                typ =lookupType[pktdesc[apid]['fields'][k]['type']]\n"
"                pos =pktdesc[apid]['fields'][k]['pos' ]\n"
"                size=lookupSize[pktdesc[apid]['fields'][k]['type']]\n"
"                if typ=='s':\n"
"                    pkt[k]=body[pos-6:].decode('utf-8','replace')\n"
"                elif typ=='c':\n"
"                    pkt[k]=body[pos-6:]\n"
"                else:\n"
"                    pkt[k]=struct.unpack(typ,body[pos-6:pos-6+size])[0]\n"
"            if pktdesc[apid]['handler'] is not None:\n"
"                pktdesc[apid]['handler'](pkt)\n"
"EOF\n"
"exit\n"
)) return false;
  if(!finish(0x323)) return false;
  if(!writeDoc(0x323,0,0,"Script")) return false;
  if(!writeDoc(0x323,6,t_string,"script")) return false;
  return true;
}
                


