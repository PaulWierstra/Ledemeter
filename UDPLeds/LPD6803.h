#ifndef LPD6803_h_
#define LPD6803_h_

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

struct lpd6803strip;

class LPD6803 {
 private:
 lpd6803strip * myStrip;
 public:
  LPD6803();
  LPD6803(uint16_t n, uint8_t dpin, uint8_t cpin);
  LPD6803(const LPD6803& obj);
  LPD6803& operator=(const LPD6803& rhs);
  ~LPD6803();
  void init(uint16_t n, uint8_t dpin, uint8_t cpin);
  void begin();
  void show();
  void doSwapBuffersAsap(uint16_t idx);
  void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
  void setPixelColor(uint16_t n, uint16_t c);
  void setCPUmax(uint8_t m);
  uint16_t numPixels(void);
  void * getCurrentStrip();
  int isDone();
  int allDone();
};

#endif
