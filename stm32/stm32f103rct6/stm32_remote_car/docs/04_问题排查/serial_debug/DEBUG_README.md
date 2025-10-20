# 串口调试系统 - STM32F103

## 📖 简介

这是一个基于printf重定向的STM32串口调试系统，支持运行时动态开关调试输出，适用于STM32F103系列微控制器。

### ✨ 主要特性

- ✅ **printf重定向** - 标准C库printf直接输出到串口
- ✅ **运行时控制** - 动态启用/禁用调试输出
- ✅ **零性能损耗** - 禁用时不影响程序性能
- ✅ **多种输出方式** - 支持Debug_Printf、printf、强制输出
- ✅ **简单易用** - 一行代码即可使用
- ✅ **完整文档** - 提供详细使用指南和示例

---

## 📁 文件结构

```
stm32_remote_car/
├── include/
│   ├── debug.hpp              # 调试系统头文件
│   └── usart.h                # 串口驱动头文件
├── src/
│   ├── debug.cpp              # 调试系统实现
│   └── usart.c                # 串口驱动实现
├── examples/
│   └── debug_example.cpp      # 使用示例代码
├── docs/
│   ├── DEBUG_SYSTEM_GUIDE.md  # 详细使用指南
│   └── DEBUG_QUICK_REF.md     # 快速参考手册
└── DEBUG_README.md            # 本文件
```

---

## 🚀 快速开始

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
    
    // 初始化调试串口（必需）
    MX_USART2_UART_Init();
    
    // ... 其他初始化
}
```

### 3. 使用调试输出

```cpp
// 启用调试
Debug_Enable();

// 方式1: 使用Debug_Printf
Debug_Printf("系统启动\r\n");
Debug_Printf("传感器值: %d\r\n", sensor_value);

// 方式2: 使用标准printf（已重定向）
printf("温度: %d°C\r\n", temperature);

// 方式3: 强制输出（不受开关控制）
Debug_Print_Always("错误代码: %d\r\n", error);

// 禁用调试
Debug_Disable();
```

---

## 🔌 硬件连接

### USART2 配置

```
STM32F103RCT6          USB转TTL
┌──────────┐          ┌──────────┐
│  PA2(TX) ├─────────►│   RXD    │
│  PA3(RX) │◄─────────┤   TXD    │
│    GND   ├──────────┤   GND    │
└──────────┘          └──────────┘
```

### 串口参数
- 波特率: **115200**
- 数据位: 8
- 停止位: 1
- 校验位: 无

---

## 📚 API参考

### 调试控制函数

| 函数 | 说明 |
|------|------|
| `void Debug_Enable(void)` | 启用调试输出 |
| `void Debug_Disable(void)` | 禁用调试输出 |
| `uint8_t Debug_IsEnabled(void)` | 查询调试状态 |

### 调试输出函数

| 函数 | 说明 | 受开关控制 |
|------|------|-----------|
| `Debug_Printf(format, ...)` | 格式化调试输出 | ✅ 是 |
| `printf(format, ...)` | 标准printf（已重定向） | ✅ 是 |
| `Debug_Print_Always(format, ...)` | 强制输出（用于错误） | ❌ 否 |

---

## 💡 使用示例

### 示例1: 基本使用

```cpp
#include "debug.hpp"

