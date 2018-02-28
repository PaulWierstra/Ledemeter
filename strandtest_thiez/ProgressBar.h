#ifndef ProgressBar_h_
#define ProgressBar_h_

#include "LPD6803.h"

#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class ProgressBar {
	public:
	/**
	 * Creates a ProgressBar which will use the LPD6803 led controller, and has max as the maximum progress value.
	 * Initial progress is 0.
	 */
	ProgressBar(LPD6803 *ledController, float max);
	/**
	 * Changes the max value. Does *not* update the leds.
	 */
	void setMax(float);
	/**
	 * Updates the current progress. Does *not* update the leds.
	 */
	void setProgress(float);
	/**
	 * Changes the color of the progress bar. Only the lowest 5 bits are used (possible values: 0-31).
	 * Does *not* update the leds.
	 */
	void setColor(byte r, byte g, byte b);
	/**
	 * Updates the leds.
	 */
	void update();
	/**
	 * Updates the leds and waits for 'wait' milliseconds afterwards.
	 */
	void update(unsigned long wait);
	/**
	 * Turns off the leds. Does not change internal values of color/progress/max: calling update will restore the previous state.
	 */
	void clear(void);
	private:
	LPD6803 * _ledController;
	float _current;
	float _max;
	byte _r;
	byte _g;
	byte _b;
};

#endif