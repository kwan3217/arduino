#ifndef packet_h
#define packet_h
#include <inttypes.h>
#include "Circular.h"
#include "float.h"

/** Abstract class providing an interface for creating packets. A packet
    contains a number of data points which are closely related in
    some way. For instance, your process may create a stream of vectors.
    It makes sense to encode each of the components of the vector in some
    binary format, then group all the components into a packet. When used this
    way, a packet is in a sense *atomic*, which means that you can't interpret
    part of the packet if some of it is missing. Packets are grouped together
    into a packet stream, which may then be saved to storage or transmitted
    across a communications link.

    This library is intended to handle any style of binary packet within reason,
    but is heavily biased towards CCSDS telemetry packet creation. The binary
    formats I have seen include a header, which at minimum include a packet
    type and length. Concrete implementations of this class keep track of the
    length for you, so that the calling code doesn't have to count bytes.
    Implementations will also construct any checksum and/or footer necessary
    for the packet.

    Being biased towards CCSDS, we call the packet type the APID (application
    process ID) after the corresponding concept in CCSDS. All packets with the
    same APID will have the same structure. Information on the packet
    structure is not usually a part of the packet, so the packet
    interpretation process (data system process) will have to agree with the
    packet creation process on the format of each packet, using a mechanism
    not directly in the scope of this class.
*/
class Packet {
protected:
  Circular* buf; //<<< Buffer to write packets to
public:
  Packet() {};

  /** Start a packet with a given apid in a given buffer
    @param Lbuf circular buffer to put packet into
    @param pkt_id Packet identifier
    @param optional optional value, interpretation of which is up to concrete implementations. Default value is to be
           interpreted as if the argument wasn't passed.
  */
  virtual bool start(Circular &Lbuf, uint16_t pkt_id, uint32_t optional=0xFFFFFFFF)=0;
  /** Finish a packet with a given packet identifier
    @param apid Packet identifier, must match packet identifier passed to last invocation of Packet::start()
  */
  virtual bool finish(uint16_t tag)=0;
  /** Add one byte to a packet
    @param in byte to add to packet
  */
  bool fill(char in) {return buf->fill(in);};
  /** Add a 16-bit (two-byte) value to a packet
    @param in value to add to packet
  */
  virtual bool fill16(uint16_t in)=0;
  /** Add a 32-bit (four-byte) value to a packet
    @param in value to add to packet
  */
  virtual bool fill32(uint32_t in)=0;
  /** Add a 64-bit (eight-byte) value to a packet
    @param in value to add to packet
  */
  virtual bool fill64(uint64_t in)=0;
  /** Add a floating-point value to a packet
    @param f value to add to packet
  */
  virtual bool fillfp(fp f)=0;
  /** Add a null-terminated string to a packet
    @param in value to add to packet
  */
  virtual bool fill(const char* in);
  /** Add a string of given length to a packet
    @param in value to add to packet
    @param length number of bytes to add
  */
  virtual bool fill(const char* in, uint32_t length);
  /** Remove a partially completed packet from the buffer. After this, another invocation of Packet::start() must be called when another packet is to be created. This packet may have any apid
  */
  virtual void forget();
};

#endif

