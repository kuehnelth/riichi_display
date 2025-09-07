#include <Arduino.h>

#include "display.h"
#include "ble.h"

void setup()
{
	Serial.begin(115200);

	setupBle();
	Serial.print("init display");
	init_display();

}

void loop()
{
	delay(1000000);
	return;
}
