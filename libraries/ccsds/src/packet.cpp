#include "packet.h"
#include "Time.h"
#include "gpio.h"
#include "Serial.h"

bool CCSDS::writeDoc(uint8_t type, const char* desc) {
  Debug.print("CCSDS::writeDoc(type=0x");
  Debug.print(type,16,2);
  Debug.print(",desc=\"");
  Debug.print(desc);
  Debug.println("\")");
  Debug.print("docd[doc_apid=0x");
  Debug.print(doc_apid,16,3);
  Debug.print("]=");
  Debug.println(docd[doc_apid],16,1);
  if(!desc) {
    Debug.println("Packet has no documentation");
    return true;
  }
  if(docd[doc_apid]) {
    Debug.println("apid is already documented");
    return true;
  }
  if(!start(apid_doc)) {
    Debug.print("Something went wrong starting doc packet");
    return false;
  }
  //uint16_t packet APID being described
  Debug.print("Field 1 - apid being documented: 0x");
  Debug.println(doc_apid,16,3);
  if(!fillu16(doc_apid)) {
    Debug.print("Something went wrong printing apid");
    return false;
  }
  //uint16_t position in the packet of the field being described, zero if the whole packet is being named
  Debug.print("Field 2 - position: 0x");
  Debug.println(type==0?0:stashlen,16,3);
  if(!fillu16(type==0?0:stashlen))  {
    Debug.print("Something went wrong printing position");
    return false;
  }
  //uint8_t type of the field
  Debug.print("Field 3 - type: 0x");
  Debug.println(type,16,2);
  if(!fill(type)) {
    Debug.print("Something went wrong printing type");
    return false;
  }
  //string field description
  Debug.print("Field 4 - description: \"");
  Debug.print(desc);
  Debug.println("\"");
  if(!fill(desc)) {
    Debug.print("Something went wrong printing description");
    return false;
  }
  return finish(apid_doc);
}

bool CCSDS::start(uint16_t apid, uint32_t TC) {
  Debug.print("CCSDS::start(apid=0x");
  Debug.print(apid,16,3);
  Debug.print(",tc=0x");
  Debug.print((unsigned int)TC,16,8);
  Debug.println(")");
  if(apid==apid_doc) {
    stash_apid=lock_apid;
    lock_apid=apid;
  } else if(lock_apid>0) {
    Debug.print("Tried to start a packet when one already in process: old: 0x");
    Debug.print(lock_apid,HEX);Debug.print(" new: 0x");Debug.print(apid,HEX);
    blinklock(apid);
  } else if(!docd[apid]) {
    //Prepare the stash
    stashlen=0;
  }
  lock_apid=apid;
  const int Ver=0;  //0 for standard CCSDS telecommand according to CCSDS 102.0-B-5 11/2000
  const int Type=0; //0 for telemetry, 1 for command
  int Sec=(TC!=0xFFFFFFFFU)?1:0;  //Presence of secondary header
  const int Grp=0x03;  //Grouping flags - 3->not in a group of packets
                 //data       len        lowbit
  uint16_t word=(((apid & ((1<<11)-1)) <<  0) | 
                 ((Sec  & ((1<< 1)-1)) << 11) | 
                 ((Type & ((1<< 1)-1)) << 12) | 
                 ((Ver  & ((1<< 3)-1)) << 13));
  Debug.print("Sending first word: ver=0x");
  Debug.print(Ver,16,1);
  Debug.print(", type=0x");
  Debug.print(Type,16,1);
  Debug.print(", sec=0x");
  Debug.print(Sec,16,1);
  Debug.print(", apid=0x");
  Debug.print(apid,16,3);
  Debug.print(", so word is 0x");
  Debug.println(word,16,4);
  if(!fillu16(word)) return false;
  unsigned short seq_=0;
  if(seq) seq_=seq[apid];
        //data       len        lowbit
  word=(((seq_ & ((1<<14)-1)) <<  0) | 
        ((Grp  & ((1<< 2)-1)) << 14));
  Debug.print("Sending second word: grp=0x");
  Debug.print(Grp,16,1);
  Debug.print(", seq=0x");
  Debug.print(seq_,16,4);
  Debug.print(", so word is 0x");
  Debug.println(word,16,4);
  if(!fillu16(word)) return false;
  word=0xDEAD;
  Debug.print("Reserving space for length: 0x");
  Debug.println(word,16,4);
  if(!fillu16(word)) return false; //Reserve space in the packet for length
  if(Sec) {
    //Secondary header: count of microseconds since beginning of minute
    Debug.print("Sending secondary header: TC=0x");
    Debug.println((unsigned int)TC,16,8);
    if(!fillu32(TC)) return false;
  }
  if(seq) seq[apid]=(seq[apid]+1)& 0x3FFF;
  return true;
}

