#include <Arduino.h>

#include "io.h"

static const int touchPins[] = {7, 6, 5, 4};
static const int ledPins[] = {38, 37, 36, 35};
static const int buttonPins[] = {42, 41, 40, 39};

static void got_touch(void *arg)
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

static void button_isr(void *arg) {
	int i = (int)arg;

	Serial.printf("button %d\n", i);
}

void set_led(int led, int state)
{
	digitalWrite(ledPins[led], state);
}

void set_leds(int state)
{
	for (int i = 0; i < 4; i++)
		digitalWrite(ledPins[i], state);
}

void init_io(void)
{
	int touch_threshold = 40;

	touchSetDefaultThreshold(touch_threshold);

	for (int i = 0; i < 4; i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);
		touchAttachInterruptArg(touchPins[i],  got_touch, (void*)i, 0);

		pinMode(ledPins[i], OUTPUT);
		digitalWrite(ledPins[i], 0);
		attachInterruptArg(buttonPins[i], button_isr, (void*)i, RISING);
	}
}
