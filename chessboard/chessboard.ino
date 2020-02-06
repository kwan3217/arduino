// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      24

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define bright 2

int delayval = 20; // delay for half a second
int32_t color[6];
void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  color[0]=pixels.Color(bright,0,0);
  color[1]=pixels.Color(bright,bright/4,0);
  color[2]=pixels.Color(bright,bright*5/8,0);
  color[3]=pixels.Color(0,bright,0);
  color[4]=pixels.Color(0,0,bright);
  color[5]=pixels.Color(bright/2,0,bright);
  
}

void loop() {

  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    int c;
    if(i>=8 && i<16) {
      c=(i%2==1)?2:5;
    } else {
      c=(i%2==1)?5:2;
    }
    pixels.setPixelColor(i%24, color[c]); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.

 //   delay(delayval); // Delay for a period of time (in milliseconds).

  }
}
