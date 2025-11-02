# SSD1315 OLED显示屏使用指南

## 📋 目录
- [硬件说明](#硬件说明)
- [快速开始](#快速开始)
- [性能评估](#性能评估)
- [API参考](#api参考)
- [最佳实践](#最佳实践)
- [常见问题](#常见问题)

---

## 🔌 硬件说明

### 屏幕参数
- **型号**：0.96寸 OLED（SSD1315驱动）
- **分辨率**：128×64像素
- **接口**：4线I2C（VCC、GND、SCL、SDA）
- **颜色**：单色（白/蓝/黄）
- **I2C地址**：0x3C（默认）

### 接线方式
```
OLED屏幕    →    STM32F103RCT6
━━━━━━━━━━━━━━━━━━━━━━━━━━━━
VCC         →    3.3V
GND         →    GND
SCL         →    PB10 (I2C2_SCL)
SDA         →    PB11 (I2C2_SDA)
```

> **注意**：OLED屏幕与EEPROM共用I2C2总线，两者地址不冲突（EEPROM: 0x52, OLED: 0x3C）

---

## 🚀 快速开始

### 步骤1：安装u8g2库

在 `platformio.ini` 中添加：

```ini
lib_deps = 
    olikraus/U8g2 @ ^2.35.9
```

### 步骤2：初始化OLED

```cpp
#include "oled_display.hpp"

// 创建对象
OLEDDisplay oled;

// 初始化（在main函数中）
if (oled.init()) {
    // 初始化成功
    oled.showWelcome();
} else {
    // 初始化失败
}
```

### 步骤3：显示文本

```cpp
// 清空屏幕
oled.clear();

// 显示文本（行号0-5）
oled.printLine(0, "Hello World");
oled.printfLine(1, "Speed: %d", 50);

// 刷新显示
oled.show();
```

### 步骤4：集成到巡线车

```cpp
// 在main.cpp中添加全局对象
OLEDDisplay oled;

// 在main函数中初始化
oled.init();

// 在主循环中更新显示（每100ms）
void loop() {
    static uint32_t last_update = 0;
    
    if (HAL_GetTick() - last_update >= 100) {
        oled.showDebugInfo(
            "Running",           // 状态
            follower->getSpeed(), // 速度
            line_sensor.getPosition(), // 位置
            line_sensor.getRawValue(0) // 传感器值
        );
        last_update = HAL_GetTick();
    }
}
```

---

## 📊 性能评估

### 资源占用

| 项目 | 占用 | 可用 | 百分比 |
|------|------|------|--------|
| **Flash** | ~10KB | 256KB | **3.9%** |
| **RAM** | ~1.5KB | 48KB | **3.1%** |

### 时间开销（72MHz主频，100kHz I2C）

| 操作 | 耗时 | 说明 |
|------|------|------|
| `init()` | ~50ms | 仅初始化时执行一次 |
| `clear()` | <0.1ms | 清空缓冲区（RAM操作） |
| `printLine()` | <0.1ms | 写入缓冲区（RAM操作） |
| `show()` | **10-15ms** | 刷新屏幕（I2C传输） |

### 性能优化建议

1. **降低刷新频率**
   - 推荐：10Hz（每100ms刷新一次）
   - 最小：5Hz（每200ms）
   - 避免：>20Hz（会占用过多CPU）

2. **批量更新**
   ```cpp
   // ❌ 错误：多次刷新
   oled.clear();
   oled.printLine(0, "Line 1");
   oled.show();  // 第1次刷新
   oled.printLine(1, "Line 2");
   oled.show();  // 第2次刷新
   
   // ✅ 正确：批量更新
   oled.clear();
   oled.printLine(0, "Line 1");
   oled.printLine(1, "Line 2");
   oled.show();  // 仅刷新1次
   ```

3. **条件更新**
   ```cpp
   // 仅在数据变化时更新
   static int last_speed = -1;
   int current_speed = get_speed();
   
   if (current_speed != last_speed) {
       oled.clear();
       oled.printfLine(0, "Speed: %d", current_speed);
       oled.show();
       last_speed = current_speed;
   }
   ```

---

## 📖 API参考

### 基本操作

#### `bool init()`
初始化OLED显示屏
```cpp
if (!oled.init()) {
    Debug_Printf("OLED初始化失败\r\n");
}
```

#### `void clear()`
清空显示缓冲区（不刷新屏幕）
```cpp
oled.clear();  // 清空缓冲区
```

#### `void show()`
刷新显示（将缓冲区内容发送到屏幕）
```cpp
oled.show();  // 刷新屏幕
```

#### `void setPower(bool on)`
开启/关闭显示
```cpp
oled.setPower(false);  // 关闭显示（省电）
HAL_Delay(5000);
oled.setPower(true);   // 重新开启
```

### 文本显示

#### `void printLine(uint8_t line, const char* text)`
在指定行显示文本
```cpp
oled.printLine(0, "Hello");
oled.printLine(1, "World");
```

#### `void printfLine(uint8_t line, const char* format, ...)`
在指定行显示格式化文本
```cpp
oled.printfLine(0, "Voltage: %.2fV", voltage);
oled.printfLine(1, "Current: %dmA", current);
```

#### `void printAt(uint8_t x, uint8_t y, const char* text)`
在指定像素位置显示文本
```cpp
oled.printAt(64, 32, "Center");  // 屏幕中心
```

### 图形绘制

#### `void drawLine(x0, y0, x1, y1)`
画线
```cpp
oled.drawLine(0, 0, 127, 63);  // 对角线
```

#### `void drawRect(x, y, w, h)`
画矩形（空心）
```cpp
oled.drawRect(10, 10, 50, 30);
```

#### `void drawBox(x, y, w, h)`
画矩形（实心）
```cpp
oled.drawBox(10, 10, 50, 30);
```

#### `void drawCircle(x, y, r)`
画圆
```cpp
oled.drawCircle(64, 32, 20);  // 圆心(64,32), 半径20
```

#### `void drawProgressBar(x, y, w, h, percentage)`
画进度条
```cpp
oled.drawProgressBar(10, 50, 108, 10, 75);  // 75%进度
```

### 高级功能

#### `void showDebugInfo(state, speed, position, sensorValue)`
一站式显示巡线车调试信息
```cpp
oled.showDebugInfo("Running", 50, -12.5f, 2500);
```

#### `void showPIDParams(kp, ki, kd)`
显示PID参数
```cpp
oled.showPIDParams(1.5f, 0.5f, 0.2f);
```

#### `void showWelcome()`
显示欢迎界面
```cpp
oled.showWelcome();
HAL_Delay(2000);
```

#### `void showCalibration()`
显示校准界面
```cpp
oled.showCalibration();
```

---

## 💡 最佳实践

### 1. 在巡线车中集成

```cpp
// main.cpp
#include "oled_display.hpp"

OLEDDisplay oled;
uint32_t last_oled_update = 0;

int main(void) {
    // ... 系统初始化 ...
    
    oled.init();
    oled.showWelcome();
    HAL_Delay(1000);
    
    while (1) {
        // 主业务逻辑
        follower->followLine();
        
        // 10Hz 刷新显示
        if (HAL_GetTick() - last_oled_update >= 100) {
            updateOLED();
            last_oled_update = HAL_GetTick();
        }
    }
}

void updateOLED() {
    const char* state_str;
    switch(system_state) {
        case SystemState::CALIBRATING: state_str = "Calib"; break;
        case SystemState::RUNNING: state_str = "Run"; break;
        case SystemState::STOPPED: state_str = "Stop"; break;
        default: state_str = "Unknown";
    }
    
    oled.showDebugInfo(
        state_str,
        follower->getSpeed(),
        line_sensor.getPosition(),
        line_sensor.getRawValue(0)
    );
}
```

### 2. 自定义显示布局

```cpp
void showCustomInfo() {
    oled.clear();
    
    // 标题
    oled.printLine(0, "Line Follower");
    
    // 状态信息
    oled.printfLine(1, "Speed: %d", speed);
    oled.printfLine(2, "Pos: %.1f", position);
    
    // 图形化速度表示
    oled.drawRect(0, 40, 128, 15);
    int bar_width = speed * 126 / 100;
    oled.drawBox(1, 41, bar_width, 13);
    
    // 传感器状态指示
    for (int i = 0; i < 8; i++) {
        if (sensor_value[i] > threshold) {
            oled.drawBox(i * 16, 58, 14, 6);
        }
    }
    
    oled.show();
}
```

### 3. 动态进度显示

```cpp
void showCalibrationProgress(uint8_t progress) {
    oled.clear();
    oled.printLine(0, "Calibrating...");
    oled.drawProgressBar(10, 30, 108, 15, progress);
    oled.printfLine(3, "Progress: %d%%", progress);
    oled.show();
}
```

### 4. 省电模式

```cpp
void enterSleepMode() {
    oled.setPower(false);  // 关闭显示
    // 其他省电操作...
}

void exitSleepMode() {
    oled.setPower(true);   // 开启显示
    oled.clear();
    oled.printLine(0, "Waking up...");
    oled.show();
}
```

---

## ❓ 常见问题

### Q1: 屏幕无显示？

**检查清单：**
1. ✅ 检查接线（VCC、GND、SCL、SDA）
2. ✅ 确认I2C地址（0x3C或0x3D，可用I2C扫描工具测试）
3. ✅ 检查I2C上拉电阻（通常OLED模块自带）
4. ✅ 确认调用了 `init()` 和 `show()`

**I2C地址测试代码：**
```cpp
// 扫描I2C设备
for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    if (HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 1, 100) == HAL_OK) {
        Debug_Printf("Found I2C device at 0x%02X\r\n", addr);
    }
}
```

### Q2: 显示内容不完整？

**原因：** 忘记调用 `show()`

```cpp
// ❌ 错误
oled.clear();
oled.printLine(0, "Hello");
// 缺少 oled.show()

// ✅ 正确
oled.clear();
oled.printLine(0, "Hello");
oled.show();  // 必须调用
```

### Q3: 屏幕刷新卡顿？

**原因：** 刷新频率过高

```cpp
// ❌ 错误：每次循环都刷新（100Hz+）
while(1) {
    oled.showDebugInfo(...);  // 内部调用show()
    HAL_Delay(1);
}

// ✅ 正确：10Hz刷新
while(1) {
    static uint32_t last = 0;
    if (HAL_GetTick() - last >= 100) {
        oled.showDebugInfo(...);
        last = HAL_GetTick();
    }
}
```

### Q4: 与EEPROM冲突？

**回答：** 不冲突，它们共用I2C总线但地址不同
- EEPROM: 0x52
- OLED: 0x3C

**注意事项：**
- I2C总线是半双工，同一时刻只能有一个设备通信
- 避免同时读写EEPROM和刷新OLED
- 建议在不同时间片操作（通常不是问题）

### Q5: 如何显示中文？

**回答：** u8g2库支持中文，但需要额外配置：

1. 使用中文字体（会增加Flash占用）
```cpp
u8g2_SetFont(u8g2_, u8g2_font_wqy12_t_chinese1);
```

2. 仅显示英文和数字可节省空间
```cpp
u8g2_SetFont(u8g2_, u8g2_font_6x10_tf);  // 默认字体
```

### Q6: 如何调整对比度？

```cpp
oled.setContrast(128);  // 0-255，默认128
```

---

## 📚 扩展阅读

- [u8g2官方文档](https://github.com/olikraus/u8g2/wiki)
- [SSD1306/SSD1315数据手册](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [STM32 HAL I2C指南](https://www.st.com/resource/en/user_manual/dm00105879.pdf)

---

## 📝 更新日志

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| v1.0 | 2024-10-27 | 初始版本，支持基本显示功能 |
