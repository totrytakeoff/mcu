# 系统架构与模块职责

## 📋 整体架构

```
┌─────────────────────────────────────────────────────────┐
│                    用户层（遥控器）                      │
│                  TLE100 遥控器 (433MHz)                  │
└─────────────────────┬───────────────────────────────────┘
                      │ 单字符指令 (F/B/L/R/...)
                      ↓
┌─────────────────────────────────────────────────────────┐
│                    通信层                                │
│              E49_Wireless (E49-400T20S)                 │
│  - 透传模式，9600 波特率                                 │
│  - UART1 接收/发送                                       │
│  - 回调机制通知上层                                      │
└─────────────────────┬───────────────────────────────────┘
                      │ 回调：onDataReceived(char)
                      ↓
┌─────────────────────────────────────────────────────────┐
│                    控制层                                │
│                 RemoteControl                            │
│  - 指令解析与映射                                        │
│  - 累积加速策略（baseSpeed → maxSpeed）                 │
│  - 超时检测与停止                                        │
│  - LED 调试显示                                          │
└─────────────────────┬───────────────────────────────────┘
                      │ setTargetSpeed(straight, turn)
                      ↓
┌─────────────────────────────────────────────────────────┐
│                    运动层                                │
│                  DriveTrain                              │
│  - 差速转向混合（Arcade Drive）                         │
│  - 梯形速度轮廓（MotionProfile × 2）                    │
│  - 死区滤波、速度归一化                                  │
│  - 原地转向优化                                          │
│  - 方向修正                                              │
└─────────────────────┬───────────────────────────────────┘
                      │ setSpeed(pwm)
                      ↓
┌─────────────────────────────────────────────────────────┐
│                    执行层                                │
│                   Motor × 4                              │
│  - PWM 输出控制                                          │
│  - 单个电机速度设置                                      │
│  - 方向控制（正/负速度）                                 │
└─────────────────────────────────────────────────────────┘
                      │ PWM 信号
                      ↓
              ┌───────┴───────┐
              │   电机驱动器   │
              └───────┬───────┘
                      ↓
              ┌───────┴───────┐
              │  直流减速电机  │
              └───────────────┘
```

---

## 🎯 模块职责详解

### 1. **MotionProfile** (新增独立模块)
**文件**：`include/motion_profile.hpp`（头文件内实现）

**职责**：
- ✅ 梯形速度轮廓算法（加速/匀速/减速）
- ✅ 反向刹车逻辑（方向切换时快速减速）
- ✅ 时间片管理（按固定间隔更新）
- ✅ 参数配置（加速度、减速度、反向减速度、更新间隔）

**输入**：
- `setTarget(int target)`：设置目标速度（-100 ~ 100）
- `update(uint32_t nowMs)`：按时间片更新

**输出**：
- `getCurrent()`：获取当前平滑后的速度
- `getTarget()`：获取目标速度

**特点**：
- 🔹 单一职责：只负责速度平滑
- 🔹 无状态依赖：可复用于任何速度控制
- 🔹 时间片驱动：不依赖外部定时器

---

### 2. **DriveTrain** (重构后)
**文件**：`include/drive_train.hpp`, `src/drive_train.cpp`

**职责**：
- ✅ 持有 2 个 `MotionProfile`（直行 + 转向）
- ✅ 差速转向混合（Arcade Drive 算法）
- ✅ 死区滤波（消除抖动）
- ✅ 速度归一化（防止溢出）
- ✅ 原地转向优化（降低速度避免堵转）
- ✅ 方向修正（硬件接线导致的反向）

**输入**：
- `setTargetSpeed(int straight, int turn)`：设置目标直行/转向速度
- `update()`：更新速度轮廓并应用到电机

**输出**：
- 4 个 `Motor` 对象的速度控制

**核心算法**：
```cpp
// 1. 从 MotionProfile 获取平滑后的当前速度
currentStraight = motionStraight_.getCurrent();
currentTurn = motionTurn_.getCurrent();

// 2. 差速转向混合
leftSpeed = currentStraight + adjustedTurn;
rightSpeed = currentStraight - adjustedTurn;

// 3. 归一化 + 限幅 + 方向修正
normalizeSpeed(leftSpeed, rightSpeed);
leftSpeed = -leftSpeed;  // 方向修正
rightSpeed = -rightSpeed;

// 4. 应用到电机
leftMotors.setSpeed(leftSpeed);
rightMotors.setSpeed(-rightSpeed);  // 右侧电机反向
```

