# C++ 中断处理快速检查清单

## ⚡ 30秒快速检查

你的 STM32 PlatformIO C++ 项目不工作？按此清单检查：

### ✅ 第1步：检查中断文件

打开 `src/stm32f1xx_it.cpp`，确认文件结构：

```cpp
#include "common.h"

#ifdef __cplusplus
extern "C" {        // ⬅️ 必须有这个！
#endif

void SysTick_Handler(void) {
    HAL_IncTick();
}

// ... 所有其他中断函数

#ifdef __cplusplus
}                   // ⬅️ 必须有这个！
#endif
```

### ✅ 第2步：检查文件扩展名

推荐使用 `.c` 扩展名：

```
✅ 推荐: src/stm32f1xx_it.c  (自动使用C链接)
⚠️ 可以: src/stm32f1xx_it.cpp (必须添加 extern "C")
```

### ✅ 第3步：验证编译输出

运行编译并检查符号：

```bash
pio run
arm-none-eabi-nm .pio/build/*/firmware.elf | grep SysTick

# 正确输出：
08000abc T SysTick_Handler

# 错误输出（有名称修饰）：
08000abc T _Z16SysTick_Handlerv  # ❌ 需要添加 extern "C"
```

---

## 🚨 常见错误症状

| 症状 | 原因 | 解决方案 |
|------|------|----------|
| HAL_Delay() 永远卡死 | SysTick_Handler 未被调用 | 添加 extern "C" |
| PWM 无输出 | TIM 中断未被调用 | 添加 extern "C" |
| 串口中断不触发 | UART 中断未被调用 | 添加 extern "C" |
| 程序能上传但行为异常 | 所有中断都失效 | 添加 extern "C" |

---

## 📖 详细文档

完整说明请阅读：**[CRITICAL_CPP_LINKAGE_FIX.md](CRITICAL_CPP_LINKAGE_FIX.md)**

---

## 🎯 标准模板

复制此模板到你的 `stm32f1xx_it.cpp`:

```cpp
/**
 * @file    stm32f1xx_it.cpp
 * @brief   Interrupt Service Routines
 */

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void NMI_Handler(void) { }
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }

void SysTick_Handler(void) {
    HAL_IncTick();
}

// 添加其他中断处理函数...

#ifdef __cplusplus
}
#endif
```

---

**记住：在 C++ 项目中，所有中断和回调函数必须用 `extern "C"` 声明！**
