#include <TimerOne.h>
#include "LPD6803_thiez.h"
#include <math.h>

#include "ProgressBar.h"

//Example to control LPD6803-based RGB LED Modules in a strand
// Original code by Bliptronics.com Ben Moyes 2009
//Use this as you wish, but please give credit, or at least buy some of my LEDs!

// Code cleaned up and Object-ified by ladyada, should be a bit easier to use

/*****************************************************************************/

#define LITER_MAX 10000
#define NUM_LEDS 50
#define NUM_LEDS2 20
#define NUM_LIGHTS 8

int curr_liters = 0;

int w = 1000; //wait time for debug

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPin = 2;       // 'yellow' wire
int clockPin = 3;      // 'green' wire
// Don't forget to connect 'blue' to ground and 'red' to +5V

// Timer 1 is also used by the strip to send pixel clocks

// Set the first variable to the NUMBER of pixels. 20 = 20 pixels in a row
LPD6803 strip = LPD6803(NUM_LEDS, dataPin, clockPin);
LPD6803 strip2 = LPD6803(20, 4, 5);

int howManyPixels(int current, int max, int numPixs) {
  long n = (long)current;
  n *= numPixs;
  n /= max;
  return (int) n;
}

int howManyPixels(int current, int max) {
  return howManyPixels(current,max,NUM_LEDS);
}

void setup() {
  //debugging
  Serial.begin(9600);
  Serial.println("Hello world");
  delay(w);

  // The Arduino needs to clock out the data to the pixels
  // this happens in interrupt timer 1, we can change how often
  // to call the interrupt. setting CPUmax to 100 will take nearly all all the
  // time to do the pixel updates and a nicer/faster display, 
  // especially with strands of over 100 dots.
  // (Note that the max is 'pessimistic', its probably 10% or 20% less in reality)

  Serial.println("Setting CPU max at 50%");
  strip.setCPUmax(50);  // start with 50% CPU usage. up this if the strand flickers or is slow
  delay(w);
  Serial.println("Start up the LED counter");
  strip.begin();
  strip2.begin();
  delay(w);
  Serial.println("Update the strip, to start they are all 'off'");
  strip.show();
  strip2.show();
  delay(w);
}


void loop() {
  // Some example procedures showing how to display to the pixels
  //curr_liters = (curr_liters + 1000) % LITER_MAX;
  //Serial.println("Starting colorwipe 63,0,0");
  //colorWipe(Color(63, 0, 0), 50);
  //curr_liters = (curr_liters + 1000) % LITER_MAX;
  //Serial.println("Starting colorwipe 0,63,0");
  //colorWipe(Color(0, 63, 0), 50);
  //curr_liters = (curr_liters + 1000) % LITER_MAX;
  //Serial.println("Starting colorwipe 0,0,63");
  //colorWipe(Color(0, 0, 63), 50);
  //curr_liters = (curr_liters + 1000) % LITER_MAX;
  //Serial.println("Starting rainbow");
  //rainbow(50);
  //curr_liters = (curr_liters + 1000) % LITER_MAX;
  Serial.println("Starting Knight Rider");
  knightRider(50);
  //  Serial.println("Starting Rainbowcycle");
  //  rainbowCycle(50);
  /*Serial.println("Starting bier-meter");
   	ProgressBar literMeter = ProgressBar(&strip, LITER_MAX);
   	float bier;
   	for (bier = 0.0 ; bier < LITER_MAX ; bier += 2.5) {
   		literMeter.setProgress(bier);
   		literMeter.update(50);
   		if (fmod(bier,1000) < 0.01 && bier > 0.01) {
   			Serial.println("Tank leeg!");
   			int j;
   			for (j = 0 ; j < 1 ; j++) {
   				literMeter.setColor(20,0,20);
   				literMeter.update(100);
   				literMeter.setColor(0,31,0);
   				literMeter.update(100);
   			}
   		}
   	}
   	literMeter.clear();
   	Serial.println("Tanks leeg? Opnieuw!");
   	delay(3000);*/
}

