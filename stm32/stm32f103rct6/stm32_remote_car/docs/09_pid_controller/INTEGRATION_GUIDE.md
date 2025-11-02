# PID控制器集成指南

## 📖 概述

本指南介绍如何将PID控制器集成到现有项目中，以及如何与其他模块配合使用。

---

## 🚀 集成到现有项目

### 步骤1：添加文件

确保项目中包含以下文件：

```
include/
  └── pid_controller.hpp

src/
  └── pid_controller.cpp
```

### 步骤2：包含头文件

在需要使用PID的源文件中包含头文件：

```cpp
#include "pid_controller.hpp"
```

### 步骤3：创建实例

```cpp
// 全局或类成员变量
PIDController speed_pid(0.5f, 0.1f, 0.05f);
PIDController position_pid(2.0f, 0.0f, 0.5f);
```

### 步骤4：初始化

```cpp
void setup() {
    // 配置PID参数
    speed_pid.setOutputLimits(-100.0f, 100.0f);
    speed_pid.setSampleTime(0.02f);
    speed_pid.setAntiWindup(true);
    
    // 可选：从EEPROM加载参数
    load_pid_from_eeprom(speed_pid);
}
```

### 步骤5：在控制循环中使用

```cpp
void control_loop() {
    // 读取传感器
    float measured = sensor.read();
    
    // 计算控制输出
    float output = speed_pid.compute(setpoint, measured);
    
    // 应用到执行器
    motor.setPWM(output);
}
```

---

## 🔗 与现有模块集成

### 1. 与巡线系统集成

**场景**：替换LineFollower中的简单PID为通用PID

```cpp
class LineFollower {
private:
    PIDController line_pid;
    
public:
    LineFollower() : line_pid(0.06f, 0.0f, 1.0f) {
        line_pid.setOutputLimits(-60.0f, 60.0f);
        line_pid.setDerivativeFilter(0.2f);
    }
    
    void update() {
        float position = sensor.getPosition();
        float steering = line_pid.compute(0.0f, position);
        
        // 差速控制
        float left_speed = base_speed + steering;
        float right_speed = base_speed - steering;
        
        drive.setSpeed(left_speed, right_speed);
    }
    
    void setPIDParams(float kp, float ki, float kd) {
        line_pid.setTunings(kp, ki, kd);
    }
};
```

### 2. 与电机控制集成

**场景**：电机速度闭环控制

```cpp
class Motor {
private:
    PIDController speed_pid;
    Encoder encoder;
    
public:
    Motor() : speed_pid(0.8f, 0.2f, 0.01f) {
        speed_pid.setOutputLimits(-100.0f, 100.0f);
        speed_pid.setSampleTime(0.01f);  // 10ms
    }
    
    void setTargetSpeed(float rpm) {
        target_speed = rpm;
    }
    
    void update() {
        float current_speed = encoder.getSpeed();
        float pwm = speed_pid.compute(target_speed, current_speed);
        setPWM(pwm);
    }
};
```

### 3. 与EEPROM集成

**场景**：保存和加载PID参数

```cpp
#include "eeprom.hpp"
#include "pid_controller.hpp"

struct __attribute__((packed)) PIDConfig {
    float kp, ki, kd;
    float output_min, output_max;
    float sample_time;
};

class PIDManager {
private:
    EEPROM& eeprom;
    static constexpr uint16_t PID_ADDR = 0x00;
    
public:
    PIDManager(EEPROM& ee) : eeprom(ee) {}
    
    bool savePID(PIDController& pid) {
        PIDConfig config = {
            pid.getKp(),
            pid.getKi(),
            pid.getKd(),
            -100.0f,  // 需要额外记录限制
            100.0f,
            0.02f
        };
        return eeprom.writeStructCRC(PID_ADDR, config);
    }
    
    bool loadPID(PIDController& pid) {
        PIDConfig config;
        if (eeprom.readStructCRC(PID_ADDR, config)) {
            pid.setTunings(config.kp, config.ki, config.kd);
            pid.setOutputLimits(config.output_min, config.output_max);
            pid.setSampleTime(config.sample_time);
            return true;
        }
        return false;
    }
};

// 使用示例
void setup() {
    EEPROM eeprom;
    eeprom.init(&hi2c2, 0xA0);
    
    PIDController my_pid;
    PIDManager manager(eeprom);
    
    // 加载保存的参数
    if (!manager.loadPID(my_pid)) {
        // 使用默认参数
        my_pid.setTunings(1.0f, 0.1f, 0.05f);
        my_pid.setOutputLimits(-100.0f, 100.0f);
        
        // 保存默认参数
        manager.savePID(my_pid);
    }
}
```

### 4. 与蓝牙/串口调试集成

**场景**：通过串口动态调整PID参数

