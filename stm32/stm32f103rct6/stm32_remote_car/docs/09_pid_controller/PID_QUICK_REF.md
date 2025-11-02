# PID控制器 - 快速参考

## 🚀 5分钟快速上手

### 基本使用

```cpp
#include "pid_controller.hpp"

// 1. 创建PID控制器
PIDController pid(1.0f, 0.1f, 0.05f);  // Kp, Ki, Kd

// 2. 设置输出限制
pid.setOutputLimits(-100.0f, 100.0f);

// 3. 在循环中使用
float output = pid.compute(setpoint, measured_value);
```

---

## 📋 常用API速查

### 创建与配置

| 函数 | 说明 | 示例 |
|------|------|------|
| `PIDController(kp, ki, kd)` | 构造函数 | `PIDController pid(1.0f, 0.1f, 0.05f)` |
| `setTunings(kp, ki, kd)` | 设置参数 | `pid.setTunings(2.0f, 0.5f, 0.1f)` |
| `setOutputLimits(min, max)` | 输出限制 | `pid.setOutputLimits(-100.0f, 100.0f)` |
| `setSampleTime(dt)` | 采样时间(秒) | `pid.setSampleTime(0.02f)` |

### 计算控制

| 函数 | 说明 | 示例 |
|------|------|------|
| `compute(sp, pv)` | 计算输出 | `float out = pid.compute(100.0f, 75.0f)` |
| `compute(sp, pv, dt)` | 指定时间间隔 | `float out = pid.compute(100, 75, 0.02f)` |
| `reset()` | 重置状态 | `pid.reset()` |

### 状态查询

| 函数 | 返回值 |
|------|--------|
| `getError()` | 当前误差 |
| `getProportional()` | P项输出 |
| `getIntegral()` | I项输出 |
| `getDerivative()` | D项输出 |
| `getOutput()` | 总输出 |
| `getKp()`, `getKi()`, `getKd()` | 参数值 |

### 高级功能

| 函数 | 说明 | 示例 |
|------|------|------|
| `setMode(mode)` | 自动/手动 | `pid.setMode(PIDController::Mode::AUTOMATIC)` |
| `setDirection(dir)` | 正向/反向 | `pid.setDirection(PIDController::Direction::DIRECT)` |
| `setAntiWindup(enable)` | 抗饱和 | `pid.setAntiWindup(true)` |
| `setDerivativeFilter(alpha)` | 微分滤波 | `pid.setDerivativeFilter(0.3f)` |

---

## 🎯 典型应用场景

### 1. 巡线控制

```cpp
PIDController line_pid(0.06f, 0.0f, 1.0f);
line_pid.setOutputLimits(-60.0f, 60.0f);

float pos = sensor.getPosition();  // -1000~1000
float steering = line_pid.compute(0.0f, pos);

left_speed = base_speed + steering;
right_speed = base_speed - steering;
```

**推荐参数：**
- 低速 (20-30): Kp=0.04-0.06, Kd=0.8-1.2
- 中速 (30-50): Kp=0.06-0.08, Kd=1.2-1.8
- 高速 (50-70): Kp=0.10-0.15, Kd=2.0-3.0

### 2. 电机速度控制

```cpp
PIDController speed_pid(0.5f, 0.2f, 0.01f);
speed_pid.setOutputLimits(-100.0f, 100.0f);

float speed = encoder.getSpeed();
float pwm = speed_pid.compute(target_speed, speed);
motor.setPWM(pwm);
```

**推荐参数：**
- Kp: 0.5-2.0
- Ki: 0.1-0.5
- Kd: 0.01-0.1

### 3. 位置控制

```cpp
PIDController pos_pid(2.0f, 0.0f, 0.5f);
pos_pid.setOutputLimits(-100.0f, 100.0f);

float pos = encoder.getPosition();
float speed = pos_pid.compute(target_pos, pos);
```

**推荐参数：**
- Kp: 1.0-5.0
- Ki: 0.0-0.1 (通常不需要)
- Kd: 0.2-1.0

