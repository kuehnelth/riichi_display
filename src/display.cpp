// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#include <cstdint>
#include <cstdio>
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include "MPLUS2_Regular24pt7b.h"
#include "MPLUS2_Regular9pt7b.h"
#include "MPLUS2_Medium12pt7b.h"

#include "game_state.h"

// 4.2'' EPD Module
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
	GxEPD2_420_GDEY042T81(/*CS=5*/ CS_PIN, /*DC=*/DC_PIN, /*RES=*/RES_PIN, /*BUSY=*/BUSY_PIN)); // 400x300, SSD1683

typedef enum {
LEFT,
CENTER,
RIGHT,
} alignH;

typedef enum {
TOP,
MIDDLE,
BOTTOM,
} alignV;

void display_print(const GFXfont *f, const char *s, alignH ah, alignV av, uint16_t offset_h, uint16_t offset_v)
{
	int16_t tbx, tby;
	uint16_t tbw, tbh;

	int16_t offsets_x[] = {50, 0, 50, 0};
	int16_t offsets_y[] = {0, 50, 0, 50};
	uint16_t margin = 5;
	uint16_t dh = min(display.height(), display.width());
	uint16_t dw = dh; // make the usable display square
	uint16_t r = display.getRotation();
	uint16_t x = offsets_x[r];
	uint16_t y = offsets_y[r];

	display.setFont(f);
	display.getTextBounds(s, 0, 0, &tbx, &tby, &tbw, &tbh);

	switch (ah) {
		case LEFT:
			x += margin + offset_v;
			break;
		case CENTER:
			x += ((dw - tbw) / 2) - tbx;
			break;
		case RIGHT:
			x += (dw - tbw) - tbx - margin;
			break;
	}
	switch (av) {
		case TOP:
			y += margin + offset_v;
			break;
		case MIDDLE:
			y += ((dh - tbh) / 2) - tby + offset_v;
			break;
		case BOTTOM:
			y += dh - margin - offset_v;
			break;
	}
	Serial.printf("display_print %s x %d y %d r %d\n", s, x, y, r);
	display.setCursor(x, y);
	display.print(s);
}



#include "display.h"

void init_display(void)
{
	pinMode(CS_PIN, OUTPUT);
	pinMode(RES_PIN, OUTPUT);
	pinMode(DC_PIN, OUTPUT);
	Serial.printf("CS %d res %d DC %d\n", CS_PIN, RES_PIN, DC_PIN);

	display.init(115200, true, 50, false);
	//display.firstPage();
	//display.nextPage();
	display.setPartialWindow(50, 0, 300, 300);
	if (display.epd2.hasFastPartialUpdate) {
		while (true) {
			Serial.println("show partial update");
			showPartialUpdate();
			break;
		}
	}
	display.hibernate();
}

char names[][32] = { "Player 1", "Player 2", "Player 3", "Player 4" };
int scores[4] = { 30000, 27000, 10000, 33000 };
int state[3] = {0, 0, 0}; //round (0==E1, 1==E2,.. ), riichi count, honba count

const char winds[] = {'E', 'S', 'W', 'N'};

void showPartialUpdate()
{
	char buf[32] = {};

	display.setRotation(0);

	display.setTextColor(GxEPD_BLACK);

	display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);

	sprintf(buf, "%c%d", winds[(game_state.round - 1 ) / 4], 1 + ((game_state.round - 1) % 4));
	display_print(&MPLUS2_Regular24pt7b, buf, CENTER, MIDDLE, 0, 0);

	sprintf(buf, "%d Riichi", game_state.riichi_count);
	display_print(&MPLUS2_Regular9pt7b, buf, CENTER, MIDDLE, 0, 28);
	sprintf(buf, "%d Honba", game_state.honba_count);
	display_print(&MPLUS2_Regular9pt7b, buf, CENTER, MIDDLE, 0, 42);
	//display_print(&FreeMono9pt7b, buf, CENTER, MIDDLE, 0, 26);

	for (uint16_t r = 0; r < 4; r++) {
		uint16_t playerId = (3 - r) % 4;

		display.setRotation(r);
		Player *player = &game_state.players[playerId];
		const char *name = player->name; //names[playerId];

		display_print(&MPLUS2_Regular9pt7b, name, CENTER, BOTTOM, 0, 6);
		if (game_state.active != 0xff) {
			int32_t score_diff = player->score - game_state.players[game_state.active].score;
			sprintf(buf, "%c%d", score_diff > 0 ? '+' : ' ', score_diff);
		} else {
			sprintf(buf, "%d", player->score);
		}
		display_print(&MPLUS2_Medium12pt7b, buf, CENTER, BOTTOM, 0, 21);
		sprintf(buf, "%c", winds[player->wind]);
		display_print(&MPLUS2_Regular24pt7b, buf, RIGHT, BOTTOM, 6, 6);
	}
	display.nextPage();
}
