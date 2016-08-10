#include "WireSlaveReg.h"

WireSlaveReg SlaveReg;

/**
Call this function to set up the TWI slave to its initial state. As opposed
to AVR311, we are ready to rock immediately, so we go into active state and stay
there.

Note that a slave initialized like this is hard-coded to not answer a 
general call.

@param[in] Laddr 7-bit address for this part
@param[in] Lreg pointer to array of characters used as register space

Code "borrowed" from TWI_Slave.c, TWI_Slave_Initialise and TWI_Start_Transceiver_With_Data
*/
void WireSlaveReg::begin(int Laddr, char* Lreg) {
  addr=Laddr;
  reg=Lreg;
  TWAR = addr<<TWI_ADR_BITS;                        // Set own TWI slave address. Accept TWI General Calls.
  TWI_statusReg.all = 0;      
  TWI_state         = TWI_NO_STATE ;
  TWCR = (1<<TWEN)|                             // TWI Interface enabled.
         (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interrupt and clear the flag.
         (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|       // Prepare to ACK next time the Slave is addressed.
         (0<<TWWC);                             //
}

/**
This function is the Interrupt Service Routine (ISR), and called when the TWI interrupt is triggered;
that is whenever a TWI event has occurred. This function should not be called directly from the main
application.

Code "borrowed" from TWI_Slave.c, TWI_Vect(). Modified to work in C++, to work
with my class/object, and to work with the register file as its write buffer.
*/
void WireSlaveReg::TWI_vect() {
  static unsigned char TWI_bufPtr;
  
  switch (TWSR)
  {
    case TWI_STX_ADR_ACK:            // Own SLA+R has been received; ACK has been returned

    case TWI_STX_DATA_ACK:           // Data byte in TWDR has been transmitted; ACK has been received
      TWDR = SlaveReg.reg[SlaveReg.addr];
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interrupt and clear the flag to send byte
             (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // 
             (0<<TWWC);                                 //
      // Data is committed to send, tell the client and update the pointer
      SlaveReg.client_onRequest(SlaveReg.addr);
      break;
    case TWI_STX_DATA_NACK:          // Data byte in TWDR has been transmitted; NACK has been received. 
                                     // I.e. this could be the end of the transmission.
                                                        
      TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
             (1<<TWIE)|(1<<TWINT)|                      // Keep interrupt enabled and clear the flag
             (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Answer on next address match
             (0<<TWWC);                                 //
      break;     
    case TWI_SRX_GEN_ACK:            // General call address has been received; ACK has been returned
      SlaveReg.TWI_statusReg.genAddressCall = true;
    case TWI_SRX_ADR_ACK:            // Own SLA+W has been received ACK has been returned
                                                       // don't need to clear TWI_S_statusRegister.generalAddressCall due to that it is the default state.
      SlaveReg.TWI_statusReg.RxDataInBuf = true;      
      SlaveReg.regAddrIncoming=true;
                                                        // Reset the TWI Interrupt to wait for a new event.
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interrupt and clear the flag to send byte
             (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Expect ACK on this transmission
             (0<<TWWC);  
      break;
    case TWI_SRX_ADR_DATA_ACK:       // Previously addressed with own SLA+W; data has been received; ACK has been returned
    case TWI_SRX_GEN_DATA_ACK:       // Previously addressed with general call; data has been received; ACK has been returned
      if(SlaveReg.regAddrIncoming) {
        SlaveReg.addr=TWDR;
      } else {
        SlaveReg.reg[SlaveReg.addr]=TWDR;
        SlaveReg.client_onReceive(SlaveReg.addr);
      }
      SlaveReg.TWI_statusReg.lastTransOK = true;                 // Set flag transmission successfully.       
                                                        // Reset the TWI Interrupt to wait for a new event.
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interrupt and clear the flag to send byte
             (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
             (0<<TWWC);                                 // 
      break;
    case TWI_SRX_STOP_RESTART:       // A STOP condition or repeated START condition has been received while still addressed as Slave    
                                                        // Enter not addressed mode and listen to address match
      TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
             (1<<TWIE)|(1<<TWINT)|                      // Enable interrupt and clear the flag
             (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Wait for new address match
             (0<<TWWC);                                 //
      
      break;           
    case TWI_SRX_ADR_DATA_NACK:      // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
    case TWI_SRX_GEN_DATA_NACK:      // Previously addressed with general call; data has been received; NOT ACK has been returned
    case TWI_STX_DATA_ACK_LAST_BYTE: // Last data byte in TWDR has been transmitted (TWEA = "0"); ACK has been received
    case TWI_BUS_ERROR:         // Bus error due to an illegal START or STOP condition
      SlaveReg.TWI_state = TWSR;                 //Store TWI State as error message, operation also clears noErrors bit
      TWCR =   (1<<TWSTO)|(1<<TWINT);   //Recover from TWI_BUS_ERROR, this will release the SDA and SCL pins thus enabling other devices to use the bus
      break;
    default:     
      SlaveReg.TWI_state = TWSR;                                 // Store TWI State as error message, operation also clears the Success bit.      
      TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
             (1<<TWIE)|(1<<TWINT)|                      // Keep interrupt enabled and clear the flag
             (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Acknowledge on any new requests.
             (0<<TWWC);                                 //
  }
}


