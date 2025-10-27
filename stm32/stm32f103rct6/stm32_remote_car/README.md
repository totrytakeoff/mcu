# STM32F103RC 电机控制项目 (Motor Control Project)

## ⚠️ 重要警告 - 必读！

**如果你的STM32 C++项目遇到以下问题：**

- HAL_Delay() 卡死不返回
- 定时器/PWM 不工作
- 中断处理函数不被调用
- 程序能编译上传但运行异常

**👉 请立即阅读：[CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md)**

这是一个隐蔽但致命的C++链接问题，会导致所有中断函数失效！

---

## 📋 项目概述

这是一个基于 STM32F103RC 的四轮电机控制项目，使用 PlatformIO 构建系统。项目实现了通过 PWM 信号控制 4 个直流电机的功能，适用于机器人底盘等应用场景。

**本项目从 CMake 构建版本迁移而来，详细迁移说明请参阅 [MIGRATION_NOTES.md](MIGRATION_NOTES.md)**

### 🎯 项目特点

- ✅ 4 通道 PWM 电机控制
- ✅ 面向对象的 Motor 类封装
- ✅ 完整的系统时钟配置 (72MHz)
- ✅ 模块化的代码组织
- ✅ 支持正反转和速度控制 (-100 ~ 100)
- ✅ 适配 MDA12E11 电机驱动模块
- ✅ **双源遥控**：E49 无线 + ESP32-S3 蓝牙
- ✅ **梯形速度轮廓**：平滑加减速控制
- ✅ **摇杆模式**：360度精确方向控制

---

## 📚 重要文档

**📖 完整文档导航请查看：[INDEX.md](INDEX.md) - 按症状快速查找解决方案**

| 文档                                                            | 说明                              | 重要程度    |
| --------------------------------------------------------------- | --------------------------------- | ----------- |
| [INDEX.md](INDEX.md)                                               | 📖**文档索引 - 快速查找**   | 🌟 推荐入口 |
| [CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md)         | ⚠️**C++中断链接问题详解** | 🔥🔥🔥 必读 |
| [CPP_INTERRUPT_CHECKLIST.md](CPP_INTERRUPT_CHECKLIST.md)           | ⚡ 30秒快速检查清单               | 🔥🔥 推荐   |
| [docs/BLUETOOTH_CONTROL_GUIDE.md](docs/BLUETOOTH_CONTROL_GUIDE.md) | 📱**蓝牙控制完整指南**      | 🔥🔥 推荐   |
| [docs/LED_DEBUG_GUIDE.md](docs/LED_DEBUG_GUIDE.md)                 | 💡 LED调试指南                    | 🔥 参考     |
| [UPLOAD_CONFIG.md](UPLOAD_CONFIG.md)                               | 串口上传完整指南                  | 🔥 参考     |
| [MIGRATION_NOTES.md](MIGRATION_NOTES.md)                           | CMake→PIO迁移说明                | 🔥 参考     |

---

## 📁 项目结构

```
stm32_pio/
├── platformio.ini                    # PlatformIO 项目配置文件
├── README.md                         # 项目文档（本文件）
├── INDEX.md                          # 📖 文档索引
├── CRITICAL_CPP_LINKAGE_FIX.md      # ⚠️ C++链接问题详解
├── CPP_INTERRUPT_CHECKLIST.md       # ⚡ 快速检查清单
├── UPLOAD_CONFIG.md                  # 串口上传完整指南
├── MIGRATION_NOTES.md                # 迁移说明文档
├── include/                          # 头文件目录
│   ├── common.h                     # 公共头文件
│   ├── motor.hpp                    # Motor 类定义
│   ├── tim.h                        # 定时器配置
│   ├── stm32f1xx_hal_conf.h        # HAL 配置（重要！）
│   └── gpio.h             # GPIO 配置
└── src/                    # 源文件目录
    ├── main.cpp           # 主程序（电机控制逻辑）
    ├── motor.cpp          # Motor 类实现
    ├── tim.c              # TIM3 PWM 配置实现
    ├── gpio.c             # GPIO 初始化
    └── stm32f1xx_it.cpp   # 中断处理文件
```

---

## 🔌 硬件连接

### TIM3 PWM 输出引脚

| 通道     | GPIO 引脚 | 功能        | 备注          |
| -------- | --------- | ----------- | ------------- |
| TIM3_CH1 | PC6       | 电机 1 控制 | 与电机 3 同向 |
| TIM3_CH2 | PC7       | 电机 2 控制 | 与电机 4 同向 |
| TIM3_CH3 | PC8       | 电机 3 控制 | 与电机 1 同向 |
| TIM3_CH4 | PC9       | 电机 4 控制 | 与电机 2 同向 |