bool CCSDS::finish(uint16_t tag) {
  Debug.print("CCSDS::finish(tag=0x");
  Debug.print(tag,16,3);
  Debug.println(")");
  if(!docd[tag] && tag!=apid_doc) {
    Debug.println("Get packet from stash");
    //The packet was being documented (and we are not working on a doc packet
    //this moment), so copy out the stash.
    Debug.print("Patch stashed packet length to 0x");
    stashlen-=7;
    Debug.print(stashlen,16,4);
    stashbuf[4]=(stashlen >> 8) & 0xFF;
    stashbuf[5]=(stashlen >> 0) & 0xFF;
    stashlen+=7;
    //If the real buffer was already full, then throw the new packet away. Just
    //don't write the packet to the buffer. Also don't mark the packet as documented,
    //so we get another crack at it later.
    if(!buf.isFull()) {
      Debug.print("Copy packet from stash buffer to real buffer");
      buf.fill(stashbuf,stashlen);
      buf.mark();
      buf.drain();
    } else {
      Debug.print("Can't copy packet, buffer is full");
    }
    stashlen=0;
    lock_apid=0;
    Debug.print("Marking docd[tag=0x");
    Debug.print(tag,16,3);
    Debug.print("]=");
    Debug.println(!buf.isFull());
    docd[tag]=!buf.isFull();
    return !buf.isFull();
  }
  //If we get here, either we are not documenting a packet, or we are writing
  //a doc packet, so write to the real circular buffer.
  //If the buffer is already full, we know not to do this
  if(buf.isFull()) {
    lock_apid=0; //otherwise the lock will never be released
    return false;
  }
  int len=buf.unreadylen()-7;
  if(len<0) {
    Debug.print("Bad packet finish: 0x");Debug.println(tag,HEX);
    blinklock(tag);
  }
  buf.pokeMid(4,(len >> 8) & 0xFF);
  buf.pokeMid(5,(len >> 0) & 0xFF);
  buf.mark();
  buf.drain();
  if(tag==apid_doc) {
    lock_apid=stash_apid;
  } else {
    lock_apid=0;
  }
  return true;
}

bool CCSDS::fill(char c) {
  if((c<' ') || (c>'~')) {
    Debug.print("CCSDS::fill(c=0x");
    Debug.print(c,16,2);
    Debug.println(")");
  } else {
    Debug.print("CCSDS::fill(c='");
    Debug.print(c);
    Debug.println("')");
  }
  //Write to the main buffer if any of these are true:
  //*The current packet has already been documented
  //*We are writing a doc packet
  //Otherwise write to the stash buffer
  Debug.print("Deciding which buffer to print to: lock_apid=0x");
  Debug.print(lock_apid,16,3);
  Debug.print(", apid_doc=0x");
  Debug.print(apid_doc,16,3);
  Debug.print(", docd[0x");
  Debug.print(lock_apid,16,3);
  Debug.print("]=");
  Debug.println(docd[lock_apid],16,1);
  if((lock_apid==apid_doc)||docd[lock_apid]) {
    Debug.println("Printing directly to main circular buffer");
    if(!buf.fill(c)) {
      Debug.println("Something went wrong in buf.fill(c)");
      return false;
    }
  } else {
    Debug.print("Printing to stash buffer at position ");
    Debug.println(stashlen);
    stashbuf[stashlen]=c;
    stashlen++;
  }
  return true;
}

