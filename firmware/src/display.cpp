#include <inttypes.h>

#include <Arduino.h>
#include <BLEDevice.h>
#include <GxEPD2_BW.h>
#include <qrcodegen.h>

#include "display.h"
#include "game_state.h"

#include "fonts/MPLUS2_Regular9pt7b.h"
#include "fonts/MPLUS2_Medium12pt7b.h"
#include "fonts/MPLUS2_Regular24pt7b.h"


#define CS_PIN 10
#define DC_PIN 17
#define RES_PIN 2
#define BUSY_PIN 1

#define SCK_PIN 12
#define MISO_PIN 13
#define MOSI_PIN 11

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

SPIClass hspi(HSPI);

// 4.2'' EPD Module
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
	GxEPD2_420_GDEY042T81(CS_PIN, DC_PIN, RES_PIN, BUSY_PIN)); // 400x300, SSD1683

void init_display(void)
{
	Serial.printf("CS %d res %d DC %d\n", CS_PIN, RES_PIN, DC_PIN);

	hspi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN); 
	display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));

	display.init(115200, true, 10, false);
	display.setPartialWindow(50, 0, 300, 300);
	display.epd2.selectFastFullUpdate(true);
	//display.hibernate();
}

static void display_print(const GFXfont *f, const char *s, alignH ah, alignV av,
			  uint16_t offset_h, uint16_t offset_v)

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
	display.setCursor(x, y);
	display.print(s);
}

static void clear_screen(void) {
	display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
	display.setRotation(0);
	display.setTextColor(GxEPD_BLACK);
}

void show_standby_screen(void) {
	clear_screen();
	display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
	display.setRotation(0);
	display.setTextColor(GxEPD_BLACK);
	display_print(&MPLUS2_Regular9pt7b, "Standby mode", CENTER, MIDDLE, 0, 20);
	display_print(&MPLUS2_Regular9pt7b, "touch corners to wakeup", CENTER, MIDDLE, 0, 35);
	display.nextPage();

	display.hibernate();
}

static void print_qr_code(const uint8_t qrcode[]) {
	int size = qrcodegen_getSize(qrcode);
	int scale = 5;
	int offset_x = (display.width() - size * scale) / 2;
	int offset_y = (display.height() - size * scale) / 2;
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if(qrcodegen_getModule(qrcode, x, y))
				display.fillRect(offset_x + (x * scale), offset_y + (y * scale), scale, scale, GxEPD_BLACK);
		}
	}
}

void show_connect_screen(void) {
	static uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX] PROGMEM;
	static uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	const char *text = BLEDevice::getAddress().toString().c_str();
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

	// Make and print the QR Code symbol
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
				       qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
				       qrcodegen_Mask_AUTO, true);
	if (ok) {
		clear_screen();
		print_qr_code(qrcode);
		display_print(&MPLUS2_Medium12pt7b, "Scan to connect", CENTER, BOTTOM, 0, 20);
		display.refresh(0);
		display.nextPage();
		display.hibernate();
	}
}

void update_game_screen(bool full_update) {
	char buf[32] = {};
	static const char winds[] = {'E', 'S', 'W', 'N'};
	int active = game_state.active;

	if (!game_state.need_redraw) {
		if (game_state.old_active == game_state.active)
			return;
		Serial.println("partial update");
	} else {
		full_update = true;
		game_state.need_redraw = false;
	}

	clear_screen();

	sprintf(buf, "%c%d", winds[(game_state.round - 1 ) / 4], 1 + ((game_state.round - 1) % 4));
	display_print(&MPLUS2_Regular24pt7b, buf, CENTER, MIDDLE, 0, 0);

	sprintf(buf, "%d Riichi", game_state.riichi_count);
	display_print(&MPLUS2_Regular9pt7b, buf, CENTER, MIDDLE, 0, 28);
	sprintf(buf, "%d Honba", game_state.honba_count);
	display_print(&MPLUS2_Regular9pt7b, buf, CENTER, MIDDLE, 0, 42);

	for (char r = 0; r < 4; r++) {
		char playerId = (3 - r) % 4;
		Player *player = &game_state.players[playerId];
		const char *name = player->name;

		display.setRotation(r);

		display_print(&MPLUS2_Regular9pt7b, name, CENTER, BOTTOM, 0, 6);
		if (active != 0xff) {
			int32_t score_diff = player->score - game_state.players[active].score;
			sprintf(buf, "%c%d", score_diff > 0 ? '+' : ' ', score_diff);
		} else {
			sprintf(buf, "%d", player->score);
		}
		display_print(&MPLUS2_Medium12pt7b, buf, CENTER, BOTTOM, 0, 21);
		sprintf(buf, "%c", winds[player->wind]);
		display_print(&MPLUS2_Regular24pt7b, buf, RIGHT, BOTTOM, 6, 6);
	}

	display.nextPage();
	if (full_update)
		display.refresh(0);
	game_state.old_active = active;
	display.hibernate();
	return;

}