**特点**：
- 🔹 不再负责速度平滑（委托给 MotionProfile）
- 🔹 专注于差速转向逻辑
- 🔹 保留硬件相关优化（死区、原地转向、方向修正）

---

### 3. **RemoteControl**
**文件**：`include/remote_control.hpp`, `src/remote_control.cpp`

**职责**：
- ✅ 接收无线指令（通过 E49_Wireless 回调）
- ✅ 指令解析与映射（F/B/L/R/... → 目标速度）
- ✅ **累积加速策略**（baseSpeed 为最低速度，持续按压累加）
- ✅ 超时检测（1秒无指令则停止）
- ✅ LED 调试显示（二进制编码）

**输入**：
- `handleCommand(char cmd)`：处理单字符指令

**输出**：
- `DriveTrain::setTargetSpeed()`：设置目标速度

**累积加速逻辑**：
```cpp
case 'F':  // 前进
    if (首次按下 || 方向切换) {
        目标速度 = baseSpeed_;  // 30%（最低速度）
    } else {
        目标速度 += speedIncrement_;  // +10%
        if (目标速度 > maxSpeed_) 目标速度 = maxSpeed_;  // 限制 100%
    }
    driveTrain_.setTargetSpeed(目标速度, 0);
```

**时间轴示例**：
```
0s:  按下 F → 目标 30% → 实际速度平滑加速 0→30%
1s:  继续 F → 目标 40% → 实际速度平滑加速 30→40%
2s:  继续 F → 目标 50% → 实际速度平滑加速 40→50%
...
7s:  继续 F → 目标 100% → 实际速度平滑加速 90→100%
8s:  超时   → 目标 0%   → 实际速度平滑减速 100→0%
```

**特点**：
- 🔹 只负责"目标速度"的计算（策略层）
- 🔹 不负责速度平滑（委托给 DriveTrain/MotionProfile）
- 🔹 清晰的累积加速逻辑（baseSpeed = 最低，maxSpeed = 最高）

---

### 4. **E49_Wireless**
**文件**：`include/e49_wireless.hpp`, `src/e49_wireless.cpp`

**职责**：
- ✅ E49 模块初始化（透传模式）
- ✅ GPIO 控制（M0/M1 模式切换）
- ✅ UART 数据发送
- ✅ 数据接收回调机制

**输入**：
- UART 中断接收的单字节数据

**输出**：
- 回调 `RemoteControl::handleCommand(char)`

**特点**：
- 🔹 纯通信层，不涉及业务逻辑
- 🔹 使用回调机制解耦

---

### 5. **Motor**
**文件**：`include/motor.hpp`, `src/motor.cpp`

**职责**：
- ✅ 单个电机的 PWM 控制
- ✅ 速度设置（-100 ~ 100）
- ✅ 方向控制（正/负）

**输入**：
- `setSpeed(int speed)`

**输出**：
- PWM 波形输出到电机驱动器

**特点**：
- 🔹 最底层执行单元
- 🔹 无状态，纯执行

---

## 📊 数据流

### 完整流程示例：按下 F 键

```
1. [TLE100 遥控器] 按下 F 键
   ↓
2. [E49-400T20S] 433MHz 发送字符 'F'
   ↓
3. [E49_Wireless] UART1 接收 'F' → 触发中断
   ↓
4. [HAL_UART_RxCpltCallback] 调用 E49::onDataReceived('F')
   ↓
5. [E49_Wireless] 回调 RemoteControl::handleCommand('F')
   ↓
6. [RemoteControl] 
   - LED 显示：setDebugLED(1)  // 0001
   - 计算目标速度：30%（首次）或 += 10%（持续）
   - 调用 driveTrain_.setTargetSpeed(30, 0)
   ↓
7. [DriveTrain]
   - motionStraight_.setTarget(30)
   - motionTurn_.setTarget(0)
   ↓
8. [主循环 10ms 一次]
   - driveTrain.update()
   ↓
9. [MotionProfile]
   - 每 20ms 更新：currentSpeed += acceleration (5)
   - 0 → 5 → 10 → 15 → ... → 30
   ↓
10. [DriveTrain::applySpeedToMotors]
    - 读取 motionStraight_.getCurrent() = 当前速度
    - 差速混合：left = 当前速度, right = 当前速度
    - 死区滤波、归一化、方向修正
    ↓
11. [Motor × 4]
    - leftFront.setSpeed(speed)
    - leftBack.setSpeed(speed)
    - rightFront.setSpeed(-speed)  // 反向
    - rightBack.setSpeed(-speed)
    ↓
12. [PWM 输出] TIM3 → 电机驱动器 → 电机转动
```

