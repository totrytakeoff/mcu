# Button类使用指南

## 📋 简介

`Button`类是一个通用的按钮封装类，提供完整的按键检测功能，包括防抖、边沿检测、长按检测等。

---

## ✨ 功能特性

- ✅ **通用GPIO支持**：支持任意GPIO端口和引脚
- ✅ **硬件防抖**：50ms防抖时间（可配置）
- ✅ **边沿检测**：按下/释放事件检测
- ✅ **长按检测**：可配置长按时间阈值
- ✅ **模式可选**：上拉/下拉模式
- ✅ **易于复用**：面向对象设计

---

## 🚀 快速开始

### 1. 包含头文件

```cpp
#include "button.hpp"
```

### 2. 创建按钮对象

```cpp
// 创建PD2按钮（上拉模式，按下为低电平）
Button calibButton(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);
```

### 3. 初始化

```cpp
calibButton.init();
```

### 4. 检测按键

```cpp
while (1) {
    if (calibButton.isPressed()) {
        // 按钮被按下
    }
    HAL_Delay(10);
}
```

---

## 📖 API参考

### 构造函数

```cpp
Button(GPIO_TypeDef* port, uint16_t pin, 
       ButtonMode mode = ButtonMode::PULL_UP,
       uint32_t debounce_ms = 50);
```

**参数**：
- `port`: GPIO端口（GPIOA, GPIOB, GPIOC, GPIOD, GPIOE）
- `pin`: GPIO引脚（GPIO_PIN_0 ~ GPIO_PIN_15）
- `mode`: 按钮模式
  - `ButtonMode::PULL_UP`: 上拉模式（按下为低电平）
  - `ButtonMode::PULL_DOWN`: 下拉模式（按下为高电平）
- `debounce_ms`: 防抖时间（毫秒），默认50ms

**示例**：
```cpp
// 上拉模式
Button btn1(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);

// 下拉模式
Button btn2(GPIOA, GPIO_PIN_0, ButtonMode::PULL_DOWN);

// 自定义防抖时间100ms
Button btn3(GPIOC, GPIO_PIN_13, ButtonMode::PULL_UP, 100);
```

---

### init()

初始化按钮GPIO。

```cpp
void init();
```

**说明**：
- 自动使能GPIO端口时钟
- 配置引脚为输入模式
- 根据模式设置上拉/下拉

**示例**：
```cpp
Button btn(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);
btn.init();
```

---

### isPressed()

检测按钮按下事件（边沿触发）。

```cpp
bool isPressed();
```

**返回值**：
- `true`: 检测到按下事件（仅触发一次）
- `false`: 未检测到

**特点**：
- 带防抖处理
- 边沿触发（按下时返回true一次）
- 释放后可再次触发

**示例**：
```cpp
if (btn.isPressed()) {
    Debug_Printf("按钮被按下！\r\n");
    // 执行操作
}
```

---

### isReleased()

检测按钮释放事件（边沿触发）。

```cpp
bool isReleased();
```

**返回值**：
- `true`: 检测到释放事件
- `false`: 未检测到

**示例**：
```cpp
if (btn.isReleased()) {
    Debug_Printf("按钮被释放\r\n");
}
```

---

### isLongPressed()

检测长按事件。

```cpp
bool isLongPressed(uint32_t long_press_ms = 2000);
```

**参数**：
- `long_press_ms`: 长按时间阈值（毫秒），默认2000ms

**返回值**：
- `true`: 检测到长按
- `false`: 未检测到

**特点**：
- 长按期间持续返回true
- 可用于实现长按功能

**示例**：
```cpp
static bool long_press_handled = false;

if (btn.isLongPressed(2000) && !long_press_handled) {
    Debug_Printf("长按2秒触发！\r\n");
    long_press_handled = true;
}

if (btn.isReleased()) {
    long_press_handled = false;
}
```

---

### getPressedDuration()

获取按钮按下持续时间。

```cpp
uint32_t getPressedDuration() const;
```

**返回值**：
- 按下持续时间（毫秒）
- 未按下时返回0

**示例**：
```cpp
uint32_t duration = btn.getPressedDuration();
if (duration > 0) {
    Debug_Printf("按下时长: %lu ms\r\n", duration);
}
```

---

### read()

读取按钮逻辑状态（已处理上拉/下拉）。

```cpp
bool read() const;
```

**返回值**：
- `true`: 按钮按下
- `false`: 按钮未按下

**示例**：
```cpp
bool state = btn.read();
```

---

### readRaw()

读取按钮原始电平。

```cpp
GPIO_PinState readRaw() const;
```

**返回值**：
- `GPIO_PIN_SET`: 高电平
- `GPIO_PIN_RESET`: 低电平

---

### setDebounceTime()

设置防抖时间。

```cpp
void setDebounceTime(uint32_t debounce_ms);
```

**参数**：
- `debounce_ms`: 防抖时间（毫秒）

**示例**：
```cpp
btn.setDebounceTime(100);  // 设置为100ms
```

---

### reset()

重置按钮状态。

```cpp
void reset();
```

**说明**：清除触发标志，通常不需要手动调用。

---

## 💡 使用示例

### 示例1：基本按键检测

```cpp
#include "button.hpp"

Button btn(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);

void setup() {
    btn.init();
}

void loop() {
    if (btn.isPressed()) {
        Debug_Printf("按钮按下！\r\n");
    }
    HAL_Delay(10);
}
```

