#include <TimerOne.h>
#include "LPD6803_thiez.h"

/*****************************************************************************
 * Example to control LPD6803-based RGB LED Modules in a strand
 * Original code by Bliptronics.com Ben Moyes 2009
 * Use this as you wish, but please give credit, or at least buy some of my LEDs!
 *
 * Code cleaned up and Object-ified by ladyada, should be a bit easier to use
 *
 * Library Optimized for fast refresh rates 2011 by michu@neophob.com
 *****************************************************************************/

static uint8_t cpumax = 50;

// the arrays of ints that hold each LED's 15 bit color values
//static uint16_t *pixels;
//static uint16_t numLEDs;

//static uint8_t dataPin, clockPin;

enum lpd6803mode {
  START,
  HEADER,
  DATA,
  DONE
};

struct lpd6803strip {
  int refCount;	// Reference counting, so the interrupt can kill strips that are no longer used.
  lpd6803mode SendMode;
  uint8_t dataPin;
  uint8_t clockPin;

  byte BitCount;
  byte BlankCounter;
  byte lastdata;

  uint16_t LedIndex;
  uint16_t swapAsap;
  uint16_t numLEDs;

  uint16_t *pixels;

  struct lpd6803strip * next;
};

static lpd6803strip * strip = NULL;

//static lpd6803mode SendMode;   // Used in interrupt 0=start,1=header,2=data,3=data done
//static byte  BitCount;   // Used in interrupt
//static uint16_t  LedIndex;   // Used in interrupt - Which LED we are sending.
//static byte  BlankCounter;  //Used in interrupt.

//static byte lastdata = 0;
//static uint16_t swapAsap = 0;   //flag to indicate that the colors need an update asap

//Interrupt routine.
//Frequency was set in setup(). Called once for every bit of data sent
//In your code, set global Sendmode to 0 to re-send the data to the pixels
//Otherwise it will just send clocks.
void LedOut() {
  // PORTB |= _BV(5);    // port 13 LED for timing debug
  lpd6803strip * currStrip;
  for (currStrip = strip ; currStrip != NULL ; currStrip = currStrip->next) {
    switch(currStrip->SendMode) {
    case DONE:            //Done..just send clocks with zero data, or free strip memory when it is no longer owned.
      if (currStrip->refCount <= 0) {
        if (strip == currStrip) {
          strip = currStrip->next;
        } 
        else {
          lpd6803strip * prev;
          for (prev = strip ; prev->next != currStrip ; prev = prev->next);
          prev = (prev->next)->next;
        }
        free( currStrip->pixels );
        free( currStrip );
        continue;
      }
      if (currStrip->swapAsap>0) {
        if(!currStrip->BlankCounter)    //AS SOON AS CURRENT pwm IS DONE. BlankCounter 
        {
          currStrip->BitCount = 0;
          currStrip->LedIndex = currStrip->swapAsap;  //set current led
          currStrip->SendMode = HEADER;
          currStrip->swapAsap = 0;
        }   	
      }
      break;

    case DATA:               //Sending Data
      if ((1 << (15-currStrip->BitCount)) & currStrip->pixels[currStrip->LedIndex]) {
        if (!currStrip->lastdata) {     // digitalwrites take a long time, avoid if possible
          // If not the first bit then output the next bits 
          // (Starting with MSB bit 15 down.)
          digitalWrite(currStrip->dataPin, 1);
          currStrip->lastdata = 1;
        }
      } 
      else if (currStrip->lastdata) {       // digitalwrites take a long time, avoid if possible
        digitalWrite(currStrip->dataPin, 0);
        currStrip->lastdata = 0;
      }
      (currStrip->BitCount)++;

      if(currStrip->BitCount == 16)    //Last bit?
      {
        (currStrip->LedIndex)++;        //Move to next LED
        if (currStrip->LedIndex < currStrip->numLEDs) //Still more leds to go or are we done?
        {
          currStrip->BitCount=0;      //Start from the fist bit of the next LED
        } 
        else {
          // no longer sending data, set the data pin low
          digitalWrite(currStrip->dataPin, 0);
          currStrip->lastdata = 0; // this is a lite optimization
          currStrip->SendMode = DONE;  //No more LEDs to go, we are done!
        }
      }
      break;      
    case HEADER:            //Header
      if (currStrip->BitCount < 32) {
        digitalWrite(currStrip->dataPin, 0);
        currStrip->lastdata = 0;
        (currStrip->BitCount)++;
        if (currStrip->BitCount==32) {
          currStrip->SendMode = DATA;      //If this was the last bit of header then move on to data.
          currStrip->LedIndex = 0;
          currStrip->BitCount = 0;
        }
      }
      break;
    case START:            //Start
      if (!currStrip->BlankCounter)    //AS SOON AS CURRENT pwm IS DONE. BlankCounter 
      {
        currStrip->BitCount = 0;
        currStrip->LedIndex = 0;
        currStrip->SendMode = HEADER; 
      }  
      break;   
    }

    // Clock out data (or clock LEDs)
    digitalWrite(currStrip->clockPin, HIGH);
    digitalWrite(currStrip->clockPin, LOW);

    //Keep track of where the LEDs are at in their pwm cycle. 
    currStrip->BlankCounter++;
  }
  // PORTB &= ~_BV(5);   // pin 13 digital output debug
}

