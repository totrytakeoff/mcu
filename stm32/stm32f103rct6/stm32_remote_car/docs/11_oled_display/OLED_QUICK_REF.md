# OLED显示屏快速参考

## ⚡ 5分钟快速上手

### 1️⃣ 添加库依赖（platformio.ini）
```ini
lib_deps = 
    olikraus/U8g2 @ ^2.35.9
```

### 2️⃣ 接线
```
OLED → STM32
VCC  → 3.3V
GND  → GND
SCL  → PB10
SDA  → PB11
```

### 3️⃣ 基本代码
```cpp
#include "oled_display.hpp"

OLEDDisplay oled;

int main(void) {
    // 初始化
    oled.init();
    
    // 显示文本
    oled.clear();
    oled.printLine(0, "Hello");
    oled.printfLine(1, "Speed: %d", 50);
    oled.show();
    
    while(1) {
        // 10Hz刷新
        HAL_Delay(100);
        oled.showDebugInfo("Run", 50, -12.5f, 2500);
    }
}
```

---

## 📋 常用API速查

### 初始化与控制
```cpp
oled.init()              // 初始化
oled.clear()             // 清空缓冲区
oled.show()              // 刷新显示
oled.setPower(false)     // 关闭显示
oled.setContrast(128)    // 设置对比度(0-255)
```

### 文本显示
```cpp
oled.printLine(0, "Text")              // 行号(0-5)
oled.printfLine(0, "Val: %d", val)     // 格式化
oled.printAt(64, 32, "Center")         // 像素位置
```

### 图形绘制
```cpp
oled.drawLine(x0, y0, x1, y1)          // 画线
oled.drawRect(x, y, w, h)              // 矩形（空心）
oled.drawBox(x, y, w, h)               // 矩形（实心）
oled.drawCircle(x, y, r)               // 圆
oled.drawProgressBar(x, y, w, h, pct)  // 进度条
```

### 预设界面
```cpp
oled.showWelcome()                              // 欢迎界面
oled.showCalibration()                          // 校准界面
oled.showDebugInfo(state, speed, pos, sensor)   // 调试信息
oled.showPIDParams(kp, ki, kd)                  // PID参数
```

---

## 🎯 最佳实践一览

### ✅ 推荐做法
```cpp
// 批量更新（10Hz刷新）
static uint32_t last = 0;
if (HAL_GetTick() - last >= 100) {
    oled.clear();
    oled.printLine(0, "Line 1");
    oled.printLine(1, "Line 2");
    oled.show();  // 仅刷新1次
    last = HAL_GetTick();
}
```

### ❌ 避免
```cpp
// ❌ 频繁刷新（占用CPU）
while(1) {
    oled.show();
    HAL_Delay(1);
}

// ❌ 多次刷新（浪费时间）
oled.printLine(0, "A");
oled.show();  // 刷新1
oled.printLine(1, "B");
oled.show();  // 刷新2（多余）
```

---

## 📊 性能指标

| 项目 | 数值 | 说明 |
|------|------|------|
| Flash占用 | ~10KB | 256KB中的3.9% |
| RAM占用 | ~1.5KB | 48KB中的3.1% |
| 刷新时间 | 10-15ms | 100kHz I2C |
| 推荐刷新率 | 10Hz | 每100ms |

---

## 🐛 故障排查

| 问题 | 解决方案 |
|------|----------|
| 无显示 | 检查接线、I2C地址(0x3C)、是否调用show() |
| 显示不完整 | 确保调用show() |
| 卡顿 | 降低刷新频率到10Hz |
| 乱码 | 检查字符编码、使用英文和数字 |

---

## 🔍 I2C设备扫描
```cpp
// 扫描I2C总线上的所有设备
for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    if (HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 1, 100) == HAL_OK) {
        Debug_Printf("Found: 0x%02X\r\n", addr);
    }
}
// 预期输出：0x3C (OLED), 0x52 (EEPROM)
```

---

## 📐 屏幕坐标系统

```
(0,0)                    (127,0)
  ┌──────────────────────┐
  │                      │
  │                      │  128x64 像素
  │       (64,32)        │
  │                      │
  │                      │
  └──────────────────────┘
(0,63)                  (127,63)
```

**文本行高**：10像素（字体8px + 2px间距）
- 行0: y=10
- 行1: y=20
- 行2: y=30
- 行3: y=40
- 行4: y=50
- 行5: y=60

---

## 📦 文件清单
```
include/oled_display.hpp        # 头文件
src/oled_display.cpp            # 实现文件
examples/oled_display_example.cpp  # 示例代码
docs/11_oled_display/
  ├─ OLED_DISPLAY_GUIDE.md      # 完整指南
  └─ OLED_QUICK_REF.md          # 本文档
```

---

## 🔗 相关资源
- 完整指南: `docs/11_oled_display/OLED_DISPLAY_GUIDE.md`
- u8g2文档: https://github.com/olikraus/u8g2/wiki
