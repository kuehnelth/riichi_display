#include <Arduino.h>

#include "display.h"
#include "ble.h"
#include "game_state.h"
#include <cstdio>
#include <cstring>


void setup()
{
	Serial.begin(115200);

	setupBle();
	Serial.print("init display");

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

	init_display();

}

const int touchPins[] = {2, 15, 14, 12};
void loop()
{
	uint8_t old_active = game_state.active;
	int i;

	game_state.active = 0xff;
	for (i = 0; i < 4; i++) {
		uint8_t touch = touchRead(touchPins[i]);
		if (touch < 60) {
			game_state.active = i;
			break;
		}
	}

	if (old_active != game_state.active)
		showPartialUpdate();
	delay(1000);
	//delay(1000000);
	return;
}
