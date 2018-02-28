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
	return (uint16_t) ((current * pixels) / max);
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
	upTo = howManyPixels(_current,_max,(int)_ledController->numPixels());
	for (i = 0 ; i < _ledController->numPixels() ; i++) {
		unsigned int color = (i <= upTo) ? Color(_r,_g,_b) : _black;
		if (i == upTo) {
			float scale = lastIntensity(_current,_max, (int)_ledController->numPixels());
			scale *= scale;
			color = Color( (byte)(scale * _r), (byte)(scale * _g), (byte)(scale * _b) );
		}
		_ledController->setPixelColor(i, color);
	}
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