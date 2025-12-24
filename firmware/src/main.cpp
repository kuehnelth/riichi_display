#include <Arduino.h>

#include "ble.h"
#include "io.h"

void setup()
{
	Serial.begin(115200);

	init_io();

	setLEDs(1);
	
	init_ble();

	//init display
	//init game

	//led off
	delay(3000);

	setLEDs(0);
}

void loop()
{
	/*
	bool ret;
	if (deviceConnected) {
		if (game_state.old_active != game_state.active) {
			ret = showPartialUpdate();
			delay(100);
		} else {
			delay(100);
		}
	} else {
		connectScreen();
		delay(30000);
		Serial.println("loop");

		if (!deviceConnected) {
			standbyScreen();
		}

	}*/
	Serial.println("loop");
	delay(3000);

	return;
}
