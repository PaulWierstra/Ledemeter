#include <TimerOne.h>

#include "LPD6803.h"
#include "SPI.h"

// Example to control LPD8806-based RGB LED Modules in a strip

/*****************************************************************************/

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPin = 2;   
int clockPin = 3; 

// Set the first variable to the NUMBER of pixels. 32 = 32 pixels in a row
// The LED strips are 32 LEDs per meter but you can extend/cut the strip
LPD6803 strip = LPD6803(50, dataPin, clockPin);

// you can also use hardware SPI, for ultra fast writes by leaving out the
// data and clock pin arguments. This will 'fix' the pins to the following:
// on Arduino 168/328 thats data = 11, and clock = pin 13
// on Megas thats data = 51, and clock = 52 
//LPD8806 strip = LPD8806(32);

void setup() {
  
	strip.setCPUmax(50);  // start with 50% CPU usage. up this if the strand flickers or is slow

  // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();
}

void loop() {
	int i, j;

	for (;;) {
		for (i=0; i < strip.numPixels(); i++) {
			for (j=0; j <= i; j++) {
				strip.setPixelColor(j, 63, 0, 0);  // turn all pixels off
			}
			for (j=i+1; j < strip.numPixels(); j++) {
				strip.setPixelColor(j, 0);  // turn all pixels off
			}

			strip.show();
		  delay(10);
		}
	}
}

