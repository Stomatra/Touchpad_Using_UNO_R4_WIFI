# Touchpad_Using_UNO_R4_WIFI

这个项目作为的是备用项目，主要是为了模拟笔记本电脑上面数位板的功能

操作如下：

1. 在触摸开发板屏幕中间时，可以实现鼠标左键点击的功能
2. 在触摸开发板周围时，可以实现拖拽鼠标光标，也就是移动鼠标的功能。

## 作业的功能

作业通过连接Arduino UNO R4 WIFI的触摸屏实现电脑鼠标的精准操控。触摸屏幕中间的区域的时候，可以触发电脑左键点击，完成鼠标左键能完成的各种操作；触摸屏幕周围区域的时候，可以拖拽鼠标光标，实现光标在屏幕上的灵活移动，精准定位目标位置。整体操作直观便捷，无需额外操作设备，仅通过触摸屏的区域区分就能完成鼠标核心功能，适配简易触控交互需求。

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