---

## 🔄 速度控制的双层结构

### 第一层：目标速度（RemoteControl）
- **作用**：决定"想要达到的速度"
- **特点**：阶跃变化（累积加速）
- **示例**：30 → 40 → 50 → ... → 100

### 第二层：实际速度（MotionProfile）
- **作用**：平滑过渡到目标速度
- **特点**：梯形轮廓（加速/匀速/减速）
- **示例**：0 → 5 → 10 → 15 → ... → 30（平滑）

### 为什么需要双层？
```
❌ 单层（直接设置实际速度）：
   按下 F → 速度立即 30% → 启动冲击大

✅ 双层（目标 + 平滑）：
   按下 F → 目标 30% → 实际速度平滑加速到 30% → 启动平稳
```

---

## ⚙️ 参数配置层级

### 1. 控制策略参数（RemoteControl）
```cpp
remoteControl.setBaseSpeed(30);      // 最低速度（首次按下）
remoteControl.setMaxSpeed(100);      // 最高速度限制
remoteControl.setSpeedIncrement(10); // 每次指令增加的速度
remoteControl.setTurnSensitivity(40);// 转向灵敏度
remoteControl.setTimeout(1000);      // 超时时间
```

### 2. 运动平滑参数（DriveTrain）
```cpp
driveTrain.setAcceleration(5, 8, 12);  // 加速度、减速度、反向减速度
driveTrain.setUpdateInterval(20);      // 更新间隔（20ms = 50Hz）
```

### 3. 底层硬件参数（Motor）
```cpp
motor.init(&htim3, TIM_CHANNEL_1);  // PWM 定时器通道
```

---

## 🎯 模块解耦优势

### 1. **单一职责**
- ✅ `MotionProfile`：只负责速度平滑
- ✅ `DriveTrain`：只负责差速转向
- ✅ `RemoteControl`：只负责指令解析

### 2. **可测试性**
```cpp
// 测试 MotionProfile（独立）
MotionProfile profile;
profile.setTarget(50);
for (int i = 0; i < 10; i++) {
    profile.update(i * 20);
    // 验证加速逻辑
}

// 测试 DriveTrain（模拟目标速度）
driveTrain.setTargetSpeed(50, 0);
driveTrain.update();
// 验证差速混合
```

### 3. **可复用性**
```cpp
// MotionProfile 可用于其他需要平滑的场景
MotionProfile armSpeed;  // 机械臂速度控制
MotionProfile servoAngle;  // 舵机角度控制
```

### 4. **易维护**
```
修改加速度算法 → 只改 MotionProfile
修改差速混合   → 只改 DriveTrain
修改指令映射   → 只改 RemoteControl
```

---

## 📝 总结

### 核心设计原则
1. **分层架构**：通信层 → 控制层 → 运动层 → 执行层
2. **单一职责**：每个模块只做一件事
3. **依赖注入**：上层依赖下层接口，不依赖实现
4. **时间片驱动**：使用 `HAL_GetTick()` 实现非阻塞更新

### 关键改进
- ✅ 抽取 `MotionProfile` 模块（解耦速度平滑逻辑）
- ✅ `DriveTrain` 简化（专注差速转向）
- ✅ `RemoteControl` 清晰（累积加速策略明确）
- ✅ LED 调试功能（可视化指令接收）

### 扩展性
- 🔹 易于添加新指令（只需修改 `RemoteControl::handleCommand`）
- 🔹 易于更换通信模块（只需实现回调接口）
- 🔹 易于调整运动参数（参数化配置）

---

**系统现在结构清晰、职责明确、易于维护！** ✨
