# 基于PID的巡线系统

## 📖 文档索引

- **[LINE_FOLLOWER_PID_GUIDE.md](LINE_FOLLOWER_PID_GUIDE.md)** - 完整使用指南
- **[../../examples/line_follower_pid_example.cpp](../../examples/line_follower_pid_example.cpp)** - 完整示例代码

---

## 🚀 5分钟快速上手

### 1. 创建巡线控制器

```cpp
#include "line_follower_pid.hpp"

LineSensor sensor;
Motor motor_lf, motor_lr, motor_rf, motor_rr;

LineFollowerPID follower(sensor, motor_lf, motor_lr, motor_rf, motor_rr);
```

### 2. 配置参数

```cpp
follower.setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);
follower.setPID(0.06f, 0.0f, 1.0f);  // Kp, Ki, Kd
follower.setBaseSpeed(30);
follower.enableDebug(true);
```

### 3. 启动巡线

```cpp
follower.init();
follower.start();

while (1) {
    follower.update();
    HAL_Delay(20);
}
```

---

## 🎯 核心特性

✅ **基于通用PID控制器**
- 使用刚封装的`PIDController`类
- 完整的P+I+D控制
- 积分抗饱和 + 微分滤波

✅ **加权算法计算位置**
- 8个传感器加权平均
- 精确的线位置计算 (-1000 到 1000)
- 自动阈值判断

✅ **差速转向控制**
- PID输出直接转换为速度差
- 平滑的转向响应
- 左右轮独立控制

✅ **智能丢线处理**
- 自动检测丢线
- 保持转向方向搜线
- 降速处理

---

## 📋 API速查

| 方法 | 说明 | 示例 |
|------|------|------|
| `setPID(kp, ki, kd)` | 设置PID参数 | `setPID(0.06f, 0.0f, 1.0f)` |
| `setBaseSpeed(speed)` | 设置基础速度 | `setBaseSpeed(30)` |
| `setLineMode(mode)` | 设置线模式 | `setLineMode(WHITE_ON_BLACK)` |
| `setThreshold(th)` | 设置阈值 | `setThreshold(2000)` |
| `start()` | 启动巡线 | `start()` |
| `stop()` | 停止巡线 | `stop()` |
| `update()` | 更新控制 | 在主循环20ms调用一次 |
| `enableDebug(bool)` | 启用调试 | `enableDebug(true)` |

---

## ⚙️ 参数快速参考

### PID参数表

| 速度 | Kp | Ki | Kd | 说明 |
|------|----|----|----|----|
| 20-30 | 0.04-0.06 | 0.0 | 0.8-1.2 | 低速，平稳 |
| 30-50 | 0.06-0.08 | 0.0 | 1.2-1.8 | 中速，推荐 |
| 50-70 | 0.10-0.15 | 0.0 | 2.0-3.0 | 高速，激进 |

### 调节步骤

1. **设置基础速度**：从30开始
2. **只调P**：找到能跟线但有震荡的Kp
3. **加入D**：消除震荡，Kd ≈ Kp × 15~25
4. **可选I**：通常不需要，如需消除误差可加小的Ki

---

## 🔧 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| 震荡严重 | Kp太大 | 减小Kp或增大Kd |
| 反应慢 | Kp太小 | 增大Kp |
| 经常丢线 | 速度太快/未校准 | 降速或重新校准 |
| 不转向 | PID参数太小 | 增大Kp |

---

## 📊 算法原理

### 位置计算（加权算法）

```
position = Σ(sensor_value[i] × weight[i]) / Σ(sensor_value[i])

权重:
传感器:   0      1      2      3      4      5      6      7
权重:  -1000  -714  -429  -143  +143  +429  +714  +1000
```

### 差速控制

```
error = 0 - position
pid_output = PID.compute(0, position)

left_speed  = base_speed + pid_output
right_speed = base_speed - pid_output
```

**转向逻辑**：
- 线在左 → pid_output > 0 → 左快右慢 → 右转 ✓
- 线在右 → pid_output < 0 → 右快左慢 → 左转 ✓

---

## 📁 文件结构

```
include/
  └── line_follower_pid.hpp     # 头文件

src/
  └── line_follower_pid.cpp     # 实现

examples/
  └── line_follower_pid_example.cpp  # 完整示例

docs/10_line_follower_pid/
  ├── README.md                  # 本文件
  └── LINE_FOLLOWER_PID_GUIDE.md # 完整指南
```

---

## 🎓 使用流程

```cpp
// 1. 初始化硬件
LineSensor sensor;
Motor motor_lf, motor_lr, motor_rf, motor_rr;

// 2. 传感器校准（首次使用）
Button calib_button(GPIOD, GPIO_PIN_2);
sensor.autoCalibrate(calib_button);

// 3. 创建巡线控制器
LineFollowerPID follower(sensor, motor_lf, motor_lr, motor_rf, motor_rr);

// 4. 配置参数
follower.setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);
follower.setPID(0.06f, 0.0f, 1.0f);
follower.setBaseSpeed(30);
follower.setThreshold(2000);

// 5. 初始化并启动
follower.init();
follower.start();

// 6. 主循环
while (1) {
    follower.update();
    HAL_Delay(20);
}
```

---

## 🐛 调试输出

启用调试后输出格式：

```
Pos:-412.7 Err:412.7 PID:24.8 L:55 R:5 | S:500 2500 3500 3000 1000 500 500 500 | B:·███·····
 ↓          ↓        ↓      ↓   ↓       ↓                                        ↓
位置       误差    PID输出 左速 右速   传感器原始值                              二值化显示
```

---

## 📚 相关文档

- [PID控制器指南](../09_pid_controller/PID_CONTROLLER_GUIDE.md) - PID原理和调参
- [传感器校准指南](../07_sensor_calibration/CALIBRATION_GUIDE.md) - 如何校准传感器
- [示例代码](../../examples/line_follower_pid_example.cpp) - 完整工作示例

---

## ⚡ 快速诊断

```bash
# 1. 检查传感器
sensor.getRawData(data);  # 查看原始值

# 2. 检查阈值
follower.setThreshold(2000);  # 白色<阈值<黑色

# 3. 检查PID参数
follower.setPID(0.06f, 0.0f, 1.0f);  # 从推荐值开始

# 4. 检查速度
follower.setBaseSpeed(30);  # 从低速开始

# 5. 启用调试
follower.enableDebug(true);  # 查看实时数据
```

---

**开始你的巡线之旅吧！🏁**