### PWM 信号参数

- **频率**: 50Hz (20ms 周期)
- **定时器时钟**: 1MHz (72MHz / 72 分频)
- **脉宽范围**:
  - 1250us: 全速倒转 (speed = -100)
  - 1500us: 停止 (speed = 0)
  - 1750us: 全速前进 (speed = 100)

---

## 🚀 快速开始

### **1. 环境准备**

确保已安装：

- VSCode
- PlatformIO 扩展

### **2. 项目配置**

项目已配置为使用 STM32F103RC 芯片：

```ini
[env:genericSTM32F103RC]
platform = ststm32
board = genericSTM32F103RC
framework = stm32cube

build_flags = 
    -DSTM32F103xE
    -DUSE_HAL_DRIVER
    -DHSE_VALUE=8000000L

upload_protocol = serial
upload_port = COM6
monitor_speed = 115200
```

### **3. 编译和烧录**

```bash
# 编译项目
pio run

# 编译并烧录到开发板
pio run --target upload

# 打开串口监视器（如需要）
pio device monitor
```

使用 VS Code：

1. 打开 PlatformIO 侧边栏
2. 点击 "Build" 编译
3. 点击 "Upload" 上传
4. 点击 "Monitor" 监视串口

---

## 💻 代码说明

### **Motor 类使用**

```cpp
// 创建电机对象
Motor motor1;

// 初始化电机（绑定定时器和通道）
motor1.init(&htim3, TIM_CHANNEL_1);

// 启动 PWM 输出
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

// 设置速度（-100 到 100）
motor1.setSpeed(50);    // 半速前进
motor1.setSpeed(-50);   // 半速后退
motor1.setSpeed(100);   // 全速前进

// 停止电机
motor1.stop();

// 反转当前方向
motor1.reverse();

// 设置为最大速度
motor1.maxSpeed();
```

### **主程序流程**

```cpp
int main(void)
{
    // 1. HAL 库初始化
    HAL_Init();
  
    // 2. 系统时钟配置
    SystemClock_Config();
  
    // 3. 外设初始化
    MX_GPIO_Init();
    MX_TIM3_Init();
  
    // 4. 启动 PWM
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  
    // 5. 初始化电机
    Motor motor1, motor2, motor3, motor4;
    motor1.init(&htim3, TIM_CHANNEL_1);
    motor2.init(&htim3, TIM_CHANNEL_2);
    motor3.init(&htim3, TIM_CHANNEL_3);
    motor4.init(&htim3, TIM_CHANNEL_4);
  
    // 6. 控制循环
    while (1) {
        // 前进 2 秒
        motor1.setSpeed(50);
        motor3.setSpeed(50);
        motor2.setSpeed(-50);
        motor4.setSpeed(-50);
        HAL_Delay(2000);
      
        // 停止 1 秒
        motor1.stop();
        motor2.stop();
        motor3.stop();
        motor4.stop();
        HAL_Delay(1000);
      
        // 后退 2 秒
        motor1.setSpeed(-50);
        motor3.setSpeed(-50);
        motor2.setSpeed(50);
        motor4.setSpeed(50);
        HAL_Delay(2000);
      
        // 停止 1 秒
        motor1.stop();
        motor2.stop();
        motor3.stop();
        motor4.stop();
        HAL_Delay(1000);
    }
}
```

### **电机方向说明**

```cpp
// 机器人底盘配置（四轮驱动）
// 前进: 左侧轮正转，右侧轮反转
motor1.setSpeed(50);   // 左前轮 - 正转
motor3.setSpeed(50);   // 左后轮 - 正转
motor2.setSpeed(-50);  // 右前轮 - 反转
motor4.setSpeed(-50);  // 右后轮 - 反转

// 后退: 方向相反
motor1.setSpeed(-50);  // 左前轮 - 反转
motor3.setSpeed(-50);  // 左后轮 - 反转
motor2.setSpeed(50);   // 右前轮 - 正转
motor4.setSpeed(50);   // 右后轮 - 正转

// 左转: 左侧轮反转，右侧轮正转
motor1.setSpeed(-50);
motor3.setSpeed(-50);
motor2.setSpeed(-50);
motor4.setSpeed(-50);

// 右转: 左侧轮正转，右侧轮反转
motor1.setSpeed(50);
motor3.setSpeed(50);
motor2.setSpeed(50);
motor4.setSpeed(50);
```

---

## ⚙️ 技术细节

### **系统时钟配置**

- **外部晶振**: 8MHz HSE
- **PLL 倍频**: ×9
- **系统时钟**: 72MHz
- **AHB 时钟**: 72MHz
- **APB1 时钟**: 36MHz (÷2)
- **APB2 时钟**: 72MHz

