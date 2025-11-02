# 基于PID的巡线系统 - 完整指南

## 📚 目录

1. [概述](#概述)
2. [快速开始](#快速开始)
3. [算法原理](#算法原理)
4. [API参考](#api参考)
5. [参数调节](#参数调节)
6. [使用示例](#使用示例)
7. [常见问题](#常见问题)

---

## 概述

### 系统特点

✅ **使用通用PID控制器**
- 基于刚封装的`PIDController`类
- 完整的P+I+D控制
- 积分抗饱和
- 微分滤波

✅ **加权算法计算位置**
- 8个传感器加权平均
- 精确的线位置计算
- 自动阈值判断

✅ **差速转向控制**
- PID输出直接转换为速度差
- 左右轮独立控制
- 平滑的转向响应

✅ **丢线处理**
- 自动检测丢线
- 保持转向方向
- 降速搜线

✅ **易于使用**
- 简洁的API
- 详细的调试输出
- 参数可调

---

## 快速开始

### 1. 硬件连接

```
传感器阵列：8路灰度传感器（0-7，从左到右）
电机：4个直流电机（左前、左后、右前、右后）
校准按钮：PD2
```

### 2. 基本使用

```cpp
#include "line_follower_pid.hpp"
#include "line_sensor.hpp"
#include "motor.hpp"

// 初始化硬件
LineSensor sensor;
Motor motor_lf, motor_lr, motor_rf, motor_rr;

// 创建巡线控制器
LineFollowerPID follower(sensor, motor_lf, motor_lr, motor_rf, motor_rr);

// 配置参数
follower.setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);
follower.setPID(0.06f, 0.0f, 1.0f);
follower.setBaseSpeed(30);
follower.enableDebug(true);

// 初始化并启动
follower.init();
follower.start();

// 主循环
while (1) {
    follower.update();  // 20ms调用一次
    HAL_Delay(20);
}
```

### 3. 校准传感器

```cpp
// 方法1：自动校准（推荐）
Button calib_button(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 200);
sensor.autoCalibrate(calib_button);

// 方法2：手动校准
sensor.calibrateWhite();  // 放在白色区域
HAL_Delay(1000);
sensor.calibrateBlack();  // 放在黑线上

// 保存到EEPROM
EEPROM eeprom;
eeprom.init(&hi2c2, 0xA0);
sensor.saveCalibration(eeprom);
```

---

## 算法原理

### 1. 整体流程

```
传感器读取 → 二值化 → 加权计算位置 → PID计算 → 差速控制 → 电机驱动
     ↓           ↓            ↓           ↓          ↓           ↓
   8路ADC     阈值判断    加权平均    速度差    左右轮速   PWM输出
```

### 2. 传感器权重

8个传感器的位置权重（左负右正）：

```
传感器索引:    0       1       2       3       4       5       6       7
位置权重:  -1000  -714.3  -428.6  -142.9  +142.9  +428.6  +714.3  +1000
```

**示例**：
```
传感器位置: [0]   [1]   [2]   [3]   [4]   [5]   [6]   [7]
         ═══════════════════════════════════════════════
传感器值:    500  2500  3500  3000  1000   500   500   500
二值化:       0     1     1     1     0     0     0     0
         (低值) (高值)(高值)(高值)(低值)(低值)(低值)(低值)

加权计算:
weighted_sum = (-714.3 * 2500) + (-428.6 * 3500) + (-142.9 * 3000)
             = -1785750 - 1500100 - 428700
             = -3714550

total_weight = 2500 + 3500 + 3000 = 9000

position = -3714550 / 9000 = -412.7

结论：线在左侧偏中位置（-412.7）
```

### 3. PID控制

**误差计算**：
```cpp
error = target_position - current_position
      = 0 - position
```

**速度差计算**：
```cpp
speed_diff = PID.compute(0, position)
           = Kp * error + Ki * integral + Kd * derivative
```

**差速控制**：
```cpp
left_speed  = base_speed + speed_diff
right_speed = base_speed - speed_diff
```

**转向逻辑**：
- 线在左边（position < 0）→ error > 0 → speed_diff > 0 → 左快右慢 → 右转
- 线在右边（position > 0）→ error < 0 → speed_diff < 0 → 右快左慢 → 左转

### 4. 丢线处理

**检测条件**：
```cpp
if (检测到线的传感器数量 < 阈值) {
    丢线 = true;
}
```

**处理策略**：
1. 保持上次位置（继续转向）
2. 降低速度到50%
3. 持续搜线

---

## API参考

### 构造函数

```cpp
LineFollowerPID(LineSensor& sensor,
                Motor& motor_lf, Motor& motor_lb,
                Motor& motor_rf, Motor& motor_rb);
```

**参数**：
- `sensor` - 线传感器对象引用
- `motor_lf` - 左前电机引用
- `motor_lb` - 左后电机引用
- `motor_rf` - 右前电机引用
- `motor_rb` - 右后电机引用

### 基本操作

#### init() - 初始化

```cpp
void init();
```

**功能**：初始化巡线系统，重置所有状态

#### start() - 启动巡线

```cpp
void start();
```

**功能**：启动巡线，重置PID并开始控制

#### stop() - 停止巡线

```cpp
void stop();
```

**功能**：停止巡线，电机停止

#### update() - 更新控制

```cpp
void update();
```

**功能**：更新巡线控制（需在主循环中定期调用，建议20ms）

### 参数配置

#### setPID() - 设置PID参数

```cpp
void setPID(float kp, float ki, float kd);
```

**示例**：
```cpp
follower.setPID(0.06f, 0.0f, 1.0f);  // Kp=0.06, Ki=0, Kd=1.0
```

#### setBaseSpeed() - 设置基础速度

```cpp
void setBaseSpeed(int speed);
```

**参数**：
- `speed` - 基础速度 (0-100)

**示例**：
```cpp
follower.setBaseSpeed(30);  // 30%速度
```

#### setLineMode() - 设置线模式

```cpp
void setLineMode(LineMode mode);
```

**模式**：
- `LineMode::WHITE_ON_BLACK` - 黑底白线（默认）
- `LineMode::BLACK_ON_WHITE` - 白底黑线

**示例**：
```cpp
follower.setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);
```

#### setThreshold() - 设置阈值

```cpp
void setThreshold(uint16_t threshold);
```

**参数**：
- `threshold` - 黑白判断阈值 (0-4095)

**建议值**：
- 黑底白线：1500-2500
- 白底黑线：1500-2500

#### setLineLostThreshold() - 设置丢线阈值

```cpp
void setLineLostThreshold(int min_sensors);
```

**参数**：
- `min_sensors` - 最少传感器数量（默认1）

**示例**：
```cpp
follower.setLineLostThreshold(1);  // 少于1个传感器视为丢线
```

#### enableDebug() - 启用调试

```cpp
void enableDebug(bool enable);
```

**示例**：
```cpp
follower.enableDebug(true);
```

### 状态查询

```cpp
State getState() const;          // 获取当前状态
float getError() const;          // 获取误差
float getPIDOutput() const;      // 获取PID输出
int getLeftSpeed() const;        // 获取左侧速度
int getRightSpeed() const;       // 获取右侧速度
```

### 其他

```cpp
void resetPID();  // 重置PID控制器
```

---

## 参数调节

### PID参数快速参考

| 速度范围 | Kp | Ki | Kd | 说明 |
|---------|----|----|----|----|
| 20-30 | 0.04-0.06 | 0.0 | 0.8-1.2 | 低速，平稳 |
| 30-50 | 0.06-0.08 | 0.0 | 1.2-1.8 | 中速，推荐 |
| 50-70 | 0.10-0.15 | 0.0 | 2.0-3.0 | 高速，激进 |

### 调节步骤

#### 步骤1：设置基础速度

```cpp
follower.setBaseSpeed(30);  // 从低速开始
```

#### 步骤2：只调P参数

```cpp
follower.setPID(0.05f, 0.0f, 0.0f);  // 只有P
```

**观察**：
- 反应慢 → 增大Kp
- 震荡大 → 减小Kp

#### 步骤3：加入D参数

```cpp
follower.setPID(0.06f, 0.0f, 1.0f);  // P+D
```

**经验**：Kd ≈ Kp × (15~25)

**观察**：
- 仍震荡 → 增大Kd
- 响应慢 → 减小Kd

#### 步骤4：可选加入I参数

```cpp
follower.setPID(0.06f, 0.01f, 1.0f);  // P+I+D
```

**注意**：大多数情况不需要I项！

### 其他参数

#### 阈值调节

```cpp
// 查看传感器原始值
uint16_t data[8];
sensor.getRawData(data);

// 白色区域：通常 < 1000
// 黑线上：通常 > 2500

// 设置中间值
follower.setThreshold(2000);
```

#### 速度调节

```cpp
// 根据场地调整速度
follower.setBaseSpeed(30);  // 起始：30
// 逐渐增加，观察效果
follower.setBaseSpeed(40);
follower.setBaseSpeed(50);
```

---

## 使用示例

### 示例1：基本巡线

```cpp
LineFollowerPID follower(sensor, motor_lf, motor_lr, motor_rf, motor_rr);

follower.setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);
follower.setPID(0.06f, 0.0f, 1.0f);
follower.setBaseSpeed(30);
follower.init();
follower.start();

while (1) {
    follower.update();
    HAL_Delay(20);
}
```

### 示例2：带校准的完整系统

```cpp
// 初始化
LineSensor sensor;
EEPROM eeprom;
eeprom.init(&hi2c2, 0xA0);

// 加载校准数据
if (sensor.loadCalibration(eeprom)) {
    Debug_Printf("校准数据加载成功\r\n");
} else {
    Debug_Printf("需要校准\r\n");
    // 进行校准...
    sensor.autoCalibrate(button);
    sensor.saveCalibration(eeprom);
}

// 创建巡线控制器
LineFollowerPID follower(sensor, motor_lf, motor_lr, motor_rf, motor_rr);
follower.setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);
follower.setPID(0.06f, 0.0f, 1.0f);
follower.setBaseSpeed(30);
follower.enableDebug(true);
follower.init();
follower.start();

// 主循环
while (1) {
    follower.update();
    HAL_Delay(20);
}
```

### 示例3：实时参数调整

```cpp
// 通过串口命令调整参数
void processCommand(char* cmd) {
    if (strncmp(cmd, "P", 1) == 0) {
        float kp = atof(cmd + 1);
        follower.setPID(kp, 0.0f, current_kd);
    }
    else if (strncmp(cmd, "D", 1) == 0) {
        float kd = atof(cmd + 1);
        follower.setPID(current_kp, 0.0f, kd);
    }
    else if (strncmp(cmd, "S", 1) == 0) {
        int speed = atoi(cmd + 1);
        follower.setBaseSpeed(speed);
    }
}

// 使用：通过串口发送 "P0.08" 设置Kp=0.08
```

### 示例4：状态监控

```cpp
void printStatus() {
    Debug_Printf("状态: %s\r\n",
                follower.getState() == LineFollowerPID::State::RUNNING ? "运行" :
                follower.getState() == LineFollowerPID::State::LINE_LOST ? "丢线" : "停止");
    Debug_Printf("误差: %.1f\r\n", follower.getError());
    Debug_Printf("PID输出: %.1f\r\n", follower.getPIDOutput());
    Debug_Printf("左速: %d, 右速: %d\r\n",
                follower.getLeftSpeed(), follower.getRightSpeed());
}

// 每5秒输出一次
while (1) {
    follower.update();
    
    if (HAL_GetTick() - last_status_time > 5000) {
        printStatus();
        last_status_time = HAL_GetTick();
    }
    
    HAL_Delay(20);
}
```

---

## 常见问题

### Q1: 小车不动？

**检查**：
1. 电机是否正确初始化？
2. 是否调用了`start()`？
3. 是否在主循环中调用`update()`？
4. 基础速度是否太小？

**解决**：
```cpp
follower.setBaseSpeed(30);  // 确保速度足够
follower.start();
// 在主循环中：
follower.update();
```

### Q2: 小车震荡严重？

**原因**：Kp太大或Kd太小

**解决**：
```cpp
// 减小Kp
follower.setPID(0.04f, 0.0f, 1.0f);

// 或增大Kd
follower.setPID(0.06f, 0.0f, 1.5f);
```

### Q3: 转向反应慢？

**原因**：Kp太小

**解决**：
```cpp
follower.setPID(0.10f, 0.0f, 1.5f);  // 增大Kp
```

### Q4: 经常丢线？

**原因**：
1. 速度太快
2. 传感器未校准
3. 阈值不合适

**解决**：
```cpp
// 降低速度
follower.setBaseSpeed(25);

// 重新校准传感器
sensor.autoCalibrate(button);

// 调整阈值
follower.setThreshold(2000);
```

### Q5: 调试信息不显示？

**检查**：
```cpp
// 确保启用调试
follower.enableDebug(true);

// 确保Debug系统已初始化
Debug_Init();
```

### Q6: 如何查看传感器原始值？

```cpp
uint16_t data[8];
sensor.getRawData(data);

for (int i = 0; i < 8; i++) {
    Debug_Printf("S%d: %d  ", i, data[i]);
}
Debug_Printf("\r\n");
```

### Q7: 如何保存PID参数？

```cpp
// 使用EEPROM保存
struct PIDParams {
    float kp, ki, kd;
    int base_speed;
};

PIDParams params = {0.06f, 0.0f, 1.0f, 30};
eeprom.writeStructCRC(0x80, params);

// 加载
if (eeprom.readStructCRC(0x80, params)) {
    follower.setPID(params.kp, params.ki, params.kd);
    follower.setBaseSpeed(params.base_speed);
}
```

---

## 调试技巧

### 1. 启用详细调试

```cpp
follower.enableDebug(true);
```

**输出格式**：
```
Pos:-412.7 Err:412.7 PID:24.8 L:55 R:5 | S:500 2500 3500 3000 1000 500 500 500 | B:·███·····
```

**解读**：
- `Pos` - 线位置（-1000~1000）
- `Err` - 误差
- `PID` - PID输出
- `L/R` - 左右轮速度
- `S` - 8个传感器原始值
- `B` - 二值化显示（█=检测到线）

### 2. 使用可视化工具

```bash
# 使用PID可视化工具
python tests/pid_visualizer.py COM3 115200
```

### 3. 单步调试

```cpp
// 禁用update，手动控制
follower.stop();

// 单步执行
follower.update();
HAL_Delay(1000);  // 慢速观察
```

---

## 性能优化

### 1. 调整采样频率

```cpp
// 默认20ms，可以调整
const uint32_t UPDATE_INTERVAL = 10;  // 10ms（更快响应）
```

### 2. 滤波器配置

```cpp
// 调整传感器滤波
sensor.setFilterAlpha(0.4f);  // 0.3-0.5推荐
```

### 3. PID微分滤波

PID控制器已自动启用微分滤波（alpha=0.2），如需调整：

```cpp
// 在pid_controller内部已配置，无需额外设置
```

---

## 相关文档

- [PID控制器完整指南](../09_pid_controller/PID_CONTROLLER_GUIDE.md)
- [PID快速参考](../09_pid_controller/PID_QUICK_REF.md)
- [传感器校准指南](../07_sensor_calibration/CALIBRATION_GUIDE.md)
- [完整示例代码](../../examples/line_follower_pid_example.cpp)

---

**祝你巡线成功！🏁**
