#include <Circular.h>
#include <Packet.h>

/*
  SD card basic file example
 
 This example shows how to create and destroy an SD card file 	
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 
 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 	 
 */
#include <SD.h>

File myFile;
// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 10;    

char cbuf[1024];
Circular buf(1024,cbuf);
CCSDS packet(buf,0x100);
uint16_t seq[32];

uint32_t microHours() {
  const uint32_t bound=3600UL*1000UL*1000UL;
  uint32_t thisMicro=micros();
  static uint32_t lastMicro=0;
  static uint32_t lastMicroHours=0;
  if(lastMicro==0) {
    lastMicro=thisMicro;
    lastMicroHours=lastMicro%bound;
  } else {
    uint32_t diff;
    if(thisMicro>lastMicro) {
      diff=thisMicro-lastMicro;
    } else {
      //pretend the counter runs 0 to 99. Last was 97, this is 4
      //Steps are 4+(100-97) but can't represent 100 so do 4+1+(99-97)
      //pretend the counter runs from 0 to N. Last was N-2, this is M
      //Steps are M+1+(N-last)
      diff=thisMicro+1+(0xFFFFFFFFU-lastMicro);  
    }
    lastMicro=thisMicro;
    lastMicroHours=(lastMicroHours+diff)%bound;
  }
  return lastMicroHours;
}

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(10, OUTPUT);

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card is inserted?");
    Serial.println("* Is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
   Serial.println("Wiring is correct and a card is present."); 
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint64_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();
  
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
//  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  uint32_t vsize=volumesize;
  Serial.println(vsize);
  Serial.print("Volume size (Mbytes): ");
  vsize /= 1024;
  Serial.println(vsize);

  
  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);
  
  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);

}

void loop() {
  root.ls(LS_R | LS_DATE | LS_SIZE);
  delay(1000);
}



