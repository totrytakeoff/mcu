# STM32 CMake 基础示例 - LED闪烁

这是一个使用CMake构建STM32项目的基础示例，演示了如何使用C++编写STM32程序并实现LED闪烁功能。

## 项目概述

- **芯片型号**: STM32F103RCTx
- **功能**: PC13 LED每秒闪烁一次
- **编程语言**: C++
- **构建工具**: CMake

## 硬件要求

- STM32F103RCTx开发板（如STM32F103C8T6最小系统板）
- LED连接到PC13引脚（大多数STM32F103开发板板载LED）

## 项目结构

```
basic_blink/
├── Core/
│   ├── Inc/                    # Core头文件
│   │   └── main.hpp
│   ├── Src/                    # Core源文件
│   │   └── main.cpp
│   └── Startup/                # 启动文件
│       └── startup_stm32f103rctx.s
├── Drivers/                    # STM32 HAL驱动
│   ├── CMSIS/
│   │   ├── Include/
│   │   └── Device/ST/STM32F1xx/Include/
│   └── STM32F1xx_HAL_Driver/
│       ├── Inc/
│       └── Src/
├── CMakeLists.txt              # 主构建文件
├── CMake/                      # CMake模块和工具链文件
│   └── arm-none-eabi.cmake
└── README.md                   # 本文件
```

## 快速开始

### 1. 准备工作

确保你已经安装了以下工具：

1. **ARM GCC工具链**
   - Windows: 从 [ARM Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm) 下载
   - Linux: `sudo apt install gcc-arm-none-eabi`
   - macOS: `brew install arm-none-eabi-gcc`

2. **CMake** (3.15或更高版本)
   - Windows: 从 [CMake官网](https://cmake.org/download/) 下载
   - Linux: `sudo apt install cmake`
   - macOS: `brew install cmake`

### 2. 设置工具链路径

#### Windows
```cmd
set ARM_NONE_EABI_GCC_PATH=C:\devtools\arm-none-eabi-gcc\bin
```

#### Linux/macOS
```bash
export ARM_NONE_EABI_GCC_PATH=/usr/bin
```

### 3. 构建项目

#### Windows
```cmd
cd basic_blink
build.bat Debug
```

#### Linux/macOS
```bash
cd basic_blink
chmod +x ../build.sh
../build.sh Debug
```

### 4. 烧录程序

构建完成后，在`build`目录中会生成以下文件：

- `basic_blink.elf` - ELF格式的可执行文件
- `basic_blink.hex` - Intel HEX格式的烧录文件
- `basic_blink.bin` - 二进制格式的烧录文件

使用相应的烧录工具将`.hex`或`.bin`文件烧录到STM32F103RCTx芯片中。

## 代码说明

### main.cpp

主要的程序逻辑：

1. **系统初始化**
   - `HAL_Init()` - 初始化HAL库
   - `SystemClock_Config()` - 配置系统时钟

2. **外设初始化**
   - `MX_GPIO_Init()` - 初始化GPIO
   - `MX_DMA_Init()` - 初始化DMA
   - `MX_ADC1_Init()` - 初始化ADC
   - `MX_TIM2_Init()` - 初始化定时器

3. **主循环**
   - 每秒翻转PC13引脚状态，实现LED闪烁

### main.hpp

包含必要的头文件和函数声明：

- `stm32f1xx_hal.h` - STM32 HAL库头文件
- `Error_Handler()` - 错误处理函数声明

### startup_stm32f103rctx.s

启动文件，包含：

- 向量表定义
- 复位处理程序
- 中断处理程序
- 系统初始化

## 自定义配置

### 修改LED引脚

在`main.cpp`中的`MX_GPIO_Init()`函数中修改：

```cpp
// 将PC13改为其他引脚，如PB0
GPIO_InitStruct.Pin = GPIO_PIN_0;  // 改为GPIO_PIN_0
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  // 改为GPIOB
```

在主循环中相应修改：

```cpp
HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // 改为GPIOB和GPIO_PIN_0
```

### 修改闪烁频率

在主循环中修改延时时间：

```cpp
HAL_Delay(500);  // 改为500ms，实现2Hz闪烁
```

## 故障排除

### 常见问题

1. **编译错误：找不到ARM工具链**
   - 检查`ARM_NONE_EABI_GCC_PATH`环境变量是否正确设置
   - 确认ARM GCC工具链已正确安装

2. **链接错误：未定义的引用**
   - 检查所有必要的源文件是否已添加到CMakeLists.txt
   - 确认链接器脚本路径正确

3. **运行时问题：LED不闪烁**
   - 检查LED连接的引脚是否正确
   - 确认引脚配置为输出模式
   - 检查系统时钟配置是否正确

### 调试技巧

1. **查看详细编译信息**
   ```bash
   cmake --build build --verbose
   ```

2. **清理构建缓存**
   ```bash
   rm -rf build/
   ```

3. **使用调试器**
   - 配置OpenOCD或ST-Link调试器
   - 使用GDB进行调试

## 扩展功能

### 添加新的外设

1. 在`main.cpp`中添加初始化函数
2. 在`MX_GPIO_Init()`后调用初始化函数
3. 在CMakeLists.txt中添加相应的源文件

### 使用中断

1. 在`startup_stm32f103rctx.s`中启用中断
2. 编写中断处理函数
3. 在CMakeLists.txt中添加中断处理源文件

### 使用RTOS

1. 添加FreeRTOS或CMSIS-RTOS2源文件
2. 配置CMakeLists.txt
3. 修改启动文件以支持RTOS

## 相关链接

- [STM32 HAL库文档](https://www.st.com/en/embedded-software/x-cube-hal.html)
- [ARM GCC工具链文档](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
- [CMake官方文档](https://cmake.org/documentation/)