### 4. 温度控制

```cpp
PIDController temp_pid(5.0f, 0.5f, 1.0f);
temp_pid.setOutputLimits(0.0f, 100.0f);

float temp = sensor.read();
float power = temp_pid.compute(target_temp, temp);
heater.setPWM(power);
```

**推荐参数：**
- Kp: 3.0-10.0
- Ki: 0.3-1.0 (需要消除稳态误差)
- Kd: 0.5-2.0

---

## 🔧 参数调节速查

### 三步调参法

```cpp
// 步骤1: 只调P (目标: 能响应，允许震荡)
pid.setTunings(1.0f, 0.0f, 0.0f);
// 反应慢 → 增大Kp
// 震荡强 → 减小Kp

// 步骤2: 加入D (目标: 消除震荡)
pid.setTunings(1.0f, 0.0f, 0.5f);
// 经验: Kd = Kp × (10~25)

// 步骤3: 加入I (目标: 消除稳态误差，可选)
pid.setTunings(1.0f, 0.05f, 0.5f);
// 从小开始: Ki = 0.01~0.1
```

### 常见问题快速诊断

| 现象 | 可能原因 | 解决方法 |
|------|---------|---------|
| 震荡严重 | Kp太大 | 减小Kp或增大Kd |
| 响应慢 | Kp太小 | 增大Kp |
| 有稳态误差 | 缺少I项 | 加小的Ki (0.01-0.1) |
| 超调大 | Ki太大 | 减小Ki或设为0 |
| D项噪声 | 传感器噪声 | 启用微分滤波 |
| 积分饱和 | 未启用抗饱和 | `setAntiWindup(true)` |

---

## 📊 调试输出模板

### 基本调试

```cpp
Debug_Printf("Error: %.2f, Output: %.2f\r\n",
             pid.getError(), pid.getOutput());
```

### 详细调试

```cpp
Debug_Printf("SP: %.2f, PV: %.2f, Err: %.2f\r\n", 
             setpoint, measured, pid.getError());
Debug_Printf("P: %.2f, I: %.2f, D: %.2f, Out: %.2f\r\n",
             pid.getProportional(),
             pid.getIntegral(),
             pid.getDerivative(),
             pid.getOutput());
```

### CSV输出（用于绘图）

```cpp
// 时间,目标,测量,输出
printf("%.3f,%.2f,%.2f,%.2f\r\n",
       HAL_GetTick()/1000.0f,
       setpoint, measured, pid.getOutput());
```

---

## ⚡ 性能提示

### ✅ 推荐做法

```cpp
// 1. 设置合适的采样时间
pid.setSampleTime(0.02f);  // 20ms

// 2. 启用抗饱和
pid.setAntiWindup(true);

// 3. 如果不需要I项，设为0
pid.setTunings(kp, 0.0f, kd);

// 4. 噪声大时启用微分滤波
pid.setDerivativeFilter(0.2f);

// 5. 只在需要时调参
if (need_retune) {
    pid.setTunings(new_kp, new_ki, new_kd);
}
```

### ❌ 避免做法

```cpp
// 1. 不要过快调用compute
// HAL_Delay(1);  // 太快！
HAL_Delay(20);    // ✓ 合适

// 2. 不要频繁调参
// for (...) { pid.setTunings(...); }  // ✗

// 3. 不要忘记设置输出限制
// pid.setOutputLimits(-100, 100);  // ✓ 必须设置
```

---

## 🎓 PID公式速记

### 标准PID

```
output = Kp×e + Ki×∫e·dt + Kd×de/dt
```

### 各项作用

| 项 | 作用 | 何时增大 | 何时减小 |
|----|------|---------|---------|
| **P** | 响应速度 | 反应慢 | 震荡大 |
| **I** | 消除稳态误差 | 有误差 | 震荡/超调 |
| **D** | 抑制震荡 | 震荡大 | 噪声大 |

