#include "HardwareSerial.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include "esp_sleep.h"
#include <Arduino.h>

#include "display.h"
#include "ble.h"
#include "game_state.h"
#include <cstring>
#include <unistd.h>


#include "USB.h"
/*
#if !ARDUINO_USB_CDC_ON_BOOT
USBCDC USBSerial;
#endif

#undef Serial
#define Serial USBSerial
*/
const int touchPins[] = {7, 6, 5, 4};
const int ledPins[] = {38, 37, 36, 35};
const int buttonPins[] = {42, 41, 40, 39};
void gotTouch(int i)
{
	int state = touchInterruptGetLastStatus(touchPins[i]);
	Serial.printf("touch %d state %d\n", i, state);
	if (state) {
		game_state.active = i;
		//analogWrite(ledPins[i], 10);
		digitalWrite(ledPins[i], 1);
	} else {
		game_state.active = 0xff;
		//analogWrite(ledPins[i], 0);
		digitalWrite(ledPins[i], 0);
	}
}

void gotTouch1() {
	gotTouch(0);
}

void gotTouch2() {
	gotTouch(1);
}

void gotTouch3() {
	gotTouch(2);
}

void gotTouch4() {
	gotTouch(3);
}

void buttonISR(void *arg) {
	int i = (int)arg;

	Serial.printf("button %d\n", i);
}

void setup()
{
	int i;
	#define TOUCH_THRESHOLD 10
	//USB.productName("riichi display");

        //USB.begin();
	Serial.begin(115200);
        //USBSerial.begin();

	touchSetDefaultThreshold(TOUCH_THRESHOLD);

	/*
	// doesn't work for whatever reason
	for (long i = 0; i < 4; i++)
		touchAttachInterruptArg(touchPins[i],  gotTouch, (void*)i, 0);
	*/

	touchAttachInterrupt(touchPins[0],  gotTouch1, 0);
	touchAttachInterrupt(touchPins[1],  gotTouch2, 0);
	touchAttachInterrupt(touchPins[2],  gotTouch3, 0);
	touchAttachInterrupt(touchPins[3],  gotTouch4, 0);

	for (long i = 0; i < 4; i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);

		pinMode(ledPins[i], OUTPUT);
		digitalWrite(ledPins[i], 1);
		attachInterruptArg(buttonPins[i], buttonISR, (void*)i, RISING);
	}

	for (long i = 0; i < 4; i++)
		touchSleepWakeUpEnable(touchPins[i], TOUCH_THRESHOLD);


	setupBle();
	Serial.println("init display");

	game_state.round = 1;
	game_state.active = 0xff;
	strcpy(game_state.players[0].name, "Player 1");
	game_state.players[0].score = 30000;
	game_state.players[0].wind = EAST;
	strcpy(game_state.players[1].name, "Player 2");
	game_state.players[1].score = 30000;
	game_state.players[1].wind = SOUTH;
	strcpy(game_state.players[2].name, "Player 3");
	game_state.players[2].score = 30000;
	game_state.players[2].wind = WEST;
	strcpy(game_state.players[3].name, "Player 4");
	game_state.players[3].score = 30000;
	game_state.players[3].wind = NORTH;

	Serial.println("call init display");
	init_display();
	Serial.println("setup done");

	for (long i = 0; i < 4; i++) {
		digitalWrite(ledPins[i], 0);
	}

}

#include <esp_bt.h>
#include <esp_wifi.h>

static uint8_t old_active = 0xff;
void loop()
{
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
			stopBle();
			esp_sleep_disable_bt_wakeup();
			esp_sleep_disable_wifi_beacon_wakeup();
			esp_deep_sleep_disable_rom_logging();
			btStop();
			esp_wifi_stop();
			esp_bt_controller_disable();
			Serial.flush();
			Serial.end();
			esp_deep_sleep_start();
		}
		
	}

	return;
}
