# 调试系统快速参考

## 🚀 快速开始

```cpp
#include "debug.hpp"
#include "usart.h"

// 初始化
MX_USART2_UART_Init();

// 启用调试
Debug_Enable();

// 输出调试信息
Debug_Printf("Hello STM32\r\n");
```

---

## 📌 核心API

| 函数 | 功能 | 受开关控制 |
|------|------|-----------|
| `Debug_Enable()` | 启用调试 | - |
| `Debug_Disable()` | 禁用调试 | - |
| `Debug_IsEnabled()` | 查询状态 | - |
| `Debug_Printf(fmt, ...)` | 调试输出 | ✅ |
| `printf(fmt, ...)` | 标准输出 | ✅ |
| `Debug_Print_Always(fmt, ...)` | 强制输出 | ❌ |

---

## 💡 常用示例

### 基本输出
```cpp
Debug_Enable();
Debug_Printf("传感器值: %d\r\n", sensor);
```

### 使用printf
```cpp
Debug_Enable();
printf("温度: %d°C\r\n", temp);
```

### 错误信息（总是输出）
```cpp
Debug_Print_Always("错误: %d\r\n", err);
```

### 条件调试
```cpp
if (error) {
    Debug_Enable();
    Debug_Printf("发现错误: %d\r\n", error);
}
```

### 格式化输出
```cpp
Debug_Printf("整数: %d\r\n", 123);
Debug_Printf("十六进制: 0x%02X\r\n", 0xFF);
Debug_Printf("字符: %c\r\n", 'A');
Debug_Printf("字符串: %s\r\n", "Hello");
```

---

## ⚙️ 硬件配置

- **串口**: USART2
- **引脚**: PA2(TX), PA3(RX)
- **波特率**: 115200
- **格式**: 8N1

---

## 📝 注意事项

1. ✅ 使用前必须初始化USART2
2. ✅ 字符串末尾加 `\r\n` 换行
3. ❌ 避免在中断中使用
4. ✅ 发布版本建议禁用调试

---

## 🔧 性能优化

```cpp
// 开发阶段
Debug_Enable();

// 发布版本
Debug_Disable();

// 只在需要时启用
if (need_debug) {
    Debug_Enable();
    Debug_Printf("调试信息\r\n");
    Debug_Disable();
}
```

---

**详细文档**: `docs/DEBUG_SYSTEM_GUIDE.md`
