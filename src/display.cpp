// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#include <cstdint>
#include <cstdio>
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include "MPLUS2_Regular24pt7b.h"
#include "MPLUS2_Regular9pt7b.h"
#include "MPLUS2_Medium12pt7b.h"

#include "qrcodegen.h"

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

static void display_print(const GFXfont *f, const char *s, alignH ah, alignV av, uint16_t offset_h, uint16_t offset_v);

static void printQr(const uint8_t qrcode[]) {
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
	//display.nextPage();
}

#include <BLEDevice.h>
uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX] PROGMEM;
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
void connectScreen(void) {
    const char *text = BLEDevice::getAddress().toString().c_str();
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

    // Make and print the QR Code symbol
    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
                                   qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (ok) {
		display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
		display.refresh(0);
		display.setRotation(0);
		display.setTextColor(GxEPD_BLACK);
        printQr(qrcode);
		display_print(&MPLUS2_Medium12pt7b, "Scan to connect", CENTER, BOTTOM, 0, 20);
		display.nextPage();
		display.hibernate();
	}
}

void standbyScreen(void) {
	display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
	display.setRotation(0);
	display.setTextColor(GxEPD_BLACK);
	display_print(&MPLUS2_Regular9pt7b, "Standby mode", CENTER, MIDDLE, 0, 20);
	display_print(&MPLUS2_Regular9pt7b, "touch corners to wakeup", CENTER, MIDDLE, 0, 35);
	display.nextPage();

	display.hibernate();
}

static void display_print(const GFXfont *f, const char *s, alignH ah, alignV av, uint16_t offset_h, uint16_t offset_v)
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



#include "display.h"
/*
#include "USB.h"
extern USBCDC USBSerial;
#undef Serial
#define Serial USBSerial
*/

#define LED_1 38
#define LED_2 37
#define LED_3 36
#define LED_4 35

SPIClass hspi(HSPI);

void init_display(void)
{
	  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

	Serial.printf("CS %d res %d DC %d\n", CS_PIN, RES_PIN, DC_PIN);
	pinMode(CS_PIN, OUTPUT);
	pinMode(RES_PIN, OUTPUT);
	pinMode(DC_PIN, OUTPUT);
	//Serial.printf("CS %d res %d DC %d\n", CS_PIN, RES_PIN, DC_PIN);

	  hspi.begin(12, 18, 11, 7); 
         display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));

	display.init(115200, true, 50, false);
	//display.firstPage();
	//display.nextPage();
	display.setPartialWindow(50, 0, 300, 300);
	if (display.epd2.hasFastPartialUpdate) {

		//connectScreen();
		//standbyScreen();
	}
	display.hibernate();
}

char names[][32] = { "Player 1", "Player 2", "Player 3", "Player 4" };
int scores[4] = { 30000, 27000, 10000, 33000 };
int state[3] = {0, 0, 0}; //round (0==E1, 1==E2,.. ), riichi count, honba count

const char winds[] = {'E', 'S', 'W', 'N'};

static uint8_t i;

static bool showUpdate(bool full)
{
	char buf[32] = {};
	int active = game_state.active;

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
	if (full)
		display.refresh(0);
	game_state.old_active = active;
	display.hibernate();
	return 0;

}

bool showFullUpdate()
{
	return showUpdate(true);
}

bool showPartialUpdate()
{
	return showUpdate(false);
}