//---
LPD6803::LPD6803(uint16_t n, uint8_t dpin, uint8_t cpin) {
  myStrip = NULL;
  init(n,dpin,cpin);
}

LPD6803::~LPD6803() {
  (myStrip->refCount)--;
}

void LPD6803::init(uint16_t n, uint8_t dpin, uint8_t cpin) {
  if (myStrip != NULL) {
    (myStrip->refCount)--;
  }
  lpd6803strip * newStrip = (lpd6803strip*)malloc( sizeof(lpd6803strip) );
  newStrip->refCount = 1;
  newStrip->dataPin = dpin;
  newStrip->clockPin = cpin;
  newStrip->numLEDs = n;
  newStrip->pixels = (uint16_t *)malloc(newStrip->numLEDs * sizeof(uint16_t));
  for (uint16_t i=0; i< newStrip->numLEDs; i++) {
    setPixelColor(i, 0, 0, 0);
  }

  newStrip->SendMode = START;
  newStrip->BitCount = newStrip->LedIndex = newStrip->BlankCounter = 0;
  newStrip->next = NULL;
  newStrip->lastdata = 0;
  newStrip->swapAsap = 0;
  //cpumax = 50;
  myStrip = newStrip;
  if (strip == NULL) {
    strip = newStrip;
  } 
  else {
    lpd6803strip * current;
    for (current = strip ; current->next != NULL ; current = current->next);
    current->next = newStrip;
  }
}

//---
void LPD6803::begin(void) {
  pinMode(myStrip->dataPin, OUTPUT);
  pinMode(myStrip->clockPin, OUTPUT);

  setCPUmax(cpumax);

  Timer1.attachInterrupt(LedOut);  // attaches callback() as a timer overflow interrupt
}

//---
uint16_t LPD6803::numPixels(void) {
  return myStrip->numLEDs;
}

//---
void LPD6803::setCPUmax(uint8_t m) {
  cpumax = m;

  // each clock out takes 20 microseconds max
  long time = 100;
  time *= 20;   // 20 microseconds per
  time /= m;    // how long between timers
  Timer1.initialize(time);
}

//---
void LPD6803::show(void) {
  myStrip->SendMode = START;
}

//---
void LPD6803::doSwapBuffersAsap(uint16_t idx) {
  myStrip->swapAsap = idx;
}

//---
void LPD6803::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  uint16_t data;

  if (n > myStrip->numLEDs) return;

  data = g & 0x1F;
  data <<= 5;
  data |= b & 0x1F;
  data <<= 5;
  data |= r & 0x1F;
  data |= 0x8000;

  myStrip->pixels[n] = data;
}

//---
void LPD6803::setPixelColor(uint16_t n, uint16_t c) {
  if (n > myStrip->numLEDs) return;

  myStrip->pixels[n] = 0x8000 | c;
}

//---
void * LPD6803::getCurrentStrip() {
  return (void*) strip;
}

LPD6803::LPD6803(const LPD6803& obj) {
  myStrip = obj.myStrip;
  (myStrip->refCount)++;
}

LPD6803::LPD6803() {
  myStrip = NULL;
}

LPD6803& LPD6803::operator=(const LPD6803& rhs) {
  if (myStrip != NULL) {
    (myStrip->refCount)--;
  }
  myStrip = rhs.myStrip;
  (myStrip->refCount)++;
  return *this;
}

int LPD6803::isDone() {
  return myStrip->SendMode == DONE;
}

int LPD6803::allDone() {
  for (lpd6803strip * curr = strip ; curr != NULL ; curr = curr->next) {
    if (curr->SendMode != DONE) return 0;
  }
  return 1;
}


