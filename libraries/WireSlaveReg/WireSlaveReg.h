#ifndef PCHARLIE_H
#define PCHARLIE_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

/**  
 *  This is based on Atmel Application Note AVR311. Their copyright
 *  and license is included below. It looks like a pretty standard/
 *  unobjectionable BSD-style license with the added proviso that
 *  it only be used on Atmel parts. I will only do so, but it is
 *  up to subsequent users of this library to honor that portion.
 *  
 *  Restructured code is copyright (C) 2016 Chris Jeppesen, and
 *  is released under the same license as the Atmel code.
 *  
 * Copyright (C) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel micro controller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *  
 */

/** Status byte holding flags. */
union TWI_statusReg_t {
    unsigned char all;
    struct {
        unsigned char lastTransOK:1;      
        unsigned char RxDataInBuf:1;
        unsigned char genAddressCall:1;                        // TRUE = General call, FALSE = TWI Address;
        unsigned char unusedBits:5;
    };
};

/** Implements a register protocol on top of the I2C wire protocol.
 *  The Wire library doesn't seem to work for me in the slave mode,
 *  namely it looks like the slave has to know how many bytes
 *  to send back to the master. In the register burst mode, the 
 *  slave just keeps sending back data until the master tells
 *  it to stop, so this isn't possible.
 *  
 *  The protocol is identical to that used on the MPU9250 and many
 *  other sensors. The part defines a register space, with a series
 *  of 8-bit registers numbered from zero. To access a register, 
 *  the master sends a register address to the part. Then to read 
 *  the register, the master tells the part to send back one byte. 
 *  To write the register, the master sends a byte to the part.
 *  The device maintains a current address pointer, and updates
 *  it in a manner convenient to "burst-read" the part. Often
 *  several consecutive registers in the register space contain
 *  several related measurements, say the 3 readouts of a 3-axis
 *  accelerometer. If each axis is 16-bits starting at address 0x12,
 *  then the master can read all the readouts by writing 0x12, then
 *  reading 6 bytes. The part automatically advances its register
 *  address pointer each byte, so that it doesn't read 0x12 six
 *  times, but 0x12-0x17 consecutively.
 *  
 *  Depending on the part, sometimes the register pointer is
 *  advanced irregularly. For instance on this accelerometer,
 *  maybe after reading 0x17, the pointer automatically jumps
 *  back to 0x12, so that reading 6 more bytes gets the next 
 *  measurement. Other registers might make sense to not advance
 *  at all, such as a device which has a write-only register
 *  which it echoes to a serial port. You would want to keep
 *  writing that address over and over again.
 *  
 *  Certain registers, when written to, cause actions to happen.
 *  For instance, writing to the correct bit of a reset register
 *  might reset the part. Other registers spontaneously update
 *  themselves, like registers holding the values of a sensor
 *  reading.
 *  
 *  This object implements this protocol. The client code sets
 *  up an array of bytes to use as the register space. Commonly
 *  this will be on top of a structure, so that the client
 *  code can use its fields, but the master can read it a byte
 *  at a time. Whenever a register is written, the client code
 *  gets a callback which tells the client code which register
 *  was written. It is then up to the callback code to do something
 *  with the value in that register and update the pointer. The
 *  same thing is true for when a register is read. In principle
 *  this means that the client can take action based on a read,
 *  but in practice we will just use this callback to update the
 *  pointer.
 */
