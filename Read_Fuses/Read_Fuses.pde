//#include <Flash.h>
#include <avr/boot.h>
#include <EEPROM.h>
#include "cpuname.h"

/*
 * SIGRD is a "must be zero" bit on most Arduino CPUs; can we read the sig or not?
 */
#if (!defined(SIGRD))
#define SIGRD 5
#endif

unsigned char fuse_bits_low;
byte fuse_bits_extended;
byte fuse_bits_high;
byte lock_bits;
byte sig1, sig2, sig3;
byte oscal;

void setup() {
  unsigned char xxx = fuse_bits_low>>4;
  Serial.begin(9600);
  Serial.print(xxx);
}

void space() {
  Serial.print(' ');
}

void print_serno(void) {
  int i;
  int unoSerial[6];
  int startAddr=1018;
  unsigned long serno = 0;

  for (i = 0; i < 6; i++) {
    unoSerial[i] = EEPROM.read(startAddr + i);
  }
  if (unoSerial[0] == 'U' && unoSerial[1] == 'N' && unoSerial[2] == 'O') {

    Serial.print("Your Serial Number is: UNO");

    for (i = 3; i < 6; i = i + 1) {
	serno = serno*256 + unoSerial[i];
	Serial.print(" ");
	Serial.print(unoSerial[i], HEX);
    }
    Serial.print(" (");
   Serial.print(serno,DEC);
   Serial.print(")");
  }
  else {
    Serial.println("No Serial Number");
  }
  Serial.println();
}

void print_binary(byte b)
{
  for (byte i=0x80; i>0; i>>=1) {
    if (b&i) {
	Serial.print('1');
    }
    else {
	Serial.print('0');
    }
  }
}

/*
 * Note that most fuses are active-low, and the the avr include files
 * define them as inverted bitmasks...
 */

void print_fuse_low(void)
{
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__) || \
  defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) ||  \
    defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__)
    /*
 * Fuse Low is same on 48/88/168/328
     */
    Serial.print("Fuse Low =   ");
  print_binary(fuse_bits_low);
  Serial.print(" (");
  Serial.print(fuse_bits_low, HEX);
  Serial.print(")\n");
  Serial.print("	     ||||++++______");
  switch (fuse_bits_low & 0xF) {
  case 0:
    Serial.print("Reserved");
    break;
  case 1:
    Serial.print("External Clock");
    break;
  case 2:
    Serial.print("Calibrated 8MHz Internal Clock");
    break;
  case 3:
    Serial.print("Internal 128kHz Clock");
    break;
  case 4:
    Serial.print("LF Crystal, 1K CK Startup");
    break;
  case 5:
    Serial.print("LF Crystal 32K CK Startup");
    break;
  case 6:
    Serial.print("Full Swing Crystal ");
    break;
  case 7:
    Serial.print("Full Swing Crystal");
    break;
  case 8:
  case 9:
    Serial.print("Low Power Crystal 0.4 - 0.8MHz");
    break;
  case 10:
  case 11:
    Serial.print("Low Power Crystal 0.9 - 3.0MHz");
    break;
  case 12:
  case 13:
    Serial.print("Low Power Crystal 3 - 8MHz");
    break;
  case 14:
  case 15:
    Serial.print("Low Power Crystal 8 - 16MHz");    
    break;
  }

  Serial.println();
  Serial.print("	     ||++__________");
  Serial.print("Start Up Time=");
  Serial.print((fuse_bits_low >> 4) & 3, BIN);

  Serial.println();
  Serial.print("	     |+____________");
  Serial.print("Clock Output ");
  if (fuse_bits_low & (~FUSE_CKOUT))
    Serial.print("Disabled");
  else
    Serial.print("Enabled");

  Serial.println();
  Serial.print("	     +_____________");
  if (fuse_bits_low & (~FUSE_CKDIV8)) {
    Serial.print("(no divide)");
  }
  else {
    Serial.print("Divide Clock by 8");
  }


#elif defined(__AVR_ATmega8__)
#endif
  Serial.println();
}
 
