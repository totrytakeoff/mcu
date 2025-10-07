# STM32F103RC PlatformIO 基础项目

## 📋 项目概述

这是一个基础的STM32F103RC PlatformIO项目，提供了最简单的STM32开发环境。项目包含基本的系统时钟配置、错误处理和中断处理，可以作为开发更复杂功能的起点。

### 🎯 项目特点

- 最小化的项目结构
- 基本的系统时钟配置
- 完整的错误处理机制
- 标准的中断处理框架
- 模块化的代码组织

---

## 📁 项目结构

```
pio_stm32/
├── platformio.ini          # PlatformIO项目配置文件
├── README.md               # 项目文档
├── include/                # 头文件目录
│   └── common.h           # 公共头文件
└── src/                    # 源文件目录
    ├── main.cpp           # 主程序文件
    └── stm32f1xx_it.cpp   # 中断处理文件
```

### 文件说明

- **platformio.ini**: 项目配置文件，定义了目标板、编译选项等
- **include/common.h**: 公共头文件，包含函数声明和必要的头文件
- **src/main.cpp**: 主程序文件，包含系统初始化和主循环
- **src/stm32f1xx_it.cpp**: 中断处理文件，包含所有中断服务程序

---

## 🚀 快速开始

### **1. 环境准备**

确保已安装：
- VSCode
- PlatformIO 扩展

### **2. 项目配置**

项目已配置为使用STM32F103RC芯片，主要配置如下：

```ini
[env:genericSTM32F103RC]
platform = ststm32
board = genericSTM32F103RC
framework = stm32cube

; 编译选项
build_flags = 
    -DSTM32F103xE
    -DUSE_HAL_DRIVER
    -DHSE_VALUE=8000000L
    -DUSE_FULL_LL_DRIVER

; 上传配置
upload_protocol = serial
upload_port = COM6
```

### **3. 编译和烧录**

```bash
# 编译项目
pio run

# 烧录到开发板
pio run --target upload

# 打开串口监视器
pio device monitor
```

---

## 🔧 代码说明

### **main.cpp - 主程序**

```cpp
int main(void)
{
    // HAL库初始化
    HAL_Init();
    
    // 系统时钟配置
    SystemClock_Config();
    
    // 简单的主循环
    while (1)
    {
        // 基础的空循环
        __asm("nop");
    }
}
```

### **common.h - 公共头文件**

```cpp
#ifndef COMMON_H
#define COMMON_H

#include "stm32f1xx_hal.h"

// 函数声明
void Error_Handler(void);
void SystemClock_Config(void);

#endif /* COMMON_H */
```

### **stm32f1xx_it.cpp - 中断处理**

包含所有必要的中断服务程序：
- 系统异常处理（NMI、HardFault等）
- SysTick中断处理
- 其他标准中断处理

---

## ⚙️ 系统时钟配置

项目使用外部8MHz晶振，通过PLL倍频到72MHz：

```cpp
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // 配置主振荡器
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  // 8MHz * 9 = 72MHz
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    // 配置系统时钟
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}
```

---

## 🐛 错误处理

```cpp
void Error_Handler(void)
{
    // 禁用中断
    __disable_irq();
    
    // 错误处理：进入无限循环
    while (1) {
        // 空循环
    }
}
```

---

## 📈 扩展开发

### **1. 添加新的外设**

1. 在 `common.h` 中添加函数声明
2. 在 `main.cpp` 中实现初始化函数
3. 在 `main` 函数中调用初始化函数
4. 在主循环中添加相应的控制逻辑

### **2. 添加GPIO控制**

```cpp
// 在 common.h 中添加
void GPIO_Init(void);
void GPIO_Toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

// 在 main.cpp 中实现
void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    // GPIO配置代码
}

// 在主循环中使用
GPIO_Toggle(GPIOA, GPIO_PIN_5);
HAL_Delay(500);
```

### **3. 添加UART通信**

```cpp
// 在 common.h 中添加
UART_HandleTypeDef huart1;
void UART_Init(void);

// 在 main.cpp 中实现
void UART_Init(void)
{
    // UART配置代码
}

// 添加printf重定向
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
    return ch;
}
```

### **4. 添加定时器功能**

```cpp
// 在 common.h 中添加
void Timer_Init(void);
void Timer_Start(void);

// 在 main.cpp 中实现
void Timer_Init(void)
{
    // 定时器配置代码
}

// 在主循环中使用
Timer_Start();
HAL_Delay(1000);
```

---

## ⚠️ 开发注意事项

### **1. 时钟配置**

- 确保HSE晶振频率与代码中的定义一致
- 修改时钟配置后需要重新计算Flash等待周期
- APB分频器会影响外设时钟频率

### **2. 引脚配置**

- 使用引脚前必须先使能对应时钟
- 注意引脚的复用功能配置
- 输出引脚需要配置上拉/下拉电阻

### **3. 中断处理**

- 合理设置中断优先级
- 中断服务程序要尽量简短
- 使用标志位在中断和主循环间通信

### **4. 内存管理**

- 注意栈大小配置
- 避免在栈上分配大数组
- 使用静态分配或动态分配管理内存

### **5. 编译选项**

- 确保编译选项与目标芯片匹配
- 注意不同的HAL驱动选项
- 调试版本和发布版本的配置不同

---

## 🔍 调试技巧

### **1. 使用LED指示**

```cpp
// 添加LED控制
void LED_On(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}
void LED_Off(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

// 在代码中使用
LED_On();   // 表示某个状态
HAL_Delay(100);
LED_Off();
```

### **2. 使用串口调试**

```cpp
// 添加UART初始化后可以使用printf
printf("System started\r\n");
printf("Clock frequency: %ld Hz\r\n", HAL_RCC_GetSysClockFreq());
```

### **3. 错误处理增强**

```cpp
void Error_Handler(void) {
    // LED闪烁指示错误
    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(200);
    }
}
```

---

## 📚 学习资源

- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103rc.pdf)
- [STM32F1xx HAL 用户手册](https://www.st.com/resource/en/user_manual/dm00154093.pdf)
- [PlatformIO 官方文档](https://docs.platformio.org/)
- [STM32参考手册](https://www.st.com/resource/en/reference_manual/dm00031936.pdf)

---

## 🎓 开发建议

### **初学者路径**

1. **理解基础概念**：GPIO、时钟、中断等
2. **从简单开始**：先实现LED闪烁
3. **逐步扩展**：添加按键检测、UART通信
4. **深入学习**：理解HAL库的工作原理

### **进阶学习**

1. **优化代码**：减少内存使用，提高执行效率
2. **错误处理**：完善错误检测和处理机制
3. **模块化设计**：提高代码的可维护性
4. **性能分析**：使用工具分析代码性能

---

## 🤝 贡献

欢迎提交问题和改进建议！如果你发现任何bug或有功能改进的想法，请创建issue或提交pull request。

---

**祝你开发愉快！🚀**
