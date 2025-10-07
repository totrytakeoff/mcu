# STM32F103 CMake从零开始项目

## 项目概述

这是一个完全从零开始构建的STM32F103项目，展示如何不依赖任何IDE，仅使用CMake构建系统和命令行工具来开发STM32项目。

### 🎯 项目特色

- ✅ **纯CMake构建** - 不依赖任何IDE
- ✅ **模块化设计** - 清晰的驱动程序架构
- ✅ **完整的HAL支持** - 使用STM32Cube HAL库
- ✅ **自动化脚本** - 构建和烧录脚本
- ✅ **详细文档** - 每个模块都有完整说明

### 🚀 主要功能

- 系统时钟配置 (72MHz)
- GPIO控制 (RGB LED)
- PWM输出 (舵机和LED控制)
- 模块化驱动程序

## 🛠️ 开发环境要求

### 必需工具

```bash
# ARM工具链
sudo apt install gcc-arm-none-eabi

# 构建工具
sudo apt install cmake make

# 烧录和调试工具
sudo apt install openocd

# 可选：代码分析工具
sudo apt install cppcheck clang-format
```

### VSCode扩展推荐

```json
{
    "recommendations": [
        "ms-vscode.cpptools",
        "ms-vscode.cmake-tools", 
        "marus25.cortex-debug",
        "dan-c-underwood.arm",
        "zixuanwang.linkerscript"
    ]
}
```

## 📁 项目结构

```
02_cmake_from_scratch/
├── CMakeLists.txt              # 主CMake配置文件
├── STM32F103VETx_FLASH.ld     # 链接脚本
├── cmake/
│   └── arm-none-eabi-gcc.cmake # ARM工具链配置
├── include/                    # 头文件目录
│   ├── main.h
│   ├── pwm_driver.h
│   ├── gpio_driver.h
│   ├── clock_config.h
│   └── stm32f1xx_hal_conf.h
├── src/                        # 源文件目录
│   ├── main.c
│   ├── pwm_driver.c
│   ├── gpio_driver.c
│   ├── clock_config.c
│   ├── stm32f1xx_hal_msp.c
│   └── stm32f1xx_it.c
├── scripts/                    # 构建脚本
│   ├── build.sh               # 构建脚本
│   └── flash.sh               # 烧录脚本
├── drivers/                    # HAL库文件 (需要下载)
└── README.md
```

## 🚀 快速开始

### 1. 获取HAL库

```bash
# 下载STM32CubeF1
git clone https://github.com/STMicroelectronics/STM32CubeF1.git temp_cube
mkdir -p drivers

# 复制必需的HAL库文件
cp -r temp_cube/Drivers/STM32F1xx_HAL_Driver drivers/
cp -r temp_cube/Drivers/CMSIS drivers/

# 清理临时文件
rm -rf temp_cube
```

### 2. 构建项目

```bash
# 使用构建脚本（推荐）
./scripts/build.sh

# 或者手动构建
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. 烧录程序

```bash
# 烧录程序
./scripts/flash.sh

# 或者使用CMake目标
cd build
make flash
```

### 4. 调试程序

```bash
# 启动调试服务器
./scripts/flash.sh debug

# 在另一个终端使用GDB
arm-none-eabi-gdb build/stm32f103_cmake_demo.elf
(gdb) target remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

## 📊 硬件连接

```
STM32F103开发板              外设
┌─────────────┐            ┌─────────────┐
│    PA0      │───────────▶│  舵机信号   │
│    PA1      │──220Ω─────▶│  LED正极    │
│    PB0      │───────────▶│  绿色LED    │
│    PB1      │───────────▶│  蓝色LED    │
│    PB5      │───────────▶│  红色LED    │
│    GND      │───────────▶│  公共地线   │
└─────────────┘            └─────────────┘
```

## 🔧 自定义配置

### 修改目标芯片

在`CMakeLists.txt`中修改：

```cmake
set(MCU_MODEL STM32F103xE)  # 改为你的芯片型号
set(CPU_PARAMETERS 
    -mcpu=cortex-m3         # 改为对应的CPU核心
    -mthumb
    -mfloat-abi=soft
)
```

### 修改时钟配置

在`src/clock_config.c`中修改：

```c
#define HSE_FREQUENCY           8000000     // 外部晶振频率
#define TARGET_SYSCLK_FREQ      72000000    // 目标系统时钟
```

### 添加新的驱动模块

1. 在`include/`中创建头文件
2. 在`src/`中创建源文件
3. 在`CMakeLists.txt`中添加源文件

## 🎨 代码风格

项目遵循以下代码风格：

- 函数命名：`Module_FunctionName()`
- 变量命名：`snake_case`
- 宏定义：`UPPER_CASE`
- 文件命名：`module_name.c/h`

## 📋 构建选项

### 构建类型

```bash
# Debug构建
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 最小尺寸构建
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel
```

### 编译选项

在`CMakeLists.txt`中可以修改：

```cmake
# 优化选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")  # 优化尺寸

# 调试选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")  # 最大调试信息

# 警告选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
```

## 🐛 故障排除

### 构建问题

1. **工具链未找到**
   ```bash
   export ARM_TOOLCHAIN_PATH=/path/to/arm-none-eabi
   ```

2. **HAL库未找到**
   ```bash
   # 确保drivers目录存在并包含HAL库文件
   ls drivers/STM32F1xx_HAL_Driver/Inc/
   ```

3. **链接错误**
   ```bash
   # 检查链接脚本路径
   ls STM32F103VETx_FLASH.ld
   ```

### 烧录问题

1. **OpenOCD连接失败**
   ```bash
   # 检查ST-Link连接
   lsusb | grep STMicro
   
   # 检查权限
   sudo usermod -a -G dialout $USER
   ```

2. **目标芯片不匹配**
   ```bash
   # 修改OpenOCD配置
   # 在flash.sh中更改target配置文件
   ```

## 📚 学习资源

- [CMake官方文档](https://cmake.org/documentation/)
- [ARM GCC工具链文档](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
- [OpenOCD用户手册](http://openocd.org/doc/html/index.html)
- [STM32F1xx HAL用户手册](https://www.st.com/resource/en/user_manual/dm00154093-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf)

## 🤝 贡献指南

1. Fork项目
2. 创建特性分支
3. 提交更改
4. 创建Pull Request

## 📄 许可证

本项目采用MIT许可证 - 查看LICENSE文件了解详情

---

**作者**: 基于官方Keil demo改进  
**版本**: v1.0.0  
**更新时间**: 2024年