### 参数关系经验

```
Kd ≈ Kp × (10~25)
Ki ≈ Kp × (0.01~0.5)

采样时间: 系统响应时间的 1/10 ~ 1/20
```

---

## 🔍 代码模板

### 完整初始化模板

```cpp
#include "pid_controller.hpp"

// 创建PID
PIDController pid(1.0f, 0.1f, 0.05f);

void setup_pid() {
    // 基本配置
    pid.setOutputLimits(-100.0f, 100.0f);
    pid.setSampleTime(0.02f);
    
    // 高级配置
    pid.setAntiWindup(true);
    pid.setDerivativeFilter(0.2f);
    pid.setDirection(PIDController::Direction::DIRECT);
    pid.setMode(PIDController::Mode::AUTOMATIC);
}

void control_loop() {
    float setpoint = 100.0f;
    float measured = sensor.read();
    
    // 计算输出
    float output = pid.compute(setpoint, measured);
    
    // 应用输出
    actuator.apply(output);
}
```

### 串级PID模板

```cpp
PIDController outer(2.0f, 0.0f, 0.5f);  // 慢环
PIDController inner(0.5f, 0.2f, 0.01f); // 快环

void cascade_control() {
    // 外环
    float outer_sp = target_position;
    float outer_pv = current_position;
    float inner_sp = outer.compute(outer_sp, outer_pv);
    
    // 内环
    float inner_pv = current_speed;
    float output = inner.compute(inner_sp, inner_pv);
    
    motor.apply(output);
}
```

---

## 📝 EEPROM保存模板

```cpp
#include "eeprom.hpp"

struct __attribute__((packed)) PIDParams {
    float kp, ki, kd;
};

void save_pid_params(PIDController& pid, EEPROM& eeprom) {
    PIDParams params = {
        pid.getKp(),
        pid.getKi(),
        pid.getKd()
    };
    eeprom.writeStructCRC(0x00, params);
}

bool load_pid_params(PIDController& pid, EEPROM& eeprom) {
    PIDParams params;
    if (eeprom.readStructCRC(0x00, params)) {
        pid.setTunings(params.kp, params.ki, params.kd);
        return true;
    }
    return false;
}
```

---

## 🎯 特殊场景配置

### 场景1: 快速响应（跟踪控制）

```cpp
pid.setTunings(2.0f, 0.0f, 0.5f);  // 大Kp
pid.setOutputLimits(-100.0f, 100.0f);
pid.setDerivativeFilter(0.0f);      // 不滤波
```

### 场景2: 平稳控制（温度控制）

```cpp
pid.setTunings(5.0f, 0.5f, 2.0f);   // 有I项
pid.setOutputLimits(0.0f, 100.0f);
pid.setDerivativeFilter(0.5f);      // 强滤波
pid.setSampleTime(1.0f);            // 慢采样
```

### 场景3: 抗干扰（噪声环境）

```cpp
pid.setTunings(1.0f, 0.0f, 0.5f);   // 小Kd
pid.setDerivativeFilter(0.8f);      // 强滤波
pid.setAntiWindup(true);
```

### 场景4: 省电模式

```cpp
pid.setSampleTime(0.1f);            // 降低频率
pid.setTunings(0.5f, 0.0f, 0.0f);   // 只用P
```

---

## 🚨 故障排查清单

- [ ] 输出限制是否设置？
- [ ] 采样时间是否合理？
- [ ] 传感器读数是否正常？
- [ ] 控制方向是否正确？
- [ ] 是否启用抗饱和？
- [ ] Kp、Ki、Kd是否为正数？
- [ ] 控制循环是否定时执行？

---

## 📚 扩展阅读

- [PID_CONTROLLER_GUIDE.md](PID_CONTROLLER_GUIDE.md) - 完整指南
- [examples/pid_controller_example.cpp](../../examples/pid_controller_example.cpp) - 示例代码

---

**快速参考完毕！更多详情请查看完整指南。** 📖
