#include <HID.h>
#include <Mouse.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

#define TFT_CS    10
#define TFT_DC     7
#define TFT_RST   -1
#define TFT_BL     9

#define SD_CS_PIN      5
#define TOUCH_CS_PIN   4
#define TOUCH_IRQ_PIN  3

#define TS_MINX 200
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3800

#define COLOR_LBLUE  0x7DFF
#define COLOR_DBLUE  0x001F
#define COLOR_BG     ST77XX_BLACK
// 新增：高亮时的颜色 (比如明亮的蓝色)
#define COLOR_ACTIVE ST77XX_BLUE

XPT2046_Touchscreen ts(TOUCH_CS_PIN, TOUCH_IRQ_PIN);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

bool isDragLocked = false;
enum ClickMode { MODE_NORMAL, MODE_1CPS, MODE_5CPS, MODE_10CPS };
ClickMode currentClickMode = MODE_NORMAL;
bool isAutoClicking = false;
unsigned long lastClickTime = 0;
int clickInterval = 0;
int activeClickButton = 0; // 0=无, 1=左键, 2=右键
#define MOUSE_SPEED 15

void drawArrow(int x1, int y1, int x2, int y2, uint16_t color) {
	tft.drawLine(x1, y1, x2, y2, color);
	float arrow_len = 14.0;
	float arrow_width = 6.0;
	float angle = atan2(y2 - y1, x2 - x1);
	int x3 = x2 - arrow_len * cos(angle) + arrow_width * sin(angle);
	int y3 = y2 - arrow_len * sin(angle) - arrow_width * cos(angle);
	int x4 = x2 - arrow_len * cos(angle) - arrow_width * sin(angle);
	int y4 = y2 - arrow_len * sin(angle) + arrow_width * cos(angle);
	tft.fillTriangle(x2, y2, x3, y3, x4, y4, color);
}

void drawCustomUI() {
	tft.fillScreen(COLOR_BG);

	tft.drawFastVLine(80, 0, 240, COLOR_LBLUE);
	tft.drawFastVLine(160, 0, 240, COLOR_LBLUE);
	tft.drawFastHLine(0, 80, 240, COLOR_LBLUE);
	tft.drawFastHLine(0, 160, 240, COLOR_LBLUE);

	// 中心圆环
	tft.drawCircle(120, 120, 25, COLOR_LBLUE);
	tft.drawCircle(120, 120, 20, COLOR_LBLUE);

	// 八个箭头
	drawArrow(120, 90, 120, 40, COLOR_LBLUE);
	drawArrow(120, 150, 120, 200, COLOR_LBLUE);
	drawArrow(90, 120, 40, 120, COLOR_LBLUE);
	drawArrow(150, 120, 200, 120, COLOR_LBLUE);
	drawArrow(100, 100, 50, 50, COLOR_LBLUE);
	drawArrow(140, 100, 190, 50, COLOR_LBLUE);
	drawArrow(100, 140, 50, 190, COLOR_LBLUE);
	drawArrow(140, 140, 190, 190, COLOR_LBLUE);

	tft.fillRect(0, 239, 240, 2, COLOR_DBLUE);
	tft.drawFastVLine(80, 240, 80, COLOR_DBLUE);
	tft.drawFastVLine(160, 240, 80, COLOR_DBLUE);

	tft.setTextColor(COLOR_DBLUE); tft.setTextSize(4);
	tft.setCursor(28, 266); tft.print("L");
	tft.setCursor(188, 266); tft.print("R");
	tft.drawTriangle(120, 260, 100, 300, 140, 300, COLOR_DBLUE);
}

void updateModeIndicator() {
	uint16_t color = COLOR_BG;
	switch (currentClickMode) {
	case MODE_NORMAL: color = COLOR_BG; break;
	case MODE_1CPS:   color = ST77XX_GREEN; break;
	case MODE_5CPS:   color = ST77XX_YELLOW; break;
	case MODE_10CPS:  color = ST77XX_RED; break;
	}
	tft.fillTriangle(120, 265, 105, 295, 135, 295, color);
}

void updateDragLockIndicator() {
	if (isDragLocked) tft.fillCircle(120, 120, 18, ST77XX_RED);
	else tft.fillCircle(120, 120, 18, COLOR_BG);
}

// ⚠️ 关键修改：按键高亮控制
// pressed = true (亮起/按下状态), pressed = false (熄灭/背景色)
void highlightButton(int btnIndex, bool pressed) {
	if (btnIndex == 1) return;
	int x = btnIndex * 80 + 2;
	int y = 242; int w = 76; int h = 76;

	uint16_t color = pressed ? COLOR_ACTIVE : COLOR_BG;

	tft.fillRect(x, y, w, h, color);

	tft.setTextColor(COLOR_DBLUE); tft.setTextSize(4);
	if (btnIndex == 0) { tft.setCursor(28, 266); tft.print("L"); }
	if (btnIndex == 2) { tft.setCursor(188, 266); tft.print("R"); }
}

