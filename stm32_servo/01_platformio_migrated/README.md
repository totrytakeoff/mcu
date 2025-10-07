# STM32F103 PWM Demo - PlatformIO版本

## 项目概述

这是从Keil MDK项目迁移到PlatformIO的STM32F103 PWM控制演示项目。

### 主要功能
- 舵机控制 (TIM2_CH1 - PA0)
- LED亮度控制 (TIM2_CH2 - PA1)  
- 板载RGB LED状态指示

### 硬件要求
- STM32F103VE开发板
- SG90舵机
- LED + 220Ω限流电阻
- 外部5V电源(≥1A)

## 快速开始

### 1. 安装依赖

```bash
# 安装PlatformIO Core
pip install platformio

# 或者安装VSCode扩展
# PlatformIO IDE
```

### 2. 编译项目

```bash
cd stm32_dev/01_platformio_migrated
pio run
```

### 3. 烧录程序

```bash
# 使用ST-Link烧录
pio run --target upload

# 或指定端口
pio run --target upload --upload-port COM3
```

### 4. 串口监视

```bash
pio device monitor --port COM3 --baud 115200
```

## 项目结构

```
01_platformio_migrated/
├── platformio.ini          # PlatformIO配置文件
├── include/                 # 头文件目录
│   ├── main.h
│   ├── pwm_control.h
│   ├── led_control.h
│   ├── stm32f1xx_it.h
│   └── stm32f1xx_hal_conf.h
├── src/                     # 源文件目录
│   ├── main.c
│   ├── pwm_control.c
│   ├── led_control.c
│   ├── stm32f1xx_it.c
│   └── stm32f1xx_hal_msp.c
├── scripts/                 # 构建脚本
│   └── build_info.py
└── README.md
```

## 从Keil迁移的主要变化

### 1. 库框架变更
- **Keil**: 使用STM32标准外设库
- **PlatformIO**: 使用STM32Cube HAL库

### 2. 函数API变更
```c
// Keil版本 (标准外设库)
TIM_SetCompare1(TIM2, ccr_value);

// PlatformIO版本 (HAL库)
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_value);
```

### 3. 初始化方式变更
- **Keil**: 手动配置寄存器
- **PlatformIO**: 使用HAL结构体配置

### 4. 项目配置
- **Keil**: `.uvprojx`项目文件
- **PlatformIO**: `platformio.ini`配置文件

## 常用命令

```bash
# 清理构建
pio run --target clean

# 构建并烧录
pio run --target upload

# 查看设备
pio device list

# 串口监视
pio device monitor

# 更新平台
pio platform update

# 查看库信息
pio lib list
```

## 调试配置

在`platformio.ini`中已配置ST-Link调试：

```ini
debug_tool = stlink
upload_protocol = stlink
```

使用VSCode调试：
1. 按F5开始调试
2. 设置断点
3. 查看变量和寄存器

## 故障排除

### 1. 编译错误
- 检查HAL库配置
- 确认包含路径正确

### 2. 烧录失败
- 检查ST-Link连接
- 确认目标芯片型号

### 3. 程序不运行
- 检查时钟配置
- 验证GPIO配置

## 扩展功能

可以轻松添加更多功能：
- UART通信
- ADC采样
- SPI/I2C外设
- RTOS支持

## 参考资料

- [PlatformIO文档](https://docs.platformio.org/)
- [STM32Cube HAL文档](https://www.st.com/en/embedded-software/stm32cube-mcu-packages.html)
- [STM32F1xx HAL驱动](https://github.com/STMicroelectronics/STM32CubeF1)