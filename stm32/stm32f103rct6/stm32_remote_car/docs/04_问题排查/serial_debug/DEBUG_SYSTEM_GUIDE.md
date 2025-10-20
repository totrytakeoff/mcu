# 串口调试系统使用指南

## 📋 目录
- [简介](#简介)
- [功能特性](#功能特性)
- [硬件连接](#硬件连接)
- [快速开始](#快速开始)
- [API接口说明](#api接口说明)
- [使用示例](#使用示例)
- [高级用法](#高级用法)
- [常见问题](#常见问题)

---

## 简介

本调试系统基于 **printf重定向** 实现，将标准C库的printf函数输出重定向到STM32的USART2串口，支持运行时动态开关调试输出。

**核心特性：**
- ✅ 支持标准printf重定向
- ✅ 运行时动态控制调试开关
- ✅ 提供专用调试函数
- ✅ 支持无条件输出（用于错误信息）
- ✅ 零开销（调试关闭时不影响性能）

---

## 功能特性

### 1. 三种输出方式

| 函数 | 说明 | 受调试开关控制 |
|------|------|----------------|
| `Debug_Printf()` | 调试输出函数 | ✅ 是 |
| `printf()` | 标准printf（已重定向） | ✅ 是 |
| `Debug_Print_Always()` | 无条件输出 | ❌ 否 |

### 2. 调试控制函数

| 函数 | 说明 |
|------|------|
| `Debug_Enable()` | 启用调试输出 |
| `Debug_Disable()` | 禁用调试输出 |
| `Debug_IsEnabled()` | 查询调试状态 |

---

## 硬件连接

### USART2 引脚配置

```
STM32F103RCT6          USB转TTL模块
┌──────────┐          ┌──────────┐
│          │          │          │
│  PA2(TX) ├─────────►│   RXD    │
│          │          │          │
│  PA3(RX) │◄─────────┤   TXD    │
│          │          │          │
│    GND   ├──────────┤   GND    │
└──────────┘          └──────────┘
```

### 串口参数

- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验位**: 无
- **流控**: 无

---

## 快速开始

### 1. 包含头文件

```cpp
#include "debug.hpp"
#include "usart.h"
```

### 2. 初始化串口

```cpp
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    // 初始化调试串口
    MX_USART2_UART_Init();
    
    // 启用调试输出
    Debug_Enable();
    
    // 输出启动信息
    Debug_Printf("系统启动成功\r\n");
    
    while(1)
    {
        // 你的代码
    }
}
```

### 3. 使用调试输出

```cpp
// 方式1: 使用Debug_Printf
Debug_Printf("传感器值: %d\r\n", sensor_value);

// 方式2: 使用标准printf（已重定向）
printf("温度: %d°C\r\n", temperature);

// 方式3: 无条件输出（用于错误信息）
Debug_Print_Always("错误: 初始化失败\r\n");
```

---

## API接口说明

### 调试控制函数

#### `void Debug_Enable(void)`
启用调试输出。

**示例:**
```cpp
Debug_Enable();
Debug_Printf("调试已启用\r\n");  // 会输出
```

---

#### `void Debug_Disable(void)`
禁用调试输出。

**示例:**
```cpp
Debug_Disable();
Debug_Printf("这条不会输出\r\n");  // 不会输出
```

---

#### `uint8_t Debug_IsEnabled(void)`
查询调试是否启用。

**返回值:**
- `1`: 调试已启用
- `0`: 调试已禁用

**示例:**
```cpp
if (Debug_IsEnabled()) {
    Debug_Printf("调试开启中\r\n");
}
```

---

### 调试输出函数

#### `void Debug_Printf(const char* format, ...)`
格式化调试输出，仅在调试启用时输出。

**参数:**
- `format`: 格式化字符串（与printf相同）
- `...`: 可变参数

**示例:**
```cpp
Debug_Printf("简单消息\r\n");
Debug_Printf("整数: %d, 字符: %c\r\n", 100, 'A');
Debug_Printf("十六进制: 0x%02X\r\n", 0xFF);
```

---

#### `void Debug_Print_Always(const char* format, ...)`
无条件格式化输出，不受调试开关控制。

**用途:** 输出重要信息、错误信息、警告等

**示例:**
```cpp
Debug_Disable();  // 即使禁用调试
Debug_Print_Always("严重错误: %d\r\n", error_code);  // 依然会输出
```

---

#### `int printf(const char* format, ...)`
标准C库printf函数，已重定向到USART2。

**特点:**
- 受调试开关控制
- 语法与标准printf完全相同

**示例:**
```cpp
printf("标准printf输出\r\n");
printf("数值: %d, 浮点: %.2f\r\n", 100, 3.14);
```

---

## 使用示例

### 示例1: 基本使用

```cpp
#include "debug.hpp"
#include "usart.h"

int main(void)
{
    HAL_Init();
    MX_USART2_UART_Init();
    
    // 启用调试
    Debug_Enable();
    Debug_Printf("系统启动\r\n");
    
    int sensor = 1234;
    Debug_Printf("传感器值: %d\r\n", sensor);
    
    // 关闭调试
    Debug_Disable();
    Debug_Printf("这条不会输出\r\n");
    
    while(1);
}
```

---

### 示例2: 使用标准printf

```cpp
#include "debug.hpp"
#include <stdio.h>

void test_printf(void)
{
    Debug_Enable();
    
    // 标准printf已被重定向
    printf("Hello World\r\n");
    printf("数值: %d, 字符: %c\r\n", 100, 'A');
    
    // 支持所有printf格式
    printf("十进制: %d\r\n", 255);
    printf("十六进制: 0x%02X\r\n", 255);
    printf("八进制: %o\r\n", 255);
    printf("字符串: %s\r\n", "STM32");
}
```

---

### 示例3: 条件调试

```cpp
void motor_control(int speed)
{
    // 只在速度异常时输出调试信息
    if (speed > 100 || speed < 0)
    {
        Debug_Enable();
        Debug_Printf("警告: 速度异常 = %d\r\n", speed);
    }
    else
    {
        Debug_Disable();
    }
    
    // 设置电机速度...
}
```

---

### 示例4: 错误信息输出

```cpp
void init_sensors(void)
{
    Debug_Disable();  // 正常运行时关闭调试
    
    int ret = sensor_init();
    if (ret != 0)
    {
        // 即使调试关闭，错误信息也要输出
        Debug_Print_Always("错误: 传感器初始化失败, 代码=%d\r\n", ret);
    }
}
```

---

### 示例5: 循环中选择性输出

```cpp
void main_loop(void)
{
    int count = 0;
    
    while(1)
    {
        count++;
        
        // 每隔100次输出一次（避免刷屏）
        if (count % 100 == 0)
        {
            Debug_Enable();
            Debug_Printf("循环计数: %d\r\n", count);
            Debug_Disable();
        }
        
        HAL_Delay(10);
    }
}
```

---

### 示例6: 启动信息输出

```cpp
void print_startup_info(void)
{
    Debug_Enable();
    
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("  STM32F103 遥控小车系统\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("固件版本: v1.0.0\r\n");
    Debug_Printf("编译时间: %s %s\r\n", __DATE__, __TIME__);
    Debug_Printf("芯片ID: 0x%08X\r\n", HAL_GetDEVID());
    Debug_Printf("系统时钟: %lu MHz\r\n", HAL_RCC_GetSysClockFreq()/1000000);
    Debug_Printf("========================================\r\n");
    Debug_Printf("\r\n");
}
```

---

## 高级用法

### 1. 编译时控制

在代码中使用宏定义控制调试：

```cpp
// 在配置文件中定义
#define DEBUG_ENABLED 1

void some_function(void)
{
    #if DEBUG_ENABLED
        Debug_Enable();
        Debug_Printf("调试模式启用\r\n");
    #else
        Debug_Disable();
    #endif
}
```

---

### 2. 模块化调试控制

为不同模块设置独立的调试开关：

```cpp
// 配置文件
#define DEBUG_MOTOR     1
#define DEBUG_SENSOR    1
#define DEBUG_BLUETOOTH 0

// 电机模块
void motor_debug(const char* msg)
{
    #if DEBUG_MOTOR
        Debug_Printf("[MOTOR] %s\r\n", msg);
    #endif
}

// 传感器模块
void sensor_debug(int value)
{
    #if DEBUG_SENSOR
        Debug_Printf("[SENSOR] value=%d\r\n", value);
    #endif
}
```

---

### 3. 带时间戳的调试

```cpp
void debug_with_timestamp(const char* msg)
{
    uint32_t tick = HAL_GetTick();
    Debug_Printf("[%lu ms] %s\r\n", tick, msg);
}

// 使用
debug_with_timestamp("系统启动");
// 输出: [1234 ms] 系统启动
```

---

### 4. 调试级别控制

```cpp
typedef enum {
    DEBUG_LEVEL_NONE = 0,
    DEBUG_LEVEL_ERROR,
    DEBUG_LEVEL_WARN,
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_DEBUG
} DebugLevel_t;

DebugLevel_t g_debug_level = DEBUG_LEVEL_INFO;

void debug_log(DebugLevel_t level, const char* format, ...)
{
    if (level <= g_debug_level)
    {
        char buffer[256];
        va_list args;
        
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        const char* prefix[] = {"", "[ERROR] ", "[WARN] ", "[INFO] ", "[DEBUG] "};
        Debug_Printf("%s%s", prefix[level], buffer);
    }
}

// 使用
debug_log(DEBUG_LEVEL_ERROR, "严重错误\r\n");
debug_log(DEBUG_LEVEL_INFO, "传感器值: %d\r\n", 100);
```

---

## 常见问题

### Q1: 串口没有输出？

**检查清单:**
1. 确认已初始化USART2: `MX_USART2_UART_Init()`
2. 确认已启用调试: `Debug_Enable()`
3. 检查硬件连接（TX、RX、GND）
4. 检查串口工具波特率设置（115200）
5. 确认使用了`\r\n`换行符

---

### Q2: printf输出乱码？

**解决方法:**
1. 检查波特率是否匹配（115200）
2. 检查数据位、停止位、校验位配置
3. 尝试使用`Debug_Printf`代替`printf`
4. 检查是否有中文字符（建议使用英文）

---

### Q3: 如何完全禁用调试以节省Flash和RAM？

**方法1: 编译时禁用**
```cpp
// 在debug.hpp中添加
#define DEBUG_COMPILE_ENABLE 0

#if DEBUG_COMPILE_ENABLE
    #define Debug_Printf(...) Debug_Printf(__VA_ARGS__)
#else
    #define Debug_Printf(...) ((void)0)  // 空操作
#endif
```

**方法2: 运行时禁用**
```cpp
Debug_Disable();  // 简单但仍占用代码空间
```

---

### Q4: 调试输出影响程序性能？

**优化建议:**
1. 正常运行时禁用调试: `Debug_Disable()`
2. 只在需要时临时启用
3. 减少调试输出频率（例如每N次循环输出一次）
4. 缩短调试字符串长度
5. 使用DMA模式发送（需修改底层代码）

---

### Q5: 支持浮点数输出吗？

**支持，但需要配置:**

在 `platformio.ini` 中添加编译选项：
```ini
build_flags = 
    -u _printf_float
```

然后可以使用：
```cpp
float temp = 25.6;
Debug_Printf("温度: %.2f°C\r\n", temp);
```

**注意:** 浮点printf会增加大约10KB代码空间

---

### Q6: 如何在中断中使用调试？

**注意事项:**
```cpp
void TIM2_IRQHandler(void)
{
    // ⚠️ 不推荐在中断中使用调试输出
    // 原因：HAL_UART_Transmit会阻塞，影响中断响应
    
    // 如果必须在中断中调试，使用标志位
    g_debug_flag = 1;
}

void main_loop(void)
{
    if (g_debug_flag)
    {
        Debug_Printf("中断已触发\r\n");
        g_debug_flag = 0;
    }
}
```

---

## 技术细节

### printf重定向原理

**GCC编译器:**
```cpp
int _write(int file, char *ptr, int len)
{
    if (!g_debug_enabled) return len;
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 1000);
    return len;
}
```

**ARM编译器 (Keil):**
```cpp
int fputc(int ch, FILE *f)
{
    if (!g_debug_enabled) return ch;
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 1000);
    return ch;
}
```

---

## 总结

✅ **推荐用法:**
- 开发阶段: 启用调试 `Debug_Enable()`
- 发布版本: 禁用调试 `Debug_Disable()`
- 错误信息: 使用 `Debug_Print_Always()`

✅ **性能优化:**
- 正常运行时关闭调试
- 减少调试输出频率
- 使用条件编译控制

✅ **最佳实践:**
- 使用有意义的调试信息
- 添加时间戳或模块标识
- 避免在中断中使用
- 定期清理不必要的调试代码

---

**文档版本:** v1.0.0  
**更新时间:** 2024  
**作者:** AI Assistant