class WireSlaveReg {
private:
typedef void (*callback_f)(int& regAddr);
  int addr;  ///< Register pointer. Next read or write will hit here.
  char* reg; ///< Register file, client code will set this pointer
  bool regAddrIncoming; ///< True if Next byte written by master will be a register address
  /** Pointer to client function which receives notifications
   *  that a register has been written to from the Master.
   *  
   *  This function should return the new value of the register
   *  pointer. It also may take any actions necessary to implement
   *  a register-activated action. Make things quick, consider
   *  using a top/bottom half structure if the action takes more than
   *  a few SCL cycles.
   */
  callback_f client_onReceive;
  /** Pointer to client function which receives notifications
   *  that a register has been read by from the Master. This code
   *  is only called after the Master has read the register,
   *  so it is too late to affect the value that the Master receives.
   *  
   *  This function should return the new value of the register
   *  pointer. It also may take any actions necessary to implement
   *  a register-activated action. Make things quick, consider
   *  using a top/bottom half structure if the action takes more than
   *  a few SCL cycles.
   */
  callback_f client_onRequest;

/****************************************************************************
  Bit and byte definitions
****************************************************************************/
  static const int TWI_READ_BIT = 0;   ///< Bit position for R/W bit in "address byte".
  static const int TWI_ADR_BITS = 1; ///<  Bit position for LSB of the slave address bits in the init byte.
  static const int TWI_GEN_BIT = 0; ///<  Bit position for LSB of the general call bit in the init byte.

/****************************************************************************
  TWI State codes
****************************************************************************/
// General TWI Master status codes                      
  static const int TWI_START = 0x08; ///<  START has been transmitted  
  static const int TWI_REP_START = 0x10; ///<  Repeated START has been transmitted
  static const int TWI_ARB_LOST = 0x38; ///<  Arbitration lost

// TWI Master Transmitter status codes                      
  static const int TWI_MTX_ADR_ACK = 0x18; ///<  SLA+W has been transmitted and ACK received
  static const int TWI_MTX_ADR_NACK = 0x20; ///<  SLA+W has been transmitted and NACK received 
  static const int TWI_MTX_DATA_ACK = 0x28; ///<  Data byte has been transmitted and ACK received
  static const int TWI_MTX_DATA_NACK = 0x30; ///<  Data byte has been transmitted and NACK received 

// TWI Master Receiver status codes  
  static const int TWI_MRX_ADR_ACK = 0x40; ///<  SLA+R has been transmitted and ACK received
  static const int TWI_MRX_ADR_NACK = 0x48; ///<  SLA+R has been transmitted and NACK received
  static const int TWI_MRX_DATA_ACK = 0x50; ///<  Data byte has been received and ACK transmitted
  static const int TWI_MRX_DATA_NACK = 0x58; ///<  Data byte has been received and NACK transmitted

// TWI Slave Transmitter status codes
  static const int TWI_STX_ADR_ACK = 0xA8; ///<  Own SLA+R has been received; ACK has been returned
  static const int TWI_STX_ADR_ACK_M_ARB_LOST = 0xB0; ///<  Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
  static const int TWI_STX_DATA_ACK = 0xB8; ///<  Data byte in TWDR has been transmitted; ACK has been received
  static const int TWI_STX_DATA_NACK = 0xC0; ///<  Data byte in TWDR has been transmitted; NOT ACK has been received
  static const int TWI_STX_DATA_ACK_LAST_BYTE = 0xC8; ///<  Last data byte in TWDR has been transmitted (TWEA = 0); ACK has been received

// TWI Slave Receiver status codes
  static const int TWI_SRX_ADR_ACK = 0x60; ///<  Own SLA+W has been received ACK has been returned
  static const int TWI_SRX_ADR_ACK_M_ARB_LOST = 0x68; ///<  Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
  static const int TWI_SRX_GEN_ACK = 0x70; ///<  General call address has been received; ACK has been returned
  static const int TWI_SRX_GEN_ACK_M_ARB_LOST = 0x78; ///<  Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
  static const int TWI_SRX_ADR_DATA_ACK = 0x80; ///<  Previously addressed with own SLA+W; data has been received; ACK has been returned
  static const int TWI_SRX_ADR_DATA_NACK = 0x88; ///<  Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
  static const int TWI_SRX_GEN_DATA_ACK = 0x90; ///<  Previously addressed with general call; data has been received; ACK has been returned
  static const int TWI_SRX_GEN_DATA_NACK = 0x98; ///<  Previously addressed with general call; data has been received; NOT ACK has been returned
  static const int TWI_SRX_STOP_RESTART = 0xA0; ///<  A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
  static const int TWI_NO_STATE = 0xF8; ///<  No relevant state information available; TWINT = "0"
  static const int TWI_BUS_ERROR = 0x00; ///<  Bus error due to an illegal START or STOP condition

  unsigned char TWI_state    = TWI_NO_STATE;  // State byte. Default set to TWI_NO_STATE.
  union TWI_statusReg_t TWI_statusReg = {0};           // TWI_statusReg is defined in TWI_Slave.h

  static void  __attribute__((signal,__INTR_ATTRS)) TWI_vect(void);
public:
  void begin(int Laddr, char* Lreg);
  void onReceive(callback_f); 
  void onRequest(callback_f); 
};

extern WireSlaveReg SlaveReg;

#endif
