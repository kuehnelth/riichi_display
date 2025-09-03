// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#include <cstdint>
#include <cstdio>
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// ESP32-C3 CS(SS)=7,SCL(SCK)=4,SDA(MOSI)=6,BUSY=3,RES(RST)=2,DC=1
#define CS_PIN (5)
#define BUSY_PIN (4)
#define RES_PIN (17)
#define DC_PIN (16)

// 4.2'' EPD Module
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
	GxEPD2_420_GDEY042T81(/*CS=5*/ CS_PIN, /*DC=*/DC_PIN, /*RES=*/RES_PIN, /*BUSY=*/BUSY_PIN)); // 400x300, SSD1683

#include "display.h"

// 'E', 11x12px
const unsigned char epd_bitmap_E [] PROGMEM = {
	0x04, 0x00, 0xff, 0xe0, 0x04, 0x00, 0x7f, 0xc0, 0x44, 0x40, 0x7f, 0xc0, 0x44, 0x40, 0x3f, 0xc0,
	0x0e, 0x00, 0x15, 0x00, 0x24, 0x80, 0xc4, 0x60
};
// 'S', 11x12px
const unsigned char epd_bitmap_S [] PROGMEM = {
	0x04, 0x00, 0x04, 0x00, 0xff, 0xe0, 0x08, 0x00, 0xff, 0xe0, 0x91, 0x20, 0x8a, 0x20, 0xbf, 0xa0,
	0x84, 0x20, 0xbf, 0xa0, 0x84, 0x20, 0x84, 0x40
};
// 'W', 11x12px
const unsigned char epd_bitmap_W [] PROGMEM = {
	0x00, 0x00, 0xff, 0xe0, 0x09, 0x00, 0x09, 0x00, 0xff, 0xe0, 0x89, 0x20, 0x91, 0x20, 0x91, 0x20,
	0xe0, 0xe0, 0x80, 0x20, 0x80, 0x20, 0x7f, 0xe0
};
// 'N', 11x12px
const unsigned char epd_bitmap_N [] PROGMEM = {
	0x12, 0x00, 0x12, 0x00, 0x12, 0x00, 0x12, 0x00, 0xf2, 0x60, 0x13, 0x80, 0x12, 0x00, 0x12, 0x00,
	0x32, 0x00, 0xd2, 0x00, 0x12, 0x20, 0x11, 0xc0
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 192)
const int epd_bitmap_allArray_LEN = 4;
const unsigned char* epd_bitmap_allArray[4] = {
	epd_bitmap_E,
	epd_bitmap_S,
	epd_bitmap_W,
	epd_bitmap_N,
};

void test(void);
void showPartialUpdate();

int rot = 0;
void init_display(void)
{
	pinMode(CS_PIN, OUTPUT);
	pinMode(RES_PIN, OUTPUT);
	pinMode(DC_PIN, OUTPUT);

	display.init(115200, true, 50, false);
	display.firstPage();
	display.nextPage();
	//delay(10000);
	display.setPartialWindow(50, 0, 300, 300);
	if (display.epd2.hasFastPartialUpdate) {
		while (true) {
			Serial.println("show partial update");
			showPartialUpdate();
			break;
			//delay(10000);

			//rot = (++rot)%4;
		}
	}
	display.hibernate();
}

char names[][16] = { "Player 1", "Player 2", "Player 3", "Player 4" };
int scores[4] = { 25000, 0, -25000, 100000 };

void showPartialUpdate()
{
	// some useful background
	// use asymmetric values for test
	uint16_t box_x = 10;
	uint16_t box_y = 15;
	uint16_t box_w = 70;
	uint16_t box_h = 20;
	int16_t tbx, tby;
	uint16_t tbw, tbh;

	int16_t offsets_x[] = {50, 0, 50, 0};
	int16_t offsets_y[] = {0, 50, 0, 50};
	float value = 13.95;
	uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
	display.setFont(&FreeMonoBold9pt7b);
	display.setTextColor(GxEPD_BLACK);

	display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);


	for (uint16_t r = 0; r < 4; r++) {
		//uint16_t x = box_x + offsets_x[r];
		uint16_t x;
		uint16_t y;
		GFXcanvas1 canvas(box_w, box_h);
		display.setRotation(r);
		const char *name = names[(3 - r + rot) % 4];

		display.setFont(&FreeMonoBold9pt7b);
		display.getTextBounds(name, 0, 0, &tbx, &tby, &tbw, &tbh);
		x = offsets_x[r] + ((300 - tbw) / 2) - tbx;
		y = 300 - 5 - tbh;
		uint16_t cursor_y = y + tbh - 6;
		Serial.printf("rot %d x %d y %d cy %d\n", rot, x, y, cursor_y);

		//display.fillRect(x, y, box_w, box_h, GxEPD_WHITE);
		display.setCursor(x, cursor_y + offsets_y[r]);
		display.print(name);


		display.setFont(&FreeMonoBold12pt7b);
		char score[32] = {};
		sprintf(score, "%d", scores[(3 - r + rot) % 4]);
		display.getTextBounds(score, 0, 0, &tbx, &tby, &tbw, &tbh);
		x = offsets_x[r] + ((300 - tbw) / 2) - tbx;
		y = 300 - 5 - tbh;
		cursor_y = y + tbh - 6 - 15;
		Serial.printf("x %d y %d cy %d\n", x, y, cursor_y);

		//display.fillRect(x, y, box_w, box_h, GxEPD_WHITE);
		display.setCursor(x, cursor_y + offsets_y[r]);
		display.print(score);

		x = offsets_x[r] + ((300 - 11) / 2) - 0;
		y = offsets_y[r] + 300 - 5 - 12 - 40;
		display.drawBitmap(x, y, epd_bitmap_allArray[(3 - r + rot) % 4], 11, 12, GxEPD_BLACK);
	}
	display.nextPage();
}
