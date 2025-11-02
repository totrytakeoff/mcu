# OLED显示屏模块

## 📖 文档索引

### 📘 [OLED_DISPLAY_GUIDE.md](OLED_DISPLAY_GUIDE.md)
**完整使用指南** - 详细的硬件说明、API文档、最佳实践、常见问题

**适合：** 首次使用、深入学习、问题排查

**内容：**
- 🔌 硬件接线详解
- 🚀 快速开始教程
- 📊 性能评估分析
- 📖 完整API参考
- 💡 最佳实践建议
- ❓ 常见问题解答

---

### ⚡ [OLED_QUICK_REF.md](OLED_QUICK_REF.md)
**快速参考手册** - 5分钟快速上手、常用API速查

**适合：** 快速查阅、日常使用、临时查找

**内容：**
- ⚡ 5分钟快速上手
- 📋 常用API速查表
- 🎯 最佳实践一览
- 📊 性能指标速查
- 🐛 故障排查清单

---

## 🎯 快速开始

### 1. 添加库依赖
编辑 `platformio.ini`：
```ini
lib_deps = 
    olikraus/U8g2 @ ^2.35.9
```

### 2. 硬件连接
```
OLED屏幕 → STM32F103RCT6
VCC     → 3.3V
GND     → GND
SCL     → PB10 (I2C2_SCL)
SDA     → PB11 (I2C2_SDA)
```

### 3. 代码示例
```cpp
#include "oled_display.hpp"

OLEDDisplay oled;

int main(void) {
    // 系统初始化...
    oled.init();
    oled.showWelcome();
    HAL_Delay(2000);
    
    while(1) {
        oled.showDebugInfo("Running", 50, -12.5f, 2500);
        HAL_Delay(100);  // 10Hz刷新
    }
}
```

---

## 📂 文件结构

```
stm32_remote_car/
├── include/
│   └── oled_display.hpp           # OLED显示类头文件
├── src/
│   └── oled_display.cpp           # OLED显示类实现
├── examples/
│   └── oled_display_example.cpp   # 完整使用示例
└── docs/11_oled_display/
    ├── README.md                  # 本文档（索引）
    ├── OLED_DISPLAY_GUIDE.md      # 完整使用指南
    └── OLED_QUICK_REF.md          # 快速参考手册
```

---

## 🔧 核心功能

### 基本显示
- ✅ 文本显示（6行，格式化支持）
- ✅ 图形绘制（线、矩形、圆、进度条）
- ✅ 自定义字体和布局

### 巡线车专用
- ✅ 调试信息显示（状态、速度、位置、传感器）
- ✅ PID参数显示
- ✅ 欢迎界面、校准界面

### 性能优化
- ✅ 低资源占用（~10KB Flash, ~1.5KB RAM）
- ✅ 可调刷新率（推荐10Hz）
- ✅ 批量更新支持

---

## 📊 技术规格

| 参数 | 值 | 说明 |
|------|---|------|
| **屏幕型号** | SSD1315 | 兼容SSD1306 |
| **分辨率** | 128×64 | 单色OLED |
| **接口** | I2C | 地址0x3C |
| **驱动库** | u8g2 | v2.35.9+ |
| **Flash占用** | ~10KB | 256KB中的3.9% |
| **RAM占用** | ~1.5KB | 48KB中的3.1% |
| **刷新时间** | 10-15ms | 100kHz I2C |

---

## 💡 使用建议

### 推荐刷新策略
```cpp
// 10Hz刷新（推荐）
static uint32_t last_update = 0;
if (HAL_GetTick() - last_update >= 100) {
    oled.showDebugInfo(...);
    last_update = HAL_GetTick();
}
```

### 与EEPROM共存
- OLED和EEPROM共用I2C2总线
- 地址不冲突（OLED: 0x3C, EEPROM: 0x52）
- 避免同时操作（通常不是问题）

---

## 🐛 常见问题

| 问题 | 解决方案 |
|------|----------|
| 屏幕无显示 | 检查接线、I2C地址、是否调用`show()` |
| 显示不完整 | 确保在绘制后调用`show()` |
| 刷新卡顿 | 降低刷新频率到10Hz |
| 与EEPROM冲突 | 不会冲突，地址不同 |

详细排查步骤请查看 [OLED_DISPLAY_GUIDE.md](OLED_DISPLAY_GUIDE.md#常见问题)

---

## 🔗 相关文档

- [I2C配置文档](../03_系统配置/I2C_CONFIG.md)（如果存在）
- [EEPROM使用指南](../06_eeprom/EEPROM_GUIDE.md)
- [调试系统指南](../04_问题排查/serial_debug/DEBUG_SYSTEM_GUIDE.md)

---

## 📝 更新记录

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| v1.0 | 2024-10-27 | 初始版本，支持SSD1315驱动 |
