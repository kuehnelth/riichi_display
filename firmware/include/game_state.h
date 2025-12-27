#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <inttypes.h>

typedef enum {
EAST = 0,
SOUTH = 1,
WEST = 2,
NORTH = 3,
} Wind;

typedef struct {
	Wind wind;
	int32_t score;
	char name[32];
} Player;

typedef struct {
	uint8_t round;
	uint8_t honba_count;
	uint8_t riichi_count;
	uint8_t old_active;
	uint8_t active;
	Player players[4];
} Game_state;

extern Game_state game_state;

void game_state_parse(char *str);
void dump_game_state(void);
void set_active_player(int p);

#endif // GAME_STATE_H_