void print_fuse_high()
{
  Serial.print("\nFuse High = ");
  print_binary(fuse_bits_high);
  Serial.print(" (");
  Serial.print(fuse_bits_high, HEX);
  Serial.print(")\n");

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
  Serial.print("		|||||||+______");
  if (fuse_bits_high & bit(FUSE_BOOTRST)) {
    Serial.print("Reset to Start of memory\n");
  }
  else {
    Serial.print("Reset to Bootstrap\n");
  }
  Serial.print("		|||||++_______");
  switch ((byte)(fuse_bits_high & ((~FUSE_BOOTSZ1)+(~FUSE_BOOTSZ0)))) {
    case (byte)((~FUSE_BOOTSZ1)+(~FUSE_BOOTSZ0)):
    Serial.print("256 words (512 bytes)\n");
    break;
    case (byte)((~FUSE_BOOTSZ1)):
    Serial.print("512 words (1024 bytes)\n");
    break;
    case (byte)((~FUSE_BOOTSZ0)):
    Serial.print("1024 words (2048 bytes)\n");
    break;
  case 0:
    Serial.print("2048 words (4096 bytes)\n");
    break;
  default:
    Serial.println(fuse_bits_high & ((~FUSE_BOOTSZ1)+(~FUSE_BOOTSZ0)), BIN);
  }
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || \
  defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__)
    Serial.print("		|||||+++______");
  switch ((byte)(fuse_bits_high & 7)) {
  case 7:
    Serial.print("Brownout Disabled\n");
    break;
  case 6:
    Serial.print("Brownout at 1.8V\n");
    break;
  case 5:
    Serial.print("Brownout at 2.7V\n");
    break;
  case 4:
    Serial.print("Brownout at 4.3V\n");
    break;
  default:
    Serial.print("Brownout Reserved value");
    Serial.println(fuse_bits_high& 7, BIN);
    break;
  }

#elif defined(__AVR_ATmega8__)
#endif
  Serial.print("		||||+_________");
  if (fuse_bits_high & ~(FUSE_EESAVE)) {
    Serial.print("EEPROM Erased on chip erase\n");
  }
  else {
    Serial.print("EEPROM Preserved on chip erase\n");
  }
  Serial.print("		|||+__________");
  if (fuse_bits_high & ~(FUSE_WDTON)) {
    Serial.print("Watchdog programmable\n");
  }
  else {
    Serial.print("Watchdog always on\n");
  }
  Serial.print("		||+___________");
  if (fuse_bits_high & ~(FUSE_SPIEN)) {
    Serial.print("ISP programming disabled\n");
  }
  else {
    Serial.print("ISP programming enabled\n");
  }
  Serial.print("		|+____________");
  if (fuse_bits_high & ~(FUSE_DWEN)) {
    Serial.print("DebugWire off\n");
  }
  else {
    Serial.print("DebugWire enabled\n");
  }
  Serial.print("		+_____________");
  if (fuse_bits_high & ~(FUSE_RSTDISBL)) {
    Serial.print("RST enabled\n");
  }
  else {
    Serial.print("RST disabled\n");
  }
}


void print_lock_bits()
{
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__) || \
  defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__)

    Serial.print("\nLock Bits = ");
  print_binary(lock_bits);
  Serial.print(" (");
  Serial.print(lock_bits, HEX);
  Serial.print(")\n");
  Serial.print("		||||||++______");
  switch ((byte)(lock_bits & 3)) {
  case 3:
    Serial.print("Read/Write to everywhere\n");
    break;
  case 2:
    Serial.print("Programming of Flash/EEPROM disabled\n");
    break;
  case 0:
    Serial.print("No Read/Write of Flash/EEPROM\n");
    break;
  default:
    Serial.println();
  }
  Serial.print("		||||++________");
  switch ((byte)(lock_bits & 0b1100)) {  //BLB0x
  case 0b1100:
    Serial.print("R/W Application\n");
    break;
  case 0b1000:
    Serial.print("No Write to App\n");
    break;
  case 0b0000:
    Serial.print("No Write to App, no read from Boot\n");
    break;
  case 0b0100:
    Serial.print("No Write to App, no read from Boot, no Ints to App\n");
    break;
  }

  Serial.print("		||++__________");
  switch ((byte)(lock_bits & 0b110000)) {  //BLB0x
  case 0b110000:
    Serial.print("R/W Boot Section\n");
    break;
  case 0b100000:
    Serial.print("No Write to Boot Section\n");
    break;
  case 0b000000:
    Serial.print("No Write to Boot, no read from App\n");
    break;
  case 0b010000:
    Serial.print("No Write to Boot, no read from App, no Ints to Boot\n");
    break;
  }
#elif defined(__AVR_ATmega8__)
#endif
}