```cpp
#include "debug.hpp"
#include "bluetooth_control.hpp"

class PIDTuner {
private:
    PIDController& pid;
    
public:
    PIDTuner(PIDController& p) : pid(p) {}
    
    void processCommand(const char* cmd) {
        char action;
        float value;
        
        // 命令格式: "P1.5" (设置Kp=1.5)
        if (sscanf(cmd, "%c%f", &action, &value) == 2) {
            switch (action) {
                case 'P':
                case 'p':
                    pid.setTunings(value, pid.getKi(), pid.getKd());
                    Debug_Printf("Kp = %.3f\r\n", value);
                    break;
                    
                case 'I':
                case 'i':
                    pid.setTunings(pid.getKp(), value, pid.getKd());
                    Debug_Printf("Ki = %.3f\r\n", value);
                    break;
                    
                case 'D':
                case 'd':
                    pid.setTunings(pid.getKp(), pid.getKi(), value);
                    Debug_Printf("Kd = %.3f\r\n", value);
                    break;
                    
                case 'R':
                case 'r':
                    pid.reset();
                    Debug_Printf("PID Reset\r\n");
                    break;
            }
        }
        else if (cmd[0] == '?') {
            // 查询当前参数
            printStatus();
        }
    }
    
    void printStatus() {
        Debug_Printf("\r\n========== PID Status ==========\r\n");
        Debug_Printf("Kp: %.3f\r\n", pid.getKp());
        Debug_Printf("Ki: %.3f\r\n", pid.getKi());
        Debug_Printf("Kd: %.3f\r\n", pid.getKd());
        Debug_Printf("Error: %.2f\r\n", pid.getError());
        Debug_Printf("P: %.2f, I: %.2f, D: %.2f\r\n",
                     pid.getProportional(),
                     pid.getIntegral(),
                     pid.getDerivative());
        Debug_Printf("Output: %.2f\r\n", pid.getOutput());
        Debug_Printf("===============================\r\n");
    }
};

// 使用示例
PIDController my_pid(1.0f, 0.1f, 0.05f);
PIDTuner tuner(my_pid);

void on_bluetooth_command(const char* cmd) {
    tuner.processCommand(cmd);
}
```

### 5. 串级PID控制

**场景**：位置控制（位置环+速度环）

```cpp
class CascadePIDController {
private:
    PIDController outer_pid;  // 位置环（慢）
    PIDController inner_pid;  // 速度环（快）
    
public:
    CascadePIDController() 
        : outer_pid(2.0f, 0.0f, 0.5f)
        , inner_pid(0.5f, 0.2f, 0.01f)
    {
        // 外环：输出是目标速度
        outer_pid.setOutputLimits(-100.0f, 100.0f);
        outer_pid.setSampleTime(0.02f);  // 20ms
        
        // 内环：输出是PWM
        inner_pid.setOutputLimits(-100.0f, 100.0f);
        inner_pid.setSampleTime(0.002f); // 2ms（更快）
    }
    
    float compute(float target_pos, float current_pos, float current_speed) {
        // 外环计算目标速度
        float target_speed = outer_pid.compute(target_pos, current_pos);
        
        // 内环计算PWM
        float pwm = inner_pid.compute(target_speed, current_speed);
        
        return pwm;
    }
    
    void setPositionPID(float kp, float ki, float kd) {
        outer_pid.setTunings(kp, ki, kd);
    }
    
    void setSpeedPID(float kp, float ki, float kd) {
        inner_pid.setTunings(kp, ki, kd);
    }
    
    void reset() {
        outer_pid.reset();
        inner_pid.reset();
    }
};
```

---

## 🎯 典型应用架构

### 架构1：单环控制

```
传感器 → PID控制器 → 执行器
  ↑                      ↓
  └──────────反馈────────┘
```

```cpp
class SimpleController {
private:
    Sensor& sensor;
    Actuator& actuator;
    PIDController pid;
    
public:
    void update() {
        float measured = sensor.read();
        float output = pid.compute(setpoint, measured);
        actuator.apply(output);
    }
};
```

### 架构2：串级控制

```
目标 → 外环PID → 内环PID → 执行器
         ↑          ↑           ↓
    位置传感器  速度传感器    系统
         ↓          ↓           ↓
         └──────────┴───────────┘
```

```cpp
class CascadeController {
private:
    PositionSensor& pos_sensor;
    SpeedSensor& speed_sensor;
    Actuator& actuator;
    PIDController pos_pid;
    PIDController speed_pid;
    
public:
    void update() {
        float pos = pos_sensor.read();
        float speed = speed_sensor.read();
        
        float target_speed = pos_pid.compute(target_pos, pos);
        float output = speed_pid.compute(target_speed, speed);
        
        actuator.apply(output);
    }
};
```

### 架构3：多路PID

```
         ┌─ PID1 → 左电机
传感器 ──┤
         └─ PID2 → 右电机
```

