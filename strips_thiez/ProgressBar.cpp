#include "ProgressBar.h"

static unsigned int Color(byte r, byte g, byte b) {
  //Take the lowest 5 bits of each value and append them end to end
  return (
			(((unsigned int)b & 0x1F)<<10)
			| (((unsigned int)g & 0x1F)<<5)
			| (unsigned int)r & 0x1F
		);
}

static uint16_t howManyPixels(float current, float max, int pixels) {
	if (current < 0.0)
		current = 0.0;
	if (max < 0.0)
		max = 0.0;
	if (current > max) current = max;
	return (uint16_t) floor((current * pixels) / max);
}

static float lastIntensity(float current, float max, int pixels) {
	if (current < 0.0)
		current = 0.0;
	if (max < 0.0)
		max = 0.0;
	float f = (current * pixels) / max;
	return f - floor(f);
}

ProgressBar::ProgressBar(LPD6803 *ledController, float max) {
	_ledController = ledController;
	_max = max;
	_current = 0;
	_r = 0;
	_g = 31;
	_b = 0;
	_r_before = 31;
	_g_before = 0;
	_b_before = 0;
	_r_after = 1;
	_g_after = 0;
	_b_after = 0;
	_marks_offset = max + 1.0;
	_marks_step = 1.0;
}

static const uint16_t _black = Color(0,0,0);

void ProgressBar::setMax(float max) {
	_max = max;
}

void ProgressBar::setProgress(float current) {
	_current = current;
}

void ProgressBar::setColor(byte r, byte g, byte b) {
	_r = r;
	_g = g;
	_b = b;
}

void ProgressBar::update() {
	int i;
	int upTo;
        //int upUntil;
	unsigned int cols[ _ledController->numPixels() ];
	upTo = howManyPixels(_current,_max,(int)_ledController->numPixels());
        //
        //upUntil = howManyPixels(_current,_max,(int)_ledController->numPixels()) +10; 
	for (i = 0 ; i < _ledController->numPixels() ; i++) {
		unsigned int color = (i <= upTo) ? Color(_r,_g,_b) : _black;
		if (i == upTo) {
			float scale = lastIntensity(_current,_max, (int)_ledController->numPixels());
			// scale *= scale;
			color = Color( (byte)(scale * _r), (byte)(scale * _g), (byte)(scale * _b) );
		}
		// _ledController->setPixelColor(i, color);
		cols[i] = color;
	}
	for (float mark = _marks_offset ; mark < _max ; mark += _marks_steps) {
		unsigned int color = (mark <= _current) ? Color(_r_before,_g_before,_b_before) : Color(_r_after,_g_after,_b_after);
		cols[howManyPixels(mark,_max,(int)_ledController->numPixels())] = color;
	}
	for (i = 0 ; i < _ledController->numPixels() ; i++) {
		_ledController->setPixelColor(i, cols[i]);
	}
        /*for (i = _ledController->numPixels()-1 ; (howManyPixels(_current,_max,(int)_ledController->numPixels())) ; i--) {
                _ledController->setPixelColor(i, cols[i]);
        }*/
	_ledController->show();
}

void ProgressBar::update(unsigned long wait) {
	update();
	delay(wait);
}

void ProgressBar::clear(void) {
	int i;
	for (i = 0 ; i < _ledController->numPixels() ; i++)
		_ledController->setPixelColor(i, _black);
	_ledController->show();
}

void ProgressBar::setMarkColors(byte r_before, byte g_before, byte b_before, byte r_after, byte g_after, byte b_after) {
	_r_before = r_before;
	_g_before = g_before;
	_b_before = b_before;
	_r_after = r_after;
	_g_after = g_after;
	_b_after = b_after;
}

void ProgressBar::addMarks(float offset, float step) {
	if (step <= 0.0) return;
	_mark_offset = offset;
	_mark_step = step;
}

//void ProgressBar::delMark(float value) {
//	_marks[_num_marks++] = value;
//}

void ProgressBar::clearMarks() {
	_mark_offset = max + 1.0;
}
