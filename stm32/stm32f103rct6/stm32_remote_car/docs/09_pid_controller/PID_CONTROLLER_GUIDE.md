# PID控制器完全指南

## 📚 目录

1. [概述](#概述)
2. [快速开始](#快速开始)
3. [PID原理](#pid原理)
4. [API参考](#api参考)
5. [使用示例](#使用示例)
6. [参数调节](#参数调节)
7. [高级功能](#高级功能)
8. [常见问题](#常见问题)

---

## 概述

### 什么是PID控制器？

PID（Proportional-Integral-Derivative）控制器是一种经典的反馈控制算法，广泛应用于工业控制系统。

### 特性

✅ **完整的PID算法**
- 比例（P）控制
- 积分（I）控制
- 微分（D）控制

✅ **高级功能**
- 积分抗饱和（Anti-Windup）
- 微分滤波（减少噪声）
- Derivative on Measurement（避免突变冲击）
- 输出限幅

✅ **灵活配置**
- 自动/手动模式
- 正向/反向控制
- 可调采样时间
- 运行时调参

✅ **易于使用**
- 简洁的API
- 详细的示例
- 完整的文档

---

## 快速开始

### 1. 基本使用

```cpp
#include "pid_controller.hpp"

// 创建PID控制器（Kp=1.0, Ki=0.1, Kd=0.05）
PIDController pid(1.0f, 0.1f, 0.05f);

// 设置输出限制
pid.setOutputLimits(-100.0f, 100.0f);

// 在控制循环中使用
float setpoint = 100.0f;  // 目标值
float measured = 0.0f;    // 当前测量值

while (1) {
    // 计算PID输出
    float output = pid.compute(setpoint, measured);
    
    // 应用输出到系统
    // system.apply(output);
    
    // 读取新的测量值
    // measured = sensor.read();
    
    HAL_Delay(20);  // 20ms控制周期
}
```

### 2. 巡线控制

```cpp
// 创建巡线PID控制器
PIDController line_pid(0.06f, 0.0f, 1.0f);
line_pid.setOutputLimits(-60.0f, 60.0f);

float line_position = sensor.getPosition();  // -1000 to 1000
float steering = line_pid.compute(0.0f, line_position);

// 差速控制
float left_speed = base_speed + steering;
float right_speed = base_speed - steering;
```

### 3. 电机速度控制

```cpp
// 速度PID控制器
PIDController speed_pid(0.5f, 0.2f, 0.01f);
speed_pid.setOutputLimits(-100.0f, 100.0f);

float target_speed = 100.0f;  // RPM
float current_speed = encoder.getSpeed();

float pwm = speed_pid.compute(target_speed, current_speed);
motor.setPWM(pwm);
```

---

## PID原理

### 控制框图

```
         ┌─────────┐
Setpoint │         │  Output   ┌────────┐  Measured
  ───────►   PID   ├──────────►│ System ├──────┬────►
         │         │           └────────┘      │
         └────▲────┘                           │
              │        Feedback                │
              └────────────────────────────────┘
```

### PID公式

**标准形式：**
```
output(t) = Kp × e(t) + Ki × ∫e(t)dt + Kd × de(t)/dt
```

其中：
- `e(t)` = 误差 = setpoint - measured
- `Kp` = 比例系数
- `Ki` = 积分系数
- `Kd` = 微分系数

**离散形式（代码实现）：**
```cpp
error = setpoint - input;
integral += Ki * error * dt;
derivative = Kd * (error - last_error) / dt;
output = Kp * error + integral + derivative;
```

### 各项作用

#### 比例项（P）
```
P = Kp × error
```
- **作用**：根据当前误差产生控制输出
- **特点**：响应快，但会产生稳态误差
- **调节**：Kp越大，响应越快，但容易震荡

#### 积分项（I）
```
I = Ki × ∫error dt
```
- **作用**：消除稳态误差
- **特点**：累积历史误差
- **问题**：容易产生积分饱和

#### 微分项（D）
```
D = Kd × d(error)/dt
```
- **作用**：预测误差趋势，抑制震荡
- **特点**：对噪声敏感
- **优化**：使用微分滤波

---

## API参考

### 构造函数

```cpp
PIDController(float kp = 0.0f, float ki = 0.0f, float kd = 0.0f);
```

**参数：**
- `kp` - 比例系数
- `ki` - 积分系数
- `kd` - 微分系数

**示例：**
```cpp
PIDController pid(1.0f, 0.1f, 0.05f);
```

### 核心方法

#### compute() - 计算PID输出

```cpp
float compute(float setpoint, float input);
float compute(float setpoint, float input, float dt);
```

**参数：**
- `setpoint` - 目标值
- `input` - 当前测量值
- `dt` - 时间间隔（秒），可选

**返回：**
- PID控制输出

**示例：**
```cpp
float output = pid.compute(100.0f, 75.0f);  // 自动采样时间
float output = pid.compute(100.0f, 75.0f, 0.02f);  // 指定dt=20ms
```

### 配置方法

#### setTunings() - 设置PID参数

```cpp
void setTunings(float kp, float ki, float kd);
```

**示例：**
```cpp
pid.setTunings(1.0f, 0.1f, 0.05f);
```

#### setOutputLimits() - 设置输出限制

```cpp
void setOutputLimits(float min, float max);
```

**示例：**
```cpp
pid.setOutputLimits(-100.0f, 100.0f);
```

#### setSampleTime() - 设置采样时间

```cpp
void setSampleTime(float sample_time_sec);
```

**示例：**
```cpp
pid.setSampleTime(0.02f);  // 20ms
pid.setSampleTime(0.001f); // 1ms
```

#### setMode() - 设置控制模式

```cpp
void setMode(Mode mode);
```

**模式：**
- `Mode::AUTOMATIC` - 自动模式（正常PID计算）
- `Mode::MANUAL` - 手动模式（不计算PID）

**示例：**
```cpp
pid.setMode(PIDController::Mode::AUTOMATIC);
pid.setMode(PIDController::Mode::MANUAL);
```

#### setDirection() - 设置控制方向

```cpp
void setDirection(Direction direction);
```

**方向：**
- `Direction::DIRECT` - 正向控制（误差为正时输出为正）
- `Direction::REVERSE` - 反向控制（误差为正时输出为负）

**示例：**
```cpp
pid.setDirection(PIDController::Direction::DIRECT);
pid.setDirection(PIDController::Direction::REVERSE);  // 用于冷却等场景
```

### 高级功能

#### setAntiWindup() - 积分抗饱和

```cpp
void setAntiWindup(bool enable);
```

**作用：** 防止积分项过度累积

**示例：**
```cpp
pid.setAntiWindup(true);  // 启用（默认）
```

#### setDerivativeFilter() - 微分滤波

```cpp
void setDerivativeFilter(float alpha);
```

**参数：**
- `alpha` - 滤波系数 (0.0-1.0)
  - 0.0 = 无滤波
  - 越大滤波越强

**示例：**
```cpp
pid.setDerivativeFilter(0.2f);  // 轻度滤波
pid.setDerivativeFilter(0.5f);  // 强滤波
```

#### reset() - 重置控制器

```cpp
void reset();
```

**作用：** 清空所有内部状态

**示例：**
```cpp
pid.reset();  // 在切换目标或系统重启时使用
```

### 状态查询

```cpp
float getError() const;           // 获取当前误差
float getProportional() const;    // 获取比例项
float getIntegral() const;        // 获取积分项
float getDerivative() const;      // 获取微分项
float getOutput() const;          // 获取输出值

float getKp() const;              // 获取Kp
float getKi() const;              // 获取Ki
float getKd() const;              // 获取Kd

bool isAutomatic() const;         // 是否自动模式
```

---

## 使用示例

### 示例1：温度控制

```cpp
PIDController temp_pid(5.0f, 0.5f, 1.0f);
temp_pid.setOutputLimits(0.0f, 100.0f);  // 加热功率 0-100%

float target_temp = 50.0f;
float current_temp = thermometer.read();

float power = temp_pid.compute(target_temp, current_temp);
heater.setPWM(power);
```

### 示例2：位置控制（串级PID）

```cpp
// 外环：位置PID
PIDController position_pid(2.0f, 0.0f, 0.5f);
position_pid.setOutputLimits(-100.0f, 100.0f);

// 内环：速度PID
PIDController speed_pid(0.5f, 0.2f, 0.01f);
speed_pid.setOutputLimits(-100.0f, 100.0f);

// 位置环输出目标速度
float target_speed = position_pid.compute(target_pos, current_pos);

// 速度环输出PWM
float pwm = speed_pid.compute(target_speed, current_speed);
```

### 示例3：云台稳定

```cpp
// 角度PID控制器
PIDController angle_pid(3.0f, 0.0f, 0.8f);
angle_pid.setOutputLimits(-90.0f, 90.0f);

// 启用微分滤波（陀螺仪有噪声）
angle_pid.setDerivativeFilter(0.3f);

float target_angle = 0.0f;  // 水平位置
float current_angle = gyro.getAngle();

float servo_angle = angle_pid.compute(target_angle, current_angle);
servo.setAngle(servo_angle);
```

### 示例4：平衡小车

```cpp
// 平衡角度PID
PIDController balance_pid(40.0f, 0.0f, 2.0f);
balance_pid.setOutputLimits(-100.0f, 100.0f);

// 速度PID（位置环）
PIDController velocity_pid(0.5f, 0.0f, 0.0f);
velocity_pid.setOutputLimits(-10.0f, 10.0f);

// 速度环输出调整平衡角度
float angle_adjust = velocity_pid.compute(target_speed, current_speed);

// 平衡环控制电机
float motor_output = balance_pid.compute(angle_adjust, tilt_angle);
```

---

## 参数调节

### 调节步骤

#### 第一步：只调P参数

```cpp
pid.setTunings(1.0f, 0.0f, 0.0f);  // 只有P
```

**目标**：让系统能够响应，允许有震荡

**现象与调整**：
| 现象 | 原因 | 调整 |
|------|------|------|
| 反应慢/无反应 | Kp太小 | 增大Kp |
| 剧烈震荡 | Kp太大 | 减小Kp |
| 有稳态误差 | 正常 | 进入下一步 |

#### 第二步：加入D参数

```cpp
pid.setTunings(1.0f, 0.0f, 0.5f);  // P+D
```

**目标**：消除震荡，平滑响应

**经验值**：`Kd = Kp × (10~25)`

**现象与调整**：
| 现象 | 原因 | 调整 |
|------|------|------|
| 仍然震荡 | Kd太小 | 增大Kd |
| 响应变慢 | Kd太大 | 减小Kd |
| 平稳但有误差 | 正常 | 进入下一步 |

#### 第三步：加入I参数（可选）

```cpp
pid.setTunings(1.0f, 0.05f, 0.5f);  // P+I+D
```

**目标**：消除稳态误差

**注意**：
- ⚠️ Ki要从小开始（0.01-0.1）
- ⚠️ 很多场景不需要I项
- ⚠️ 启用积分抗饱和

**现象与调整**：
| 现象 | 原因 | 调整 |
|------|------|------|
| 震荡加剧 | Ki太大 | 减小Ki |
| 响应变慢 | Ki太大 | 减小Ki |
| 超调严重 | Ki太大 | 减小Ki |

### 快速调参表

#### 巡线控制

| 速度 | Kp | Ki | Kd | 输出限制 |
|------|----|----|----|----|
| 慢速 (20-30) | 0.04-0.06 | 0.0 | 0.8-1.2 | ±40 |
| 中速 (30-50) | 0.06-0.08 | 0.0 | 1.2-1.8 | ±60 |
| 高速 (50-70) | 0.10-0.15 | 0.0 | 2.0-3.0 | ±80 |

#### 电机速度控制

| 应用 | Kp | Ki | Kd | 输出限制 |
|------|----|----|----|----|
| 普通电机 | 0.5-1.0 | 0.1-0.3 | 0.01-0.05 | ±100 |
| 编码器电机 | 1.0-2.0 | 0.2-0.5 | 0.05-0.1 | ±100 |

#### 位置控制

| 应用 | Kp | Ki | Kd | 输出限制 |
|------|----|----|----|----|
| 舵机位置 | 1.0-3.0 | 0.0 | 0.2-0.5 | ±90° |
| 电机位置 | 2.0-5.0 | 0.0-0.1 | 0.5-1.0 | ±100 RPM |

#### 温度控制

| 应用 | Kp | Ki | Kd | 输出限制 |
|------|----|----|----|----|
| 加热器 | 3.0-10.0 | 0.3-1.0 | 0.5-2.0 | 0-100% |
| 空调 | 2.0-5.0 | 0.1-0.5 | 1.0-3.0 | ±100% |

### 调参工具

#### 实时监控

```cpp
Debug_Printf("Error: %.2f, P: %.2f, I: %.2f, D: %.2f, Out: %.2f\r\n",
             pid.getError(),
             pid.getProportional(),
             pid.getIntegral(),
             pid.getDerivative(),
             pid.getOutput());
```

#### 响应曲线分析

观察系统响应曲线特征：

```
1. 欠阻尼（震荡）- 减小Kp或增大Kd
   ┌─────┐
   │  /\/\/\___
   └─────────────

2. 过阻尼（缓慢）- 增大Kp
   ┌─────┐
   │    ___/‾‾‾‾
   └─────────────

3. 理想响应
   ┌─────┐
   │   _/‾‾‾‾‾‾
   └─────────────
```

---

## 高级功能

### 1. 积分抗饱和（Anti-Windup）

**问题**：当输出达到限制时，积分项继续累积，导致超调和响应迟缓

**解决**：Back-calculation方法

```cpp
pid.setAntiWindup(true);  // 启用（默认）
```

**工作原理**：
```cpp
// 计算未限幅输出
unclamped = P + I + D;

// 限幅
clamped = constrain(unclamped, min, max);

// 如果发生饱和，调整积分项
if (unclamped != clamped) {
    I = clamped - P - D;
}
```

### 2. Derivative on Measurement

**问题**：setpoint突变时，微分项会产生巨大冲击（derivative kick）

**解决**：对测量值微分而非误差微分

```cpp
// 传统方法（会产生冲击）
derivative = (error - last_error) / dt;

// 本实现（避免冲击）
derivative = -(input - last_input) / dt;
```

**效果对比**：
```
Setpoint突变: 0 → 100
传统方法: D项瞬间 = 100/dt (巨大冲击)
本实现:   D项平滑变化
```

### 3. 微分滤波

**问题**：传感器噪声导致微分项波动

**解决**：低通滤波器

```cpp
pid.setDerivativeFilter(0.3f);  // alpha = 0.3
```

**滤波公式**：
```cpp
filtered_d = alpha × new_d + (1 - alpha) × old_d
```

**选择alpha**：
- `0.0` - 无滤波（快速响应）
- `0.1-0.3` - 轻度滤波（推荐）
- `0.5-0.8` - 强滤波（高噪声场景）

### 4. 自动/手动切换

```cpp
// 手动模式（不计算PID）
pid.setMode(PIDController::Mode::MANUAL);

// 执行一些操作...

// 切换回自动（平滑切换）
pid.setMode(PIDController::Mode::AUTOMATIC);
```

**应用场景**：
- 系统初始化
- 紧急停止
- 手动调试

### 5. 正向/反向控制

```cpp
// 正向：加热器（误差为正增加输出）
pid.setDirection(PIDController::Direction::DIRECT);

// 反向：冷却器（误差为正减少输出）
pid.setDirection(PIDController::Direction::REVERSE);
```

---

## 常见问题

### Q1: 系统震荡严重怎么办？

**原因：**
- Kp太大
- Kd太小
- 采样时间太长

**解决：**
```cpp
// 方法1：减小Kp
pid.setTunings(pid.getKp() * 0.5f, pid.getKi(), pid.getKd());

// 方法2：增大Kd
pid.setTunings(pid.getKp(), pid.getKi(), pid.getKd() * 1.5f);

// 方法3：减小采样时间
pid.setSampleTime(0.01f);  // 从20ms改为10ms
```

### Q2: 有稳态误差怎么办？

**原因：**
- 没有积分项
- 积分项太小

**解决：**
```cpp
// 添加小的积分项
pid.setTunings(pid.getKp(), 0.01f, pid.getKd());

// 确保启用抗饱和
pid.setAntiWindup(true);
```

### Q3: 响应太慢怎么办？

**原因：**
- Kp太小
- Kd太大
- 输出限制太小

**解决：**
```cpp
// 增大Kp
pid.setTunings(pid.getKp() * 1.5f, pid.getKi(), pid.getKd());

// 放宽输出限制
pid.setOutputLimits(-100.0f, 100.0f);
```

### Q4: 超调严重怎么办？

**原因：**
- Ki太大
- Kp太大

**解决：**
```cpp
// 减小Ki
pid.setTunings(pid.getKp(), pid.getKi() * 0.5f, pid.getKd());

// 或暂时关闭I
pid.setTunings(pid.getKp(), 0.0f, pid.getKd());
```

### Q5: 微分项噪声大怎么办？

**解决：**
```cpp
// 启用微分滤波
pid.setDerivativeFilter(0.3f);

// 或减小Kd
pid.setTunings(pid.getKp(), pid.getKi(), pid.getKd() * 0.5f);
```

### Q6: 如何处理积分饱和？

**确保启用抗饱和：**
```cpp
pid.setAntiWindup(true);
pid.setOutputLimits(-100.0f, 100.0f);
```

**或重置积分项：**
```cpp
// 在系统停止或setpoint改变时
pid.reset();
```

### Q7: 如何实现串级PID？

```cpp
// 外环（慢）
PIDController outer(2.0f, 0.0f, 0.5f);
outer.setOutputLimits(-100.0f, 100.0f);
outer.setSampleTime(0.02f);

// 内环（快）
PIDController inner(0.5f, 0.2f, 0.01f);
inner.setOutputLimits(-100.0f, 100.0f);
inner.setSampleTime(0.002f);

// 计算
float outer_output = outer.compute(target, measured_outer);
float inner_output = inner.compute(outer_output, measured_inner);
```

### Q8: 如何保存/加载PID参数？

```cpp
// 使用EEPROM保存
struct PIDParams {
    float kp, ki, kd;
};

PIDParams params = {pid.getKp(), pid.getKi(), pid.getKd()};
eeprom.writeStructCRC(0x00, params);

// 加载
if (eeprom.readStructCRC(0x00, params)) {
    pid.setTunings(params.kp, params.ki, params.kd);
}
```

---

## 性能优化

### 1. 减少计算量

```cpp
// 设置合适的采样时间（不要过快）
pid.setSampleTime(0.02f);  // 20ms通常够用

// 简化PID（如果不需要I项）
pid.setTunings(kp, 0.0f, kd);  // 省略积分计算
```

### 2. 避免频繁调参

```cpp
// 不好：每次都调参
for (...) {
    pid.setTunings(new_kp, new_ki, new_kd);  // 慢
}

// 好：只在需要时调参
if (needs_retune) {
    pid.setTunings(new_kp, new_ki, new_kd);
}
```

### 3. 使用合适的数据类型

```cpp
// 如果精度要求不高，可以考虑整数PID
// 但本实现使用float以保证通用性和精度
```

---

## 调试技巧

### 1. 启用详细日志

```cpp
void printPIDInfo(PIDController& pid) {
    Debug_Printf("PID Info:\r\n");
    Debug_Printf("  Kp=%.3f, Ki=%.3f, Kd=%.3f\r\n",
                 pid.getKp(), pid.getKi(), pid.getKd());
    Debug_Printf("  Error: %.2f\r\n", pid.getError());
    Debug_Printf("  P: %.2f, I: %.2f, D: %.2f\r\n",
                 pid.getProportional(),
                 pid.getIntegral(),
                 pid.getDerivative());
    Debug_Printf("  Output: %.2f\r\n", pid.getOutput());
}
```

### 2. 绘制响应曲线

```cpp
// 通过串口输出数据，用Python绘图
printf("%.3f,%.3f,%.3f\r\n", 
       HAL_GetTick()/1000.0f,  // 时间
       setpoint,                // 目标
       measured);               // 实际
```

### 3. 单步调试

```cpp
// 逐步测试各项
pid.setTunings(kp, 0.0f, 0.0f);  // 只测P
Debug_Printf("P only: %.2f\r\n", pid.compute(sp, mv));

pid.setTunings(kp, 0.0f, kd);    // 测P+D
Debug_Printf("P+D: %.2f\r\n", pid.compute(sp, mv));

pid.setTunings(kp, ki, kd);      // 测P+I+D
Debug_Printf("P+I+D: %.2f\r\n", pid.compute(sp, mv));
```

---

## 相关文档

- [PID_QUICK_REF.md](PID_QUICK_REF.md) - 快速参考
- [examples/pid_controller_example.cpp](../../examples/pid_controller_example.cpp) - 完整示例
- [EEPROM_GUIDE.md](../06_eeprom/EEPROM_GUIDE.md) - 参数保存

---

## 参考资料

- [Wikipedia - PID Controller](https://en.wikipedia.org/wiki/PID_controller)
- [Arduino PID Library](https://github.com/br3ttb/Arduino-PID-Library/)
- "控制理论基础" - 经典控制理论教材

---

**祝你调试顺利！🎯**
