#include <Arduino.h>

#include "io.h"

static const int touchPins[] = {7, 6, 5, 4};
static const int ledPins[] = {38, 37, 36, 35};
static const int buttonPins[] = {42, 41, 40, 39};

static void gotTouch(void *arg)
{
	int i = (int)arg;
	int state = touchInterruptGetLastStatus(touchPins[i]);

	Serial.printf("touch %d state %d\n", i, state);
	if (state) {
		digitalWrite(ledPins[i], 1);
	} else {
		digitalWrite(ledPins[i], 0);
	}
}

static void buttonISR(void *arg) {
	int i = (int)arg;

	Serial.printf("button %d\n", i);
}

void init_io(void)
{
	int touch_threshold = 40;

	touchSetDefaultThreshold(touch_threshold);

	for (int i = 0; i < 4; i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);
		touchAttachInterruptArg(touchPins[i],  gotTouch, (void*)i, 0);

		pinMode(ledPins[i], OUTPUT);
		digitalWrite(ledPins[i], 0);
		attachInterruptArg(buttonPins[i], buttonISR, (void*)i, RISING);
	}
}