void basic_example(void)
{
    // 启用调试
    Debug_Enable();
    
    // 输出信息
    Debug_Printf("系统启动成功\r\n");
    
    int value = 1234;
    Debug_Printf("读取值: %d\r\n", value);
    
    // 禁用调试
    Debug_Disable();
    
    // 这条不会输出
    Debug_Printf("这条消息被忽略\r\n");
}
```

### 示例2: 条件调试

```cpp
void conditional_debug(int sensor_value)
{
    // 只在异常时输出调试信息
    if (sensor_value > 1000) {
        Debug_Enable();
        Debug_Printf("警告: 传感器异常 = %d\r\n", sensor_value);
    } else {
        Debug_Disable();
    }
}
```

### 示例3: 错误处理

```cpp
void error_handling(void)
{
    Debug_Disable();  // 正常情况下关闭调试
    
    int ret = init_hardware();
    if (ret != 0) {
        // 即使调试关闭，错误信息也要输出
        Debug_Print_Always("致命错误: 硬件初始化失败, 代码=%d\r\n", ret);
    }
}
```

### 示例4: 启动信息

```cpp
void print_system_info(void)
{
    Debug_Enable();
    
    Debug_Printf("\r\n");
    Debug_Printf("=====================================\r\n");
    Debug_Printf("  STM32F103 遥控小车系统\r\n");
    Debug_Printf("=====================================\r\n");
    Debug_Printf("固件版本: v1.0.0\r\n");
    Debug_Printf("编译时间: %s %s\r\n", __DATE__, __TIME__);
    Debug_Printf("芯片型号: STM32F103RCT6\r\n");
    Debug_Printf("系统时钟: 72 MHz\r\n");
    Debug_Printf("调试串口: USART2 @ 115200bps\r\n");
    Debug_Printf("=====================================\r\n");
    Debug_Printf("\r\n");
}
```

### 示例5: 使用标准printf

```cpp
void printf_example(void)
{
    Debug_Enable();
    
    // printf已被重定向到USART2
    printf("Hello STM32!\r\n");
    printf("整数: %d, 十六进制: 0x%02X\r\n", 255, 255);
    printf("字符: %c, 字符串: %s\r\n", 'A', "STM32");
}
```

---

## 🎯 应用场景

### 开发阶段
```cpp
// 启用所有调试输出，方便开发调试
Debug_Enable();
```

### 发布版本
```cpp
// 禁用调试输出，提高性能，减少干扰
Debug_Disable();
```

### 故障诊断
```cpp
// 出现问题时临时启用调试
if (system_error) {
    Debug_Enable();
    Debug_Printf("错误诊断信息...\r\n");
}
```

---

## ⚙️ 高级配置

### 修改调试串口

如果要使用USART1作为调试串口，修改`src/debug.cpp`:

```cpp
// 将所有 huart2 改为 huart1
HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 1000);
```

### 修改缓冲区大小

在`src/debug.cpp`中修改:

```cpp
char buffer[256];  // 改为需要的大小，如512
```

### 启用浮点数支持

在`platformio.ini`中添加:

```ini
build_flags = 
    -u _printf_float
```

然后可以使用:
```cpp
float temp = 25.5;
Debug_Printf("温度: %.2f°C\r\n", temp);
```

---

## 🔧 故障排除

### 串口无输出？

**检查清单:**
1. ✅ 已调用 `MX_USART2_UART_Init()`
2. ✅ 已调用 `Debug_Enable()`
3. ✅ 硬件连接正确（TX->RX, RX->TX, GND相连）
4. ✅ 串口工具波特率设置为115200
5. ✅ 字符串末尾加了 `\r\n`

### 输出乱码？

**解决方法:**
1. 检查波特率是否匹配（115200）
2. 检查串口工具的字符编码设置
3. 确认数据位、停止位、校验位配置正确

### 影响程序性能？

**优化建议:**
1. 正常运行时禁用调试: `Debug_Disable()`
2. 减少调试输出频率
3. 缩短调试字符串
4. 只在必要时启用调试

---

## 📖 文档导航

- **快速参考**: `docs/DEBUG_QUICK_REF.md` - 常用API和示例
- **详细指南**: `docs/DEBUG_SYSTEM_GUIDE.md` - 完整功能说明
- **示例代码**: `examples/debug_example.cpp` - 实际应用示例

---

## 🌟 最佳实践

1. ✅ **开发时启用，发布时禁用**
   ```cpp
   #ifdef DEBUG
       Debug_Enable();
   #else
       Debug_Disable();
   #endif
   ```

2. ✅ **使用有意义的调试信息**
   ```cpp
   // 好的做法
   Debug_Printf("[MOTOR] Speed=%d, Direction=%d\r\n", speed, dir);
   
   // 不好的做法
   Debug_Printf("1\r\n");
   ```

3. ✅ **错误信息使用强制输出**
   ```cpp
   if (error) {
       Debug_Print_Always("[ERROR] Init failed: %d\r\n", error);
   }
   ```

4. ✅ **避免在中断中使用**
   ```cpp
   // 不推荐
   void TIM_IRQHandler(void) {
       Debug_Printf("IRQ\r\n");  // 会阻塞中断
   }
   
   // 推荐
   volatile uint8_t flag = 0;
   void TIM_IRQHandler(void) {
       flag = 1;
   }
   void main_loop(void) {
       if (flag) {
           Debug_Printf("IRQ triggered\r\n");
           flag = 0;
       }
   }
   ```

5. ✅ **定期清理调试代码**
   - 删除不必要的调试输出
   - 保留关键的错误处理信息

---

## 🤝 技术支持

如有问题，请参考：
1. `docs/DEBUG_SYSTEM_GUIDE.md` - 详细文档
2. `examples/debug_example.cpp` - 示例代码
3. 项目 README.md

---

## 📜 更新日志

### v1.0.0 (2024)
- ✅ 初始版本发布
- ✅ 支持printf重定向
- ✅ 支持运行时控制
- ✅ 提供完整文档和示例

---

## 📄 许可证

本项目为开源项目，可自由使用和修改。

---

**作者**: AI Assistant  
**版本**: v1.0.0  
**更新**: 2024