```cpp
class DifferentialController {
private:
    LineSensor& sensor;
    Motor& left_motor;
    Motor& right_motor;
    PIDController steering_pid;
    
public:
    void update() {
        float position = sensor.getPosition();
        float steering = steering_pid.compute(0.0f, position);
        
        // 差速控制
        left_motor.setSpeed(base_speed + steering);
        right_motor.setSpeed(base_speed - steering);
    }
};
```

---

## 🔧 调试工具集成

### 实时监控工具

```cpp
class PIDMonitor {
private:
    PIDController& pid;
    uint32_t last_print;
    
public:
    PIDMonitor(PIDController& p) : pid(p), last_print(0) {}
    
    void update() {
        uint32_t now = HAL_GetTick();
        if (now - last_print >= 100) {  // 每100ms
            last_print = now;
            
            // CSV格式输出（可导入Excel/Python绘图）
            printf("%.3f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
                   now / 1000.0f,           // 时间
                   pid.getError(),          // 误差
                   pid.getProportional(),   // P项
                   pid.getIntegral(),       // I项
                   pid.getDerivative(),     // D项
                   pid.getOutput());        // 输出
        }
    }
};
```

### 参数自动保存

```cpp
class AutoSavePID {
private:
    PIDController& pid;
    EEPROM& eeprom;
    float last_kp, last_ki, last_kd;
    uint32_t last_check;
    
public:
    void update() {
        // 检测参数是否改变
        if (pid.getKp() != last_kp || 
            pid.getKi() != last_ki || 
            pid.getKd() != last_kd) {
            
            // 延迟保存（避免频繁写入）
            uint32_t now = HAL_GetTick();
            if (now - last_check > 5000) {  // 5秒后
                savePID();
                last_check = now;
            }
        }
    }
    
private:
    void savePID() {
        // 保存逻辑...
        last_kp = pid.getKp();
        last_ki = pid.getKi();
        last_kd = pid.getKd();
    }
};
```

---

## 📝 最佳实践

### 1. 模块化设计

```cpp
// 推荐：将PID封装在功能类中
class LineFollower {
private:
    PIDController pid;  // 成员变量
    // ...
};

// 不推荐：使用全局PID
PIDController global_pid;  // 难以管理
```

### 2. 参数管理

```cpp
// 推荐：集中管理参数
struct ControlParams {
    struct {
        float kp, ki, kd;
    } line_pid;
    
    struct {
        float kp, ki, kd;
    } speed_pid;
};

// 从配置文件加载
void loadConfig(ControlParams& params);
```

### 3. 错误处理

```cpp
void updateControl() {
    // 检查传感器状态
    if (!sensor.isValid()) {
        pid.reset();  // 重置PID
        motor.stop();
        return;
    }
    
    // 正常控制
    float output = pid.compute(setpoint, sensor.read());
    motor.apply(output);
}
```

### 4. 调试开关

```cpp
class ControlSystem {
private:
    PIDController pid;
    bool debug_enabled;
    
public:
    void enableDebug(bool enable) {
        debug_enabled = enable;
    }
    
    void update() {
        float output = pid.compute(setpoint, measured);
        
        if (debug_enabled) {
            printDebugInfo();
        }
        
        actuator.apply(output);
    }
};
```

---

## 🚨 常见陷阱

### 1. 忘记设置输出限制

```cpp
// ❌ 错误
PIDController pid(1.0f, 0.1f, 0.05f);
// 没有设置限制！

// ✅ 正确
PIDController pid(1.0f, 0.1f, 0.05f);
pid.setOutputLimits(-100.0f, 100.0f);
```

### 2. 采样时间不一致

```cpp
// ❌ 错误
pid.setSampleTime(0.02f);  // 设置20ms
// 但实际调用周期是10ms
HAL_Delay(10);

// ✅ 正确
pid.setSampleTime(0.01f);  // 与实际周期一致
HAL_Delay(10);
```

### 3. 忘记重置PID

```cpp
// ❌ 错误：切换目标但不重置
pid.compute(100.0f, measured);
// ...
pid.compute(50.0f, measured);  // 积分项还保留着之前的值

// ✅ 正确
pid.compute(100.0f, measured);
// ...
pid.reset();  // 重置
pid.compute(50.0f, measured);
```

---

## 📚 相关文档

- [PID_CONTROLLER_GUIDE.md](PID_CONTROLLER_GUIDE.md) - 完整指南
- [PID_QUICK_REF.md](PID_QUICK_REF.md) - 快速参考
- [EEPROM_GUIDE.md](../06_eeprom/EEPROM_GUIDE.md) - 参数保存
- [DEBUG_SYSTEM_GUIDE.md](../04_问题排查/serial_debug/DEBUG_SYSTEM_GUIDE.md) - 调试方法

---

**集成愉快！🎯**
