#include <HID.h>
#include <Mouse.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_GFX.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_ST77xx.h>
#include <Adafruit_ST7796S.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "map0.h"

/*��Ҫ������ʵ��һ�������壬�ܹ�ģ��ʼǱ������·��Ĵ�����������ʹ��*/

#define CS_PIN  4
#define TIRQ_PIN  3
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
#define TFT_CS         14
#define TFT_RST        15
#define TFT_DC         32

#elif defined(ESP8266)
#define TFT_CS         4
#define TFT_RST        16
#define TFT_DC         5

#else
// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS        10
#define TFT_RST        -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         7
#endif

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Serial.println("Serial initialized successfully");

	pinMode(D9, OUTPUT);
	digitalWrite(D9, HIGH);

	ts.begin();
	ts.setRotation(0);
	Serial.println("Touchscreen initialized successfully");

	tft.init(240, 320);
	Serial.println("TFT initialized successfully");
	tft.setRotation(0);
	tft.drawRGBBitmap(0, 0, map0, 320, 240);

	Mouse.begin();
	Serial.println("Mouse initialized successfully");
}

bool crazycilckmode = false;
u_int16_t delaytime = 100;

void loop() {
	if (ts.touched()) {
		TS_Point p = ts.getPoint();
		int16_t x = p.x;
		int16_t y = p.y;
		int16_t x1 = map(x, 1, 4095, 0, 1920);
		int16_t y1 = map(y, 1, 4095, 0, 1080);
		int16_t x1 = x1 - 960;
		int16_t y1 = -(y1 - 540);

		if (abs(-y) > 3072) {
			if (abs(x) < 1365) {
				if (!crazycilckmode) {
					Mouse.click();
				}
				else {
					if (delaytime > 20) {
						delaytime -= 20;
					}
				}
			}
			else if (abs(x) > 2730) {
				if (!crazycilckmode) {
					Mouse.click(MOUSE_RIGHT);
				}
				else {
					if (delaytime < 1000) {
						delaytime += 20;
					}
				}
			}
			else {
				crazycilckmode = !crazycilckmode;
				delaytime = 100;
			}
			delay(500);
		}
		else {
			Mouse.move(x1 / 100, y1 / 100);
			delay(20);
		}
	}

	if (crazycilckmode) {
		Mouse.click();
		delay(delaytime);
	}
}