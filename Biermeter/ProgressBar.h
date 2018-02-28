#ifndef ProgressBar_h_
#define ProgressBar_h_

#include "LPD6803_thiez.h"

#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class ProgressBar {
	public:
        /**
	 * Sets the colors of the marks. Marks equal or below 'progress' will be the first color, marks higher than 'progress' will be the latter.
	 */
	void setMarkColors(byte r_before, byte g_before, byte b_before, byte r_after, byte g_after, byte b_after);
	/**
	 * Adds a marks starting at offset, the next at offset + step, the next at offset + step + step, etc.
	 */
	void setMarks(float offset, float step);
        void clearMarks();
	/**
	 * Creates a ProgressBar which will use the LPD6803 led controller, and has max as the maximum progress value.
	 * Initial progress is 0.
	 */
	ProgressBar(LPD6803 *ledController, float max);
        /**
         * Creates an invalid ProgressBar instance.
         */
        ProgressBar();
        ProgressBar& operator=(const ProgressBar& rhs);
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
	byte _r_before;
	byte _g_before;
	byte _b_before;
	byte _r_after;
	byte _g_after;
	byte _b_after;
	float _marks_offset;
	float _marks_step;
};

#endif
