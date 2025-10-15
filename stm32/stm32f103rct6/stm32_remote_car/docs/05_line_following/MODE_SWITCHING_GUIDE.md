# 🔄 巡线模式切换指南

## 📋 概述

本文档介绍如何在 **白底黑线** 和 **黑底白线** 两种模式之间切换。

---

## 🎯 两种模式对比

### 模式1：白底黑线（BLACK_ON_WHITE）

```
场景：白色地面 + 黑色胶带引导线

地面: [白] [白] [黑] [黑] [黑] [白] [白] [白]
ADC:   350  380  3800 3850 3820 370  360  340
目标:              ↑  跟随黑线
```

**特点**：
- 适用于浅色地面（白纸、白色地板）
- 使用黑色电工胶带作为引导线
- 最常见的巡线场景

---

### 模式2：黑底白线（WHITE_ON_BLACK）

```
场景：黑色地面 + 白色胶带引导线

地面: [黑] [黑] [白] [白] [白] [黑] [黑] [黑]
ADC:  3850 3800  350  340  330 3820 3840 3860
目标:              ↑  跟随白线
```

**特点**：
- 适用于深色地面（黑色地板、黑色纸张）
- 使用白色胶带作为引导线
- 适合竞赛中的特殊赛道

---

## 💻 代码使用

### 方法1：在初始化时设置（推荐）

```cpp
LineSensor sensor;
sensor.init();

// 设置为白底黑线模式（默认，可省略）
sensor.setLineMode(LineSensor::LineMode::BLACK_ON_WHITE);

// 或设置为黑底白线模式
sensor.setLineMode(LineSensor::LineMode::WHITE_ON_BLACK);

// 其他代码完全相同
LineFollower follower(sensor, driveTrain);
follower.start();
```

---

### 方法2：运行时动态切换

```cpp
LineSensor sensor;
sensor.init();

// 初始模式：白底黑线
sensor.setLineMode(LineSensor::LineMode::BLACK_ON_WHITE);

// ... 运行一段时间 ...

// 切换到黑底白线
sensor.setLineMode(LineSensor::LineMode::WHITE_ON_BLACK);

// 立即生效，无需重新初始化
```

---

### 方法3：查询当前模式

```cpp
// 获取当前模式
LineSensor::LineMode currentMode = sensor.getLineMode();

if (currentMode == LineSensor::LineMode::BLACK_ON_WHITE) {
    printf("当前模式: 白底黑线\n");
} else {
    printf("当前模式: 黑底白线\n");
}
```

---

## 🎨 完整示例程序

### 白底黑线示例

```cpp
#include "line_sensor.hpp"
#include "line_follower.hpp"

LineSensor sensor;
DriveTrain driveTrain(...);
LineFollower follower(sensor, driveTrain);

int main(void)
{
    // 初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM3_Init();
    
    // 初始化传感器
    sensor.init();
    
    // ⭐ 设置为白底黑线模式（默认）
    sensor.setLineMode(LineSensor::LineMode::BLACK_ON_WHITE);
    
    // 校准
    sensor.calibrateWhite();  // 放在白色地面
    HAL_Delay(1000);
    sensor.calibrateBlack();  // 放在黑色线条
    sensor.finishCalibration();
    
    // 启动巡线
    follower.init();
    follower.setPID(0.1, 0.0, 1.2);
    follower.setSpeed(50);
    follower.start();
    
    while (1) {
        follower.update();
        HAL_Delay(20);
    }
}
```

---

### 黑底白线示例

```cpp
#include "line_sensor.hpp"
#include "line_follower.hpp"

LineSensor sensor;
DriveTrain driveTrain(...);
LineFollower follower(sensor, driveTrain);

int main(void)
{
    // 初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM3_Init();
    
    // 初始化传感器
    sensor.init();
    
    // ⭐ 设置为黑底白线模式
    sensor.setLineMode(LineSensor::LineMode::WHITE_ON_BLACK);
    
    // 校准（注意顺序可以调换）
    sensor.calibrateBlack();  // 放在黑色地面
    HAL_Delay(1000);
    sensor.calibrateWhite();  // 放在白色线条
    sensor.finishCalibration();
    
    // 启动巡线
    follower.init();
    follower.setPID(0.1, 0.0, 1.2);
    follower.setSpeed(50);
    follower.start();
    
    while (1) {
        follower.update();
        HAL_Delay(20);
    }
}
```

---

## 🔧 实现原理

