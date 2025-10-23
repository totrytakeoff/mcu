# 简易双传感器巡线系统使用指南

## 📋 概述

`SimpleLineFollower` 是一个基于**梯度分级控制**的简化巡线系统，仅使用**传感器0（最左）和传感器7（最右）**实现巡线功能。

### 核心特点

✅ **简单可靠** - 仅用两个传感器，降低硬件依赖  
✅ **直接驱动** - 不依赖 `DriveTrain`，直接控制4个电机  
✅ **梯度平滑** - 7级状态分级，过渡自然  
✅ **硬件容错** - 适合传感器精度不高的场景  
✅ **易于调参** - 参数少，逻辑清晰  

---

## 🎯 工作原理

### 1. 传感器归一化

使用校准数据将传感器原始值归一化为 **0-100%**：

```
归一化值 = (当前值 - 白色校准值) / (黑色校准值 - 白色校准值) × 100%
```

- **黑底白线模式**：白线上 ≈ 100%，黑色区域 ≈ 0%
- **白底黑线模式**：黑线上 ≈ 100%，白色区域 ≈ 0%

### 2. 状态判定（7级梯度）

以**黑底白线**为例：

| 状态 | 左传感器 | 右传感器 | 左轮速度 | 右轮速度 | 说明 |
|------|---------|---------|---------|---------|------|
| **居中直行** | ≥70% | ≥70% | base | base | 两侧都在白线上 |
| **微右偏** | ≥70% | 60-70% | base | base+1 | 右侧稍偏离 |
| **轻度右偏** | ≥70% | 40-60% | base | base+3 | 右侧中度偏离 |
| **中度右偏** | ≥70% | <40% | base | base+6 | 右侧大幅偏离 |
| **微左偏** | 60-70% | ≥70% | base+1 | base | 左侧稍偏离 |
| **轻度左偏** | 40-60% | ≥70% | base+3 | base | 左侧中度偏离 |
| **中度左偏** | <40% | ≥70% | base+6 | base | 左侧大幅偏离 |
| **急左转** | <15% | ≥40% | base | -base | 左侧极黑（直角） |
| **急右转** | ≥40% | <15% | -base | base | 右侧极黑（直角） |
| **丢线/路口** | <15% | <15% | 搜索策略 | 搜索策略 | 两侧都是黑 |

### 3. 电机控制策略

- **正常转向**：单侧加速，另一侧保持基础速度
- **急转弯**：单侧前进，另一侧倒车（原地转向）
- **丢线恢复**：根据上次转向方向继续搜索

---

## 🚀 快速开始

### 基本用法

```cpp
#include "simple_line_follower.hpp"
#include "line_sensor.hpp"
#include "motor.hpp"

// 创建对象
LineSensor sensor;
Motor motor_fl, motor_fr, motor_rl, motor_rr;

// 初始化电机
motor_fl.init(&htim1, TIM_CHANNEL_1);
motor_fr.init(&htim1, TIM_CHANNEL_2);
motor_rl.init(&htim1, TIM_CHANNEL_3);
motor_rr.init(&htim1, TIM_CHANNEL_4);

// 创建巡线控制器
SimpleLineFollower follower(sensor, motor_fl, motor_fr, motor_rl, motor_rr);

// 配置参数
follower.setLineMode(SimpleLineFollower::LineMode::WHITE_LINE_ON_BLACK);
follower.setBaseSpeed(20);  // 基础速度
follower.init();

// 主循环
while (1) {
    follower.update();
    HAL_Delay(20);
}
```

---

## ⚙️ 参数配置

### 1. 基础速度设置

```cpp
follower.setBaseSpeed(20);  // 范围: 0-100
```

**调参建议**：
- 初始值：15-20
- 传感器响应慢 → 降低速度
- 赛道宽且直 → 可提高速度

### 2. 速度梯度设置

```cpp
follower.setSpeedGradient(
    1,  // 轻微偏离时的速度增量
    3,  // 中度偏离时的速度增量
    6   // 大幅偏离时的速度增量
);
```

**调参建议**：
- 小车转向灵敏 → 减小增量（如 1, 2, 4）
- 小车转向迟钝 → 增大增量（如 2, 5, 10）
- 保持梯度比例约 1:3:6

### 3. 阈值设置

```cpp
follower.setThresholds(
    15.0f,  // 丢线判定阈值（两侧都低于此值）
    15.0f,  // 急转弯判定阈值（单侧低于此值）
    70.0f   // 在线判定阈值（单侧高于此值）
);
```

**调参建议**：
- 传感器对比度高 → 提高阈值（如 20%, 20%, 75%）
- 传感器对比度低 → 降低阈值（如 10%, 10%, 60%）

### 4. 巡线模式切换

```cpp
// 黑底白线（默认）
follower.setLineMode(SimpleLineFollower::LineMode::WHITE_LINE_ON_BLACK);

// 白底黑线
follower.setLineMode(SimpleLineFollower::LineMode::BLACK_LINE_ON_WHITE);
```

### 5. 调试输出

```cpp
follower.enableDebug(true);  // 启用调试信息

// 输出示例：
// [直行] L:85.3% R:82.1% | Base:20
// [轻右] L:78.4% R:55.6% | Base:20
```

---

## 🔧 调参流程

### Step 1: 传感器校准

```cpp
// 确保已进行传感器校准
sensor.autoCalibrate(button);
sensor.saveCalibration(eeprom);

// 验证校准值
uint16_t white[8], black[8];
sensor.getCalibrationValues(white, black);
// 检查 white[0], white[7], black[0], black[7]
```