//Fill in Big-endian order as specified by CCSDS 102.0-B-5, 1.6a
bool CCSDS::fillu16(uint16_t in) {
  Debug.print("CCSDS::fillu16(in=");
  Debug.print(in,16,4);
  Debug.println(")");
  if(!fill((char)((in >> 8) & 0xFF))) return false;
  if(!fill((char)((in >> 0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fillu32(uint32_t in) {
  Debug.print("CCSDS::fillu32(in=");
  Debug.print((unsigned int)in,16,8);
  Debug.println(")");
  if(!fill((char)((in >> 24) & 0xFF))) return false;
  if(!fill((char)((in >> 16) & 0xFF))) return false;
  if(!fill((char)((in >>  8) & 0xFF))) return false;
  if(!fill((char)((in >>  0) & 0xFF))) return false;
  return true;
}

bool CCSDS::fillu64(uint64_t in) {
  Debug.print("CCSDS::fillu64(in=");
  Debug.print((unsigned int)in);
  Debug.println(")");
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

bool CCSDS::fillfp(fp f) {
  char* fc=(char*)(&f);
  return fill(fc,sizeof(f));
}

bool CCSDS::metaDoc() {
  if(!metaDoc("This file contains CCSDS packets as described in CCSDS 133.0-B-1 with updates."))return false;
  if(!metaDoc("https://public.ccsds.org/Pubs/133x0b1c2.pdf"))return false;
  if(!metaDoc("There are no padding bytes or sync markers between packets."))return false;
  if(!metaDoc("All packets are an integer number of whole bytes."))return false;
  if(!metaDoc("Each packet starts with a six-byte header which holds the packet "
		   "type (APID), sequence number and length."))return false;
  if(!metaDoc("All numbers in a CCSDS packet are stored Big-Endian."))return false;
  if(!metaDoc("Packets start with a 16-bit unsigned integer, upper 5 bits are "
		   "version and type, lower 11 bits are the APID"))return false;
  if(!metaDoc("Next is a 16-bit number with the upper 2 bits set (flags indicating unfragmented packet) "
		   "and the lower 14 bits are a sequence number which increments for each APID and "
		   "wraps in 14 bits."))return false;
  if(!metaDoc("Next 16-bit number is length of packet minus 7 since every packet "
		   "has a 6-byte header and must have at least 1 byte of payload."))return false;
  /*
  if(!metaDoc0("Packets of apid ")) return false;
  print(Apids::parse,DEC);
  if(!metaDoc1(" encapsulates a python 3 script which can decode this stream into "
                   "separate CSV streams for each apid. The apid and length are correct for this "
                   "type of packet, but other flags may not be."))return false;
  */
  if(!metaDocDec("Packets of apid ",apid_metadoc," contain this English meta-documentation. "))return false;
  if(!metaDocDec("Packets of apid ",apid_doc," describe the detailed format of the other packets."))return false;
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
  if(!metaDocHex("0x",0,2,": This name describes the whole packet, not any one field"))return false;
  if(!metaDocHex("0x",t_u8    ,2,": t_u8 (8-bit unsigned integer)"))return false;
  if(!metaDocHex("0x",t_i16   ,2,": t_i16 (16-bit signed integer)"))return false;
  if(!metaDocHex("0x",t_i32   ,2,": t_i32 (32-bit signed integer)"))return false;
  if(!metaDocHex("0x",t_u16   ,2,": t_u16 (16-bit unsigned integer)"))return false;
  if(!metaDocHex("0x",t_u32   ,2,": t_u32 (32-bit unsigned integer)"))return false;
  if(!metaDocHex("0x",t_float ,2,": t_float (32-bit IEEE-754 floating point)"))return false;
  if(!metaDocHex("0x",t_double,2,": t_double (64-bit IEEE-754 floating point)"))return false;
  if(!metaDocHex("0x",t_string,2,": t_string (UTF-8 text)"))return false;
  if(!metaDocHex("0x",t_binary,2,": t_binary (unformatted data dump)"))return false;
  if(!metaDoc("The fourth field is a UTF-8 text string with the name of the field."))return false;
  if(!metaDoc("For all strings and binary data, no length information is included."))return false;
  if(!metaDoc("If the string or binary is the only such field in the packet, its "
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
  return true;
}