void print_fuse_extended()
{
  Serial.print("\nFuse Extended = ");
  print_binary(fuse_bits_extended);
  Serial.print(" (");
  Serial.print(fuse_bits_extended, HEX);
  Serial.print(")\n");
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
  Serial.print("		    |||||+++______");
  switch ((byte)(fuse_bits_extended & 7)) {
  case 7:
    Serial.print("Brownout Disabled\n");
    break;
  case 6:
    Serial.print("Brownout at 1.8V\n");
    break;
  case 5:
    Serial.print("Brownout at 2.7V\n");
    break;
  case 4:
    Serial.print("Brownout at 4.3V\n");
    break;
  default:
    Serial.print("Brownout Reserved value");
    Serial.println(fuse_bits_extended & 7, BIN);
    break;
  }
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__)
  Serial.print("		    |||||||+______");
  if (fuse_bits_extended & bit(FUSE_BOOTRST)) {
    Serial.print("Reset to Start of memory\n");
  }
  else {
    Serial.print("Reset to Bootstrap\n");
  }
  Serial.print("		    |||||++_______");
  switch ((byte)(fuse_bits_extended & ((~FUSE_BOOTSZ1)+(~FUSE_BOOTSZ0)))) {
    case (byte)((~FUSE_BOOTSZ1)+(~FUSE_BOOTSZ0)):
    Serial.print("128 words (256 bytes)\n");
    break;
    case (byte)((~FUSE_BOOTSZ1)):
    Serial.print("256 words (512 bytes)\n");
    break;
    case (byte)((~FUSE_BOOTSZ0)):
    Serial.print("512 words (1024 bytes)\n");
    break;
  case 0:
    Serial.print("1024 words (2048 bytes)\n");
    break;
  default:
    Serial.println(fuse_bits_extended & ((~FUSE_BOOTSZ1)+(~FUSE_BOOTSZ0)), BIN);
  }
#elif defined(__AVR_ATmega8__)
#endif
}


void print_signature()
{
  Serial.print("\nSignature:	   ");
  Serial.print(sig1, HEX);
  space();
  Serial.print(sig2, HEX);
  space();
  Serial.print(sig3, HEX);
  if (sig1 == 0x1E) { /* Atmel ? */
    switch (sig2) {
    case 0x92:  /* 4K flash */
	if (sig3 == 0x0A)
	  Serial.print(" (ATmega48P)");
	else if (sig3 == 0x05)
	  Serial.print(" (ATmega48A)");
	else if (sig3 == 0x09)
	  Serial.print(" (ATmega48)");
	break;
    case 0x93:  /* 8K flash */
	if (sig3 == 0x0F)
	  Serial.print(" (ATmega88P)");
	else if (sig3 == 0x0A)
	  Serial.print(" (ATmega88A)");
	else if (sig3 == 0x11)
	  Serial.print(" (ATmega88)");
	else if (sig3 == 0x08)
	  Serial.print(" (ATmega8)");
	break;
    case 0x94:  /* 16K flash */
	if (sig3 == 0x0B)
	  Serial.print(" (ATmega168P)");
	else if (sig3 == 0x06)
	  Serial.print(" (ATmega168A)");
	break;
    case 0x95:  /* 32K flash */
	if (sig3 == 0x0F)
	  Serial.print(" (ATmega328P)");
	else if (sig3 == 0x14)
	  Serial.print(" (ATmega328)");
	break;
    }
  }
  else {
#if defined (__AVR_ATmega168__) || defined(__AVR_ATmega8__)
    Serial.print(" (Fuses not readable on non-P AVR)");
#else
    Serial.print("????");
#endif
  }
}

void loop()
{
  delay(2000);
  Serial.print("\nCompiled for " __CPUNAME "\n");
  print_serno();
  cli();
  fuse_bits_low = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
  fuse_bits_extended = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
  fuse_bits_high = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
  lock_bits = boot_lock_fuse_bits_get(GET_LOCK_BITS);
  sig1 = boot_signature_byte_get(0);
  sig2 = boot_signature_byte_get(2);
  sig3 = boot_signature_byte_get(4);
  oscal = boot_signature_byte_get(1);
  sei();

  Serial.print("\nFuse bits (L/E/H): ");
  Serial.print(fuse_bits_low, HEX);
  space();
  Serial.print(fuse_bits_extended, HEX);
  space();
  Serial.print(fuse_bits_high, HEX);
  Serial.print("\nLock bits:	   ");
  Serial.print(lock_bits, HEX);
  print_signature();
  Serial.print("\nOscal:		 ");
  Serial.println(oscal, HEX);
  Serial.println();

  print_fuse_low();
  print_fuse_high();
  print_fuse_extended();
  print_lock_bits();

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || \
  defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__)
#elif defined(__AVR_ATmega8__)
#endif
    while (1) {
	if (Serial.read() > 0)
	  break;
    }
}


 
 

