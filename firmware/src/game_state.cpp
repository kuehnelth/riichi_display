#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Arduino.h>

#include "game_state.h"

Game_state game_state;

void game_state_parse(char *str)
{
	int type = 0;
	int item = 0;
	uint8_t wind = 0;

	int res, id, score;
	while (strlen(str)) {
		char *next = strchr(str, '\n');
		if (!next)
			break;
		next[0] = '\0';

		if (item == 0) {
			sscanf(str, "%" SCNu8 " %" SCNu8 " %" SCNu8, &game_state.round,
			       &game_state.riichi_count, &game_state.honba_count);
			item++;
			type = 0;
		} else {
			Player *player;
			switch (type) {
			case 0:
				id = atoi(str);
				wind = id;
				break;
			case 1:
				score = atoi(str);
				break;
			case 2:
				player = &game_state.players[item - 1];
				strcpy(player->name, str);
				player->score = score;
				player->wind = (Wind)wind;
				break;
			default:
				break;
			}
			type = (++type) % 3;
			if (type == 0)
				item++;
		}
		str = &next[1];
	}

	game_state.active = 0xff;
	game_state.old_active = 0xff;
}

void dump_game_state(void)
{
	Serial.printf("Round %d honba %d riichi %d\n",
		      game_state.round,
		      game_state.honba_count,
		      game_state.riichi_count);
	Serial.println("Players:");

	for (int i = 0; i < 4; i++) {
		Player *p = &game_state.players[i];
		Serial.printf("Wind %c % 7d %s\n", "ESWN"[p->wind], p->score, p->name);
	}

}

void set_active_player(int p)
{
	game_state.active = p;
}