### Step 2: 确定基础速度

```cpp
follower.setBaseSpeed(15);  // 从较低速度开始
follower.enableDebug(true);

// 观察：
// - 能否稳定跟线？ → 是：可提速 / 否：降速
// - 转向是否及时？ → 否：降速
```

### Step 3: 调整速度梯度

```cpp
// 观察调试输出，根据表现调整
if (转向过头 || 震荡) {
    follower.setSpeedGradient(1, 2, 4);  // 减小增量
}
if (转向不足 || 反应慢) {
    follower.setSpeedGradient(2, 5, 10);  // 增大增量
}
```

### Step 4: 微调阈值

```cpp
// 观察归一化值（L: R:）
if (经常误判丢线) {
    follower.setThresholds(10.0f, 10.0f, 70.0f);  // 降低丢线阈值
}
if (急转弯识别不准) {
    follower.setThresholds(15.0f, 20.0f, 70.0f);  // 提高急转阈值
}
```

---

## 📊 常见问题排查

### 问题1: 小车抖动/震荡

**可能原因**：
- 速度梯度增量过大
- 基础速度过快
- 传感器噪声

**解决方案**：
```cpp
follower.setBaseSpeed(12);           // 降低速度
follower.setSpeedGradient(1, 2, 3);  // 减小梯度
sensor.setFilterAlpha(0.3f);         // 增强滤波
```

### 问题2: 转向不及时/冲出赛道

**可能原因**：
- 速度梯度增量过小
- 更新频率过低

**解决方案**：
```cpp
follower.setSpeedGradient(2, 5, 10);  // 增大梯度
// 在主循环中提高更新频率：
follower.update();
HAL_Delay(15);  // 改为15ms或10ms
```

### 问题3: 直角转弯失败

**可能原因**：
- 急转弯阈值设置不当
- 基础速度过快

**解决方案**：
```cpp
follower.setThresholds(15.0f, 20.0f, 70.0f);  // 提高急转阈值
follower.setBaseSpeed(15);  // 降低速度
```

### 问题4: 频繁丢线

**可能原因**：
- 传感器校准不准确
- 丢线阈值过高
- 环境光变化

**解决方案**：
```cpp
// 重新校准传感器
sensor.autoCalibrate(button);
sensor.saveCalibration(eeprom);

// 降低丢线阈值
follower.setThresholds(10.0f, 15.0f, 70.0f);
```

### 问题5: 归一化值异常

**症状**：调试输出显示 L: 或 R: 的值始终很低/很高

**解决方案**：
```cpp
// 检查校准数据
uint16_t white[8], black[8];
sensor.getCalibrationValues(white, black);
Debug_Printf("传感器0: W=%d B=%d\r\n", white[0], black[0]);
Debug_Printf("传感器7: W=%d B=%d\r\n", white[7], black[7]);

// 确保: black > white + 200（有明显差值）
// 如果差值太小，重新校准
```

---

## 🎓 进阶技巧

### 1. 动态速度调整

```cpp
// 根据状态动态调整速度
if (follower.getStatus() == SimpleLineFollower::Status::STRAIGHT) {
    follower.setBaseSpeed(25);  // 直道加速
} else {
    follower.setBaseSpeed(18);  // 弯道减速
}
```

### 2. 丢线恢复策略优化

在 `simple_line_follower.cpp` 中修改 `handleLostLine()`：

```cpp
// 示例：路口检测（两侧都丢线超过一定时间）
static uint32_t lost_start_time = 0;
if (status_ == Status::LOST_LINE) {
    if (lost_start_time == 0) {
        lost_start_time = HAL_GetTick();
    } else if (HAL_GetTick() - lost_start_time > 500) {
        // 超过500ms丢线，可能是路口，执行特殊操作
        // 例如：停车、直行穿越、或按预设路径
    }
} else {
    lost_start_time = 0;
}
```

### 3. 多场景参数切换

```cpp
// 定义参数组
struct LineParams {
    int base_speed;
    int soft, mid, hard;
};

LineParams straight_params = {25, 1, 3, 6};   // 直道参数
LineParams curve_params = {15, 2, 5, 10};     // 弯道参数

// 根据场景切换
void applyParams(const LineParams& params) {
    follower.setBaseSpeed(params.base_speed);
    follower.setSpeedGradient(params.soft, params.mid, params.hard);
}
```

---

## 📈 与复杂算法对比

| 特性 | SimpleLineFollower | LineFollower (抛物线拟合) |
|------|-------------------|-------------------------|
| 传感器使用 | 2个（边缘） | 8个（全部） |
| 算法复杂度 | 低（规则引擎） | 高（PID + 拟合） |
| 精度 | 中等 | 高 |
| 硬件要求 | 低 | 高（需准确校准） |
| 调参难度 | 简单 | 较难 |
| 适用场景 | 直线多、硬件一般 | 弯道多、硬件精良 |

---

## 📝 完整示例

参考 `examples/simple_line_follower_example.cpp`，包含：

- ✅ 传感器校准流程
- ✅ EEPROM数据保存/加载
- ✅ 按钮触发校准
- ✅ 完整主循环结构
- ✅ 调试输出

---

## 🔗 相关文档

- [传感器校准指南](../07_sensor_calibration/CALIBRATION_GUIDE.md)
- [EEPROM使用快速参考](../06_eeprom/EEPROM_QUICK_REF.md)
- [复杂巡线系统对比](PARABOLIC_LINE_FOLLOWER_GUIDE.md)

---

**提示**：如果硬件误差较大，`SimpleLineFollower` 通常比复杂的 PID 系统更稳定可靠！
