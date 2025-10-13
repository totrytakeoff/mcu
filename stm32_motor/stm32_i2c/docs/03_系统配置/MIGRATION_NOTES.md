# 项目迁移说明 (Migration Notes)

## 概述 (Overview)
本文档记录了从 CMake 构建项目到 PlatformIO 项目的迁移过程。

## 迁移的文件 (Migrated Files)

### 1. Motor Control Module (电机控制模块)
- **源文件位置**: `stm32_cmake/Core/Src/motor.cpp` → `stm32_pio/src/motor.cpp`
- **头文件位置**: `stm32_cmake/Core/Inc/motor.hpp` → `stm32_pio/include/motor.hpp`
- **功能**: Motor 类实现，支持四个直流电机的 PWM 控制
- **速度范围**: -100 (全速倒转) 到 100 (全速前进)

### 2. Timer Configuration (定时器配置)
- **源文件位置**: `stm32_cmake/Core/Src/tim.c` → `stm32_pio/src/tim.c`
- **头文件位置**: `stm32_cmake/Core/Inc/tim.h` → `stm32_pio/include/tim.h`
- **功能**: TIM3 配置为 4 通道 PWM 输出，用于电机控制
- **PWM 频率**: 50Hz (20ms 周期)
- **GPIO 引脚**: PC6 (CH1), PC7 (CH2), PC8 (CH3), PC9 (CH4)

### 3. GPIO Configuration (GPIO 配置)
- **源文件位置**: `stm32_cmake/Core/Src/gpio.c` → `stm32_pio/src/gpio.c`
- **头文件位置**: `stm32_cmake/Core/Inc/gpio.h` → `stm32_pio/include/gpio.h`
- **功能**: GPIO 端口时钟使能和基础配置

### 4. Main Program (主程序)
- **源文件位置**: `stm32_cmake/Core/Src/main.cpp` → `stm32_pio/src/main.cpp`
- **功能**: 
  - HAL 库初始化
  - 系统时钟配置 (72MHz)
  - 电机初始化和控制逻辑
  - 演示程序：前进 → 停止 → 后退 → 停止 (循环)

### 5. Common Header (公共头文件)
- **文件位置**: `stm32_pio/include/common.h` (已更新)
- **功能**: 包含错误处理和系统时钟配置的函数声明

## 硬件配置 (Hardware Configuration)

### TIM3 PWM 输出引脚
| 通道 | GPIO 引脚 | 功能 |
|------|----------|------|
| TIM3_CH1 | PC6 | 电机 1 控制 |
| TIM3_CH2 | PC7 | 电机 2 控制 |
| TIM3_CH3 | PC8 | 电机 3 控制 |
| TIM3_CH4 | PC9 | 电机 4 控制 |

### PWM 信号参数
- **频率**: 50Hz (20ms 周期)
- **定时器时钟**: 1MHz (72MHz / 72)
- **脉宽范围**: 1250us - 1750us
  - 1250us: 全速倒转 (-100)
  - 1500us: 停止 (0)
  - 1750us: 全速前进 (+100)

## 电机控制逻辑 (Motor Control Logic)

### 电机方向配置
```cpp
// 电机 1 和 3: 同向旋转
// 电机 2 和 4: 同向旋转 (与 1、3 相反)

// 前进
motor1.setSpeed(50);   // 正向
motor3.setSpeed(50);   // 正向
motor2.setSpeed(-50);  // 反向
motor4.setSpeed(-50);  // 反向

// 后退
motor1.setSpeed(-50);  // 反向
motor3.setSpeed(-50);  // 反向
motor2.setSpeed(50);   // 正向
motor4.setSpeed(50);   // 正向
```

## 编译和上传 (Build & Upload)

### 使用 PlatformIO CLI
```bash
# 编译项目
pio run

# 编译并上传
pio run --target upload

# 串口监视
pio device monitor
```

### 使用 VS Code
1. 打开 PlatformIO 侧边栏
2. 点击 "Build" 编译
3. 点击 "Upload" 上传
4. 点击 "Monitor" 监视串口

## 上传配置 (Upload Configuration)

当前配置使用串口上传：
- **端口**: COM6
- **协议**: serial
- **波特率**: 115200

如需使用 ST-Link：
```ini
; 取消注释以下行
debug_tool = stlink
upload_protocol = stlink

; 注释掉串口上传相关配置
; upload_protocol = serial
; upload_port = COM6
```

## 注意事项 (Notes)

1. **系统时钟**: 项目配置为使用外部 8MHz 晶振，PLL 倍频至 72MHz
2. **电机驱动器**: 代码适配 MDA12E11 电机驱动模块
3. **PWM 极性**: 配置为低电平有效 (TIM_OCPOLARITY_LOW)
4. **定时器重映射**: 使用 TIM3 全重映射到 GPIOC

## 未迁移的部分 (Not Migrated)

以下模块在原 CMake 项目中存在但未用于电机控制，因此未迁移：
- ADC 配置 (`adc.h/adc.c`)
- I2C 配置 (`i2c.h/i2c.c`)
- USART 配置 (`usart.h/usart.c`)
- 颜色传感器模块 (`colour_GY33.h/colour_GY33.c`)

如需这些模块，可以从 `stm32_cmake/Core` 目录中手动复制。

## 测试建议 (Testing Recommendations)

1. **首次测试**: 建议先使用较低速度 (如 30-40) 测试
2. **检查方向**: 确认电机旋转方向是否符合预期
3. **PWM 信号**: 使用示波器检查 PWM 输出信号
4. **电源供应**: 确保电机驱动器供电充足

## 问题排查 (Troubleshooting)

### 电机不转
1. 检查 PWM 信号输出 (PC6-PC9)
2. 确认电机驱动器供电
3. 验证驱动器使能信号

### 方向错误
- 调整 `setSpeed()` 参数的正负号

### 编译错误
- 确保所有头文件路径正确
- 检查 `platformio.ini` 配置

## 相关文档 (Related Documents)

- 电机驱动手册: `docs/MDA12E11-830机器人驱动组件产品手册.pdf`
- 控制板手册: `docs/RCB6406_12机器人控制板用户手册.pdf`
- 电机调试指南: `MOTOR_DEBUG_GUIDE.md`
