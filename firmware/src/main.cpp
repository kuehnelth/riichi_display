#include <Arduino.h>

#include "ble.h"
#include "display.h"
#include "io.h"
#include "game_state.h"

void setup()
{
	Serial.begin(115200);

	init_io();

	set_leds(1);
	
	init_ble();

	init_display();

	set_leds(0);
}

void loop()
{
	if (deviceConnected) {
		update_game_screen();
		delay(100);
	} else {
		show_connect_screen();
		delay(60000);

		if (!deviceConnected) {
			show_standby_screen();
			deinit_ble();
			Serial.flush();
			Serial.end();
			
			esp_deep_sleep_start();
		}
	}
	return;
}