---

### 示例2：校准按钮（实际应用）

```cpp
Button calibButton(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);

int main() {
    // 初始化
    calibButton.init();
    
    LineSensor sensor;
    EEPROM eeprom;
    
    while (1) {
        // 检测校准按钮
        if (calibButton.isPressed()) {
            Debug_Printf("开始校准...\r\n");
            sensor.autoCalibrate();
            sensor.saveCalibration(eeprom);
        }
        
        HAL_Delay(10);
    }
}
```

---

### 示例3：多按钮管理

```cpp
// 创建多个按钮
Button btnStart(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);
Button btnStop(GPIOD, GPIO_PIN_3, ButtonMode::PULL_UP);
Button btnCalib(GPIOD, GPIO_PIN_4, ButtonMode::PULL_UP);

void setup() {
    btnStart.init();
    btnStop.init();
    btnCalib.init();
}

void loop() {
    if (btnStart.isPressed()) {
        Debug_Printf("启动系统\r\n");
        // 启动逻辑
    }
    
    if (btnStop.isPressed()) {
        Debug_Printf("停止系统\r\n");
        // 停止逻辑
    }
    
    if (btnCalib.isPressed()) {
        Debug_Printf("开始校准\r\n");
        // 校准逻辑
    }
    
    HAL_Delay(10);
}
```

---

### 示例4：短按与长按

```cpp
Button btn(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);

bool long_press_handled = false;

void loop() {
    // 短按
    if (btn.isPressed()) {
        Debug_Printf("短按：执行功能A\r\n");
        long_press_handled = false;
    }
    
    // 长按（3秒）
    if (btn.isLongPressed(3000) && !long_press_handled) {
        Debug_Printf("长按：执行功能B\r\n");
        long_press_handled = true;
    }
    
    // 释放
    if (btn.isReleased()) {
        long_press_handled = false;
    }
    
    HAL_Delay(100);
}
```

---

## 🔧 硬件连接

### 上拉模式（推荐）

```
     VCC
      |
     [R] 10kΩ (内部上拉)
      |
     PD2 ---- 按钮 ---- GND
```

**特点**：
- 未按下：高电平
- 按下时：低电平
- 使用：`ButtonMode::PULL_UP`

---

### 下拉模式

```
     PD2 ---- 按钮 ---- VCC
      |
     [R] 10kΩ (内部下拉)
      |
     GND
```

**特点**：
- 未按下：低电平
- 按下时：高电平
- 使用：`ButtonMode::PULL_DOWN`

---

## 📊 技术参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| 防抖时间 | 50ms | 可配置 |
| 长按时间 | 2000ms | 可配置 |
| GPIO模式 | 输入 | 带上拉/下拉 |
| 支持端口 | GPIOA~E | STM32F103 |

---

## 🎯 与旧代码对比

### 旧方式（散落在main.cpp）

```cpp
// 全局变量
static bool last_button_state = false;
static uint32_t last_change_time = 0;
static bool calibration_triggered = false;

// 防抖函数
bool isButtonPressed() {
    bool current_state = readButton();
    uint32_t current_time = HAL_GetTick();
    
    if (current_state != last_button_state) {
        last_change_time = current_time;
        last_button_state = current_state;
    }
    
    if ((current_time - last_change_time) > 50) {
        if (current_state && !calibration_triggered) {
            calibration_triggered = true;
            return true;
        } else if (!current_state) {
            calibration_triggered = false;
        }
    }
    
    return false;
}

// 初始化函数
void initCalibrationButton(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

// 使用
initCalibrationButton();
if (isButtonPressed()) {
    // ...
}
```

### 新方式（Button类）

```cpp
// 创建对象
Button calibButton(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP);

// 使用
calibButton.init();
if (calibButton.isPressed()) {
    // ...
}
```

**优点**：
- ✅ 代码更简洁（3行 vs 30+行）
- ✅ 易于复用（支持多按钮）
- ✅ 功能更强（长按、时长等）
- ✅ 更好的封装
- ✅ main.cpp更清晰

---

## ❓ 常见问题

### Q1: 为什么按钮没有响应？

**检查**：
1. GPIO引脚是否正确
2. 按钮模式是否匹配硬件
3. 是否调用了`init()`

**调试**：
```cpp
Debug_Printf("按钮状态: %d\r\n", btn.read());
```

---

### Q2: 如何增加防抖时间？

```cpp
// 方法1：构造时指定
Button btn(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 100);

// 方法2：运行时修改
btn.setDebounceTime(100);
```

---

### Q3: 如何实现双击？

```cpp
uint32_t last_press_time = 0;
const uint32_t DOUBLE_CLICK_INTERVAL = 300; // 300ms内双击有效

if (btn.isPressed()) {
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_press_time < DOUBLE_CLICK_INTERVAL) {
        Debug_Printf("双击检测到！\r\n");
        last_press_time = 0;
    } else {
        last_press_time = current_time;
    }
}
```

---

## 📚 相关文档

- **示例程序**：`examples/button_example.cpp`
- **实际应用**：`src/main.cpp` (校准按钮)

---

## ✅ 总结

Button类提供了：
- ✅ 完整的按键检测功能
- ✅ 简洁的API
- ✅ 易于复用
- ✅ 代码更清晰

**推荐使用Button类代替散落的按钮代码！** 🎉