void rainbow(uint8_t wait) {
  int i, j;
  int max_ledNum = howManyPixels(curr_liters, LITER_MAX); 
  for (j=0; j < 96 * 3; j++) {     // 3 cycles of all 96 colors in the wheel
    for (i=0; i < strip.numPixels() && i < max_ledNum; i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 96));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

void wipe() {
  colorWipe(Color(0,0,0), 0);
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  wipe();
  for (j=0; j < 96 * 5; j++) {     // 5 cycles of all 96 colors in the wheel
    int max_ledNum = howManyPixels(j, 96*5);
    for (i=0; i < strip.numPixels() && i < max_ledNum; i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 96 / strip.numPixels()) + j) % 96) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
  wipe();
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint16_t c, uint8_t wait) {
  int i;
  for (i=0; i < strip.numPixels() ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

struct RGB {
  float r;
  float g;
  float b;
};

struct MovingLight {
  float position;
  float direction;
  float r;
  float g;
  float b;
};


void knightRider(uint8_t wait) {
  int i;
  RGB cols[NUM_LEDS];
  RGB cols2[NUM_LEDS2];
  MovingLight lights[NUM_LIGHTS];
  MovingLight lights2[NUM_LIGHTS];

  for (i = 0 ; i < NUM_LIGHTS ; i++) {
    lights[i].position = (float)random(NUM_LEDS);
    lights2[i].position = (float)random(NUM_LEDS2);
    lights[i].direction = (((float)random(20)) - 10.0) / 10.0 ;
    lights2[i].direction = lights[i].direction;
    lights[i].r = (float)random(32);
    lights[i].g = (float)random(32);
    lights[i].b = (float)random(32);
    lights2[i].r = lights[i].r;
    lights2[i].g = lights[i].g;
    lights2[i].b = lights[i].b;
  }
  for (i = 0 ; i < NUM_LEDS ; i++) {
    cols[i].r = 0;
    cols[i].g = 0;
    cols[i].b = 0;
  }
  for (i = 0 ; i < NUM_LEDS2 ; i++) {
    cols2[i].r = 0;
    cols2[i].g = 0;
    cols2[i].b = 0; 
  }
  int updates;
  //for (updates = 0 ; updates < 10000 ; updates++) {
  while (1) {
    for (i = 0 ; i < NUM_LEDS ; i++) {
      cols[i].r *= 0.7;
      cols[i].g *= 0.6;
      cols[i].b *= 0.4;
    }
    for (i = 0 ; i < NUM_LEDS2 ; i++) {
      cols2[i].r *= 0.7;
      cols2[i].g *= 0.6;
      cols2[i].b *= 0.4;
    }
    for (i = 0 ; i < NUM_LIGHTS ; i++) {
      MovingLight *light = &lights[i];
      RGB *rgb = &cols[ (int)light->position ];

      if (random(100) == 500) {
        light->r = (float)random(32);
        light->g = (float)random(32);
        light->b = (float)random(32);
      }

      rgb->r += light->r;
      rgb->g += light->g;
      rgb->b += light->b;

      light->position += light->direction;
      if ( light->position < 0.0 ) {
        light->position = 0;
        light->direction *= -1.0;
      } 
      else if ( ((int)light->position) >= NUM_LEDS ) {
        light->position = (float)(NUM_LEDS - 1);
        light->direction *= -1.0;
      }
    }
    for (i = 0 ; i < NUM_LIGHTS ; i++) {
      MovingLight *light = &lights2[i];
      RGB *rgb = &cols2[ (int)light->position ];

      if (random(100) == 500) {
        light->r = (float)random(32);
        light->g = (float)random(32);
        light->b = (float)random(32);
      }

      rgb->r += light->r;
      rgb->g += light->g;
      rgb->b += light->b;

      light->position += light->direction;
      if ( light->position < 0.0 ) {
        light->position = 0;
        light->direction *= -1.0;
      } 
      else if ( ((int)light->position) >= NUM_LEDS2 ) {
        light->position = (float)(NUM_LEDS2 - 1);
        light->direction *= -1.0;
      }
    }
    for (i = 0 ; i < NUM_LEDS ; i++) {
      if (cols[i].r > 31.0) cols[i].r = 31.0;
      if (cols[i].g > 31.0) cols[i].g = 31.0;
      if (cols[i].b > 31.0) cols[i].b = 31.0;
    }
    for (i = 0 ; i < NUM_LEDS ; i++) {
      strip.setPixelColor(i, Color((byte)floor(cols[i].r), (byte)floor(cols[i].g), (byte)floor(cols[i].b)));
    }
    for (i = 0 ; i < NUM_LEDS2 ; i++) {
      if (cols2[i].r > 31.0) cols2[i].r = 31.0;
      if (cols2[i].g > 31.0) cols2[i].g = 31.0;
      if (cols2[i].b > 31.0) cols2[i].b = 31.0;
    }
    for (i = 0 ; i < NUM_LEDS2 ; i++) {
      strip2.setPixelColor(i, Color((byte)floor(cols2[i].r), (byte)floor(cols2[i].g), (byte)floor(cols2[i].b)));
    }
    strip.show();
    strip2.show();
    delay(wait);
  }
  /*
	int times;
   	for (times = 0 ; times < 10 ; times++) {
   		for (i = 0 ; i < NUM_LEDS ; i++) {
   			int j;
   			for (j = 0 ; j < NUM_LEDS ; j++) {
   				if (i == j) {
   					cols[i].r = 31;
   				} else if ((NUM_LEDS - i) == j) {
   					cols[i].g = 31;
   				} else {
   					cols[i].r = cols[i].r / 2;
   					cols[i].g = cols[i].g / 2;
   					cols[i].b = cols[i].b / 2;
   				}
   				strip.setPixelColor(j, Color(cols[i].r, cols[i].g, cols[i].b));
   			}
   			strip.show();
   			delay(wait);
   		}
   		for (i = NUM_LEDS - 1; i >= 0 ; i--) {
   			int j;
   			for (j = 0 ; j < NUM_LEDS ; j++) {
   				if (i == j) {
   					cols[i].r = 31;
   				} else if ((NUM_LEDS - i) == j) {
   					cols[i].g = 31;
   				} else {	
   					cols[i].r = cols[i].r / 2;
   					cols[i].g = cols[i].g / 2;
   					cols[i].b = cols[i].b / 2;
   				}
   				strip.setPixelColor(j, Color(cols[i].r, cols[i].g, cols[i].b));
   			}
   			strip.show();
   			delay(wait);
   		}
   	}*/
  wipe();
}

/* Helper functions */

// Create a 15 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b)
{
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)b & 0x1F )<<10 | ((unsigned int)g & 0x1F)<<5 | (unsigned int)r & 0x1F);
}

//Input a value 0 to 127 to get a color value.
//The colours are a transition r - g -b - back to r
unsigned int Wheel(byte WheelPos)
{
  byte r,g,b;
  switch(WheelPos >> 5)
  {
  case 0:
    r=31- WheelPos % 32;   //Red down
    g=WheelPos % 32;      // Green up
    b=0;                  //blue off
    break; 
  case 1:
    g=31- WheelPos % 32;  //green down
    b=WheelPos % 32;      //blue up
    r=0;                  //red off
    break; 
  case 2:
    b=31- WheelPos % 32;  //blue down 
    r=WheelPos % 32;      //red up
    g=0;                  //green off
    break; 
  }
  return(Color(r,g,b));
}