void setup() {
	Serial.begin(115200);
	Mouse.begin();

	pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL, HIGH);
	pinMode(TFT_CS, OUTPUT); digitalWrite(TFT_CS, HIGH);
	pinMode(SD_CS_PIN, OUTPUT); digitalWrite(SD_CS_PIN, HIGH);

	tft.init(240, 320);
	tft.setRotation(2); // USB 朝上

	drawCustomUI();

	ts.begin();
	ts.setRotation(2);
}

void loop() {
	// 连点执行逻辑
	if (isAutoClicking && currentClickMode != MODE_NORMAL) {
		if (millis() - lastClickTime > clickInterval) {
			if (activeClickButton == 1) Mouse.click(MOUSE_LEFT);
			else if (activeClickButton == 2) Mouse.click(MOUSE_RIGHT);
			lastClickTime = millis();
		}
	}

	if (ts.touched()) {
		TS_Point p = ts.getPoint();

		// 坐标映射 (已修正镜像)
		int16_t x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
		int16_t y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

		x = constrain(x, 0, 240);
		y = constrain(y, 0, 320);

		// === A. 上方九宫格 (Y < 240) ===
		if (y < 240) {
			int col = x / 80;
			int row = y / 80;

			if (col == 1 && row == 1) { // 拖拽锁
				tft.fillCircle(120, 120, 18, 0x528A);
				while (ts.touched()) delay(10);
				if (!isDragLocked) { Mouse.press(MOUSE_LEFT); isDragLocked = true; }
				else { Mouse.release(MOUSE_LEFT); isDragLocked = false; }
				updateDragLockIndicator();
			}
			else {
				int moveX = 0, moveY = 0;
				if (col == 0) moveX = -MOUSE_SPEED;
				if (col == 2) moveX = MOUSE_SPEED;
				if (row == 0) moveY = -MOUSE_SPEED;
				if (row == 2) moveY = MOUSE_SPEED;
				Mouse.move(moveX, moveY);
				delay(20);
			}
		}

		// === B. 下方功能区 (Y >= 240) ===
		else {
			int btnIndex = x / 80;
			if (btnIndex != 1) highlightButton(btnIndex, true);
			while (ts.touched()) delay(10);
			if (btnIndex == 0) { // 左键 L
				if (currentClickMode == MODE_NORMAL) {
					Mouse.click(MOUSE_LEFT);
					highlightButton(0, false); // 普通模式松手即灭
				}
				else {
					// 连点模式
					if (isAutoClicking && activeClickButton == 1) {
						// 停止连点 -> 灭灯
						isAutoClicking = false; activeClickButton = 0;
						highlightButton(0, false);
					}
					else {
						// 如果右键亮着，先灭右键
						if (activeClickButton == 2) highlightButton(2, false);

						// 开始连点 -> 保持亮灯 (不灭)
						isAutoClicking = true; activeClickButton = 1;
						highlightButton(0, true);
					}
				}
			}
			else if (btnIndex == 1) { // 模式 M
				// 切换模式时强制停止所有连点并灭灯
				if (activeClickButton == 1) highlightButton(0, false);
				if (activeClickButton == 2) highlightButton(2, false);
				isAutoClicking = false; activeClickButton = 0;

				// 切换速度
				switch (currentClickMode) {
				case MODE_NORMAL: currentClickMode = MODE_1CPS; clickInterval = 1000; break;
				case MODE_1CPS:   currentClickMode = MODE_5CPS; clickInterval = 200; break;
				case MODE_5CPS:   currentClickMode = MODE_10CPS; clickInterval = 100; break;
				case MODE_10CPS:  currentClickMode = MODE_NORMAL; clickInterval = 0; break;
				}
				updateModeIndicator();
			}
			else if (btnIndex == 2) { // 右键 R
				if (currentClickMode == MODE_NORMAL) {
					Mouse.click(MOUSE_RIGHT);
					highlightButton(2, false);
				}
				else {
					if (isAutoClicking && activeClickButton == 2) {
						isAutoClicking = false; activeClickButton = 0;
						highlightButton(2, false);
					}
					else {
						if (activeClickButton == 1) highlightButton(0, false);
						isAutoClicking = true; activeClickButton = 2;
						highlightButton(2, true);
					}
				}
			}
		}
	}
}