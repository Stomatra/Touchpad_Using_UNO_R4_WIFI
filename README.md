# Touchpad_Using_UNO_R4_WIFI

## 作业的功能

系统界面以USB 接口朝上为正方向，划分为两个核心功能区：

上半区 (240x240)：是九宫格光标控制区。
下半区 (240x80)：是三键功能操作区。

光标控制区（上半部九宫格）：此处用于控制鼠标指针的移动与拖拽操作。

八方向移动：
按住中心以外的周围 8 个格子，鼠标指针向对应方向（上、下、左、右及四个斜角）持续匀速移动，实现鼠标光标的精准定位。

拖拽锁定：
位置：九宫格正中心（圆环图标），点击一次激活，再次点击取消。在激活状态时，中心圆环变红，模拟“按下左键不松手”；在取消状态时，中心圆环恢复默认，模拟“松开左键”。方便进行文件拖拽或框选操作，无需长时间按压物理按键。

功能操作区（下半部三键）：此处包含鼠标点击及连点器控制。

左键：普通模式：点击触发一次鼠标左键单击。连点模式：点击激活左键连点（按键常亮），再次点击停止连点（按键熄灭）。

* **右键 (R)**：
  * **普通模式**：点击触发一次鼠标右键单击。
  * **连点模式**：点击**激活**右键连点（按键常亮），再次点击**停止**连点（按键熄灭）。
  * *注：左键连点与右键连点互斥，开启一方会自动关闭另一方。*
* **模式切换键 (M / Δ图标)**：
  * **位置**：下方正中间。
  * **操作**：点击循环切换连点速度模式。
  * **指示灯反馈**（位于按键上方的小横条）：
    1.  **无色/灭**：普通模式（无连点）。
    2.  **绿色**：低速连点 (1次/秒)。
    3.  **黄色**：中速连点 (5次/秒)。
    4.  **红色**：高速连点 (10次/秒)。

## 作业的实现

作业通过使用Arduino UNO R4 WIFI的触摸屏部分来进行实现。首先是初始化引脚，然后Serial部分用来监测屏幕的工作状况，XPT2046_Touchscreen,Mouse部分用来实现具体的功能。在void setup()中对它们进行初始化。

```c
Serial.begin(115200);
Serial.println("Serial initialized successfully");

pinMode(D9, OUTPUT);
digitalWrite(D9, HIGH);

ts.begin();
ts.setRotation(0);
Serial.println("Touchscreen initialized successfully");

Mouse.begin();
Serial.println("Mouse initialized successfully");
```

之后在loop部分首先使用ts对压力的地方进行检测：

```c
	TS_Point p = ts.getPoint();
	Serial.print("X = "); Serial.print(p.x);
	Serial.print("\tY = "); Serial.print(p.y);
	Serial.print("\tPressure = "); Serial.println(p.z);
```

然后把触控的位置转移成屏幕的分辨率，并具体化定位成鼠标光标要到达的正负坐标：

```c
	int16_t x = map(p.x, 1, 4095, 0, 1920); // Map to screen resolution
	int16_t y = map(p.y, 1, 4095, 0, 1080);// Map to screen resolution
	int16_t x1 = x - 960;
	int16_t y1 = -(y - 540); // Move relative to center of screen
	Serial.print("Mapped X = "); Serial.print(x1);
	Serial.print("\tMapped Y = "); Serial.println(y1);
```

最后对鼠标触控的功能进行实现：使用if循环来进行判断点的是中间位置还是周边位置，并由此来对鼠标行为进行判定。如果触摸的是中间位置，那么鼠标执行点击行为，如果触摸的是周围，那么鼠标执行移动行为。

```c
	if (abs(x1) < 100 && abs(y1) < 100) {
		Mouse.click();
		delay(500);
	}
	else {
		Mouse.move(x1 / 100, y1 / 100);
		delay(20);// Small delay to avoid overwhelming the host
	}
```