### 核心差异

两种模式的唯一区别在于 **目标线的判断逻辑**：

```cpp
bool LineSensor::isBlack(uint8_t index) const
{
    if (lineMode_ == LineMode::WHITE_ON_BLACK) {
        // 黑底白线：低电压 = 白线 = 目标线
        return rawValues_[index] < threshold_;
    } else {
        // 白底黑线：高电压 = 黑线 = 目标线
        return rawValues_[index] > threshold_;
    }
}
```

### ADC值不变

无论哪种模式，ADC读数都是相同的：
- **白色表面** → 低电压 → ADC值低（~300-800）
- **黑色表面** → 高电压 → ADC值高（~3500-4000）

**只是判断逻辑反转了！**

---

## 📊 校准说明

### 白底黑线校准

```cpp
// 1. 放在白色地面上
sensor.calibrateWhite();  // 读取 ~400

// 2. 放在黑色线条上
sensor.calibrateBlack();  // 读取 ~3800

// 3. 计算阈值
sensor.finishCalibration();  // 阈值 = (400 + 3800) / 2 = 2100
```

### 黑底白线校准

```cpp
// 1. 放在黑色地面上
sensor.calibrateBlack();  // 读取 ~3800

// 2. 放在白色线条上
sensor.calibrateWhite();  // 读取 ~400

// 3. 计算阈值（结果相同）
sensor.finishCalibration();  // 阈值 = (3800 + 400) / 2 = 2100
```

**结论**：校准顺序不影响最终阈值！

---

## ✅ 检查清单

### 白底黑线模式

- [ ] 设置模式：`setLineMode(BLACK_ON_WHITE)`
- [ ] 地面：白色（A4纸、白色地板）
- [ ] 引导线：黑色胶带（2-3cm宽）
- [ ] 校准：白色 → 黑色
- [ ] 确认：传感器能区分白色和黑色

### 黑底白线模式

- [ ] 设置模式：`setLineMode(WHITE_ON_BLACK)`
- [ ] 地面：黑色（黑色纸张、黑色地板）
- [ ] 引导线：白色胶带（2-3cm宽）
- [ ] 校准：黑色 → 白色
- [ ] 确认：传感器能区分黑色和白色

---

## 🐛 常见问题

### Q1：切换模式后不工作？

**A**：确保重新校准！

```cpp
// 切换模式
sensor.setLineMode(LineSensor::LineMode::WHITE_ON_BLACK);

// ⚠️ 必须重新校准
sensor.calibrateBlack();
sensor.calibrateWhite();
sensor.finishCalibration();
```

---

### Q2：如何判断应该用哪种模式？

**A**：根据场地颜色选择：

```
地面颜色 > 线条颜色 → 用白底黑线
地面颜色 < 线条颜色 → 用黑底白线

例子：
- 白色地板 + 黑色胶带 → BLACK_ON_WHITE
- 黑色地板 + 白色胶带 → WHITE_ON_BLACK
- 灰色地板 + 黑色胶带 → BLACK_ON_WHITE
```

---

### Q3：可以在运行时切换模式吗？

**A**：可以，但建议停车后切换：

```cpp
// 停止巡线
follower.stop();

// 切换模式
sensor.setLineMode(LineSensor::LineMode::WHITE_ON_BLACK);

// 重新校准
sensor.calibrateBlack();
sensor.calibrateWhite();
sensor.finishCalibration();

// 重新启动
follower.start();
```

---

### Q4：两种模式的PID参数需要不同吗？

**A**：通常相同，但可以根据实际情况微调：

```cpp
// 白底黑线
follower.setPID(0.12, 0.0, 1.5);

// 黑底白线（通常相同）
follower.setPID(0.12, 0.0, 1.5);

// 如果线条对比度不同，可以微调
```

---

## 📚 相关文档

- [LINE_SENSOR_GUIDE.md](./LINE_SENSOR_GUIDE.md) - 完整使用指南
- [ALGORITHM_EXPLAINED.md](./ALGORITHM_EXPLAINED.md) - 算法原理详解
- [QUICK_START.md](./QUICK_START.md) - 快速开始

---

## 🎯 示例代码文件

- `examples/line_following_demo.cpp` - 白底黑线示例
- `examples/line_following_white_on_black_demo.cpp` - 黑底白线示例

---

**两种模式，一行代码切换！** ✅

```cpp
sensor.setLineMode(LineSensor::LineMode::WHITE_ON_BLACK);
```