### **TIM3 配置**

```c
htim3.Instance = TIM3;
htim3.Init.Prescaler = 71;           // 72MHz / 72 = 1MHz
htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
htim3.Init.Period = 20000;           // 1MHz / 20000 = 50Hz
htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
```

### **PWM 计算公式**

```
PWM 脉宽 (us) = 1500 + speed × 2.5
- speed = 100:  1500 + 100 × 2.5 = 1750us
- speed = 0:    1500 + 0 × 2.5 = 1500us
- speed = -100: 1500 - 100 × 2.5 = 1250us
```

---

## 🔧 自定义配置

### **修改 PWM 频率**

如需修改 PWM 频率，调整 `tim.c` 中的 Period 值：

```c
// 例如改为 100Hz
htim3.Init.Period = 10000;  // 1MHz / 10000 = 100Hz
```

### **修改速度映射**

如需修改速度到脉宽的映射，编辑 `motor.cpp` 中的 `setSpeed()` 函数：

```cpp
void Motor::setSpeed(int speed) {
    if (!initialized_) return;
  
    // 自定义映射公式
    // 例如: 1000us ~ 2000us 范围
    int pulse = 1500 + speed * 5;
    __HAL_TIM_SET_COMPARE(htim_, channel_, pulse);
    speed_ = speed;
}
```

### **修改 GPIO 引脚**

如需使用不同的 GPIO 引脚，修改 `tim.c` 中的 `HAL_TIM_MspPostInit()` 函数。

---

## 🐛 问题排查

### **电机不转动**

1. ✅ 检查 PWM 信号输出（使用示波器）
2. ✅ 确认电机驱动器供电正常
3. ✅ 验证驱动器使能信号
4. ✅ 检查 GPIO 引脚配置和连接

### **电机转向错误**

- 调整 `setSpeed()` 中的正负号
- 或修改硬件连接

### **速度不正确**

- 检查 PWM 脉宽范围是否匹配驱动器要求
- 验证定时器时钟配置

### **编译错误**

```bash
# 清理并重新编译
pio run --target clean
pio run
```

---

## 📈 扩展开发

### **添加速度控制功能**

```cpp
// 平滑启动
void Motor::smoothStart(int targetSpeed) {
    for (int i = 0; i <= targetSpeed; i++) {
        setSpeed(i);
        HAL_Delay(10);
    }
}

// 平滑停止
void Motor::smoothStop() {
    for (int i = speed_; i >= 0; i--) {
        setSpeed(i);
        HAL_Delay(10);
    }
}
```

### **添加位置反馈**

```cpp
// 使用编码器读取电机位置
class MotorWithEncoder : public Motor {
private:
    int encoderCount_;
  
public:
    void updateEncoder();
    int getPosition();
    void resetPosition();
};
```

### **添加 PID 控制**

```cpp
class PIDController {
private:
    float kp_, ki_, kd_;
    float error_, integral_, derivative_;
  
public:
    float calculate(float setpoint, float measured);
};
```

---

## 📚 相关文档

- [MIGRATION_NOTES.md](MIGRATION_NOTES.md) - 项目迁移详细说明
- `docs/MDA12E11-830机器人驱动组件产品手册.pdf` - 电机驱动器手册
- `docs/RCB6406_12机器人控制板用户手册.pdf` - 控制板手册
- `MOTOR_DEBUG_GUIDE.md` - 电机调试指南

### 在线资源

- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103rc.pdf)
- [STM32F1xx HAL 用户手册](https://www.st.com/resource/en/user_manual/dm00154093.pdf)
- [PlatformIO 官方文档](https://docs.platformio.org/)

---

## ⚠️ 安全提示

1. **首次测试时使用低速度**（建议 30-40）
2. **确保电机有足够的活动空间**
3. **检查电源容量是否满足要求**
4. **注意电机驱动器的散热**
5. **添加急停按钮以确保安全**

---

## 📝 版本历史

- **v1.0.0** (2024) - 从 CMake 项目迁移到 PlatformIO
  - ✅ 4 通道 PWM 电机控制
  - ✅ Motor 类封装
  - ✅ 基础运动控制（前进、后退、停止）

---

## 🤝 贡献

欢迎提交问题和改进建议！如果你发现任何 bug 或有功能改进的想法，请创建 issue 或提交 pull request。

---

## 📄 许可证

本项目使用 STMicroelectronics 提供的 HAL 库，相关许可证请参阅相应的 LICENSE 文件。

---

**祝你开发愉快！🚀 Happy Coding!**
