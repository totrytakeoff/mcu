# USART2 串口调试使用指南

## 📌 概述

本文档介绍如何使用 USART2 进行串口调试输出，为后续 E49 无线模块集成做准备。

---

## 🔌 硬件连接

### 需要的硬件
- STM32F103RC 开发板
- USB转TTL模块（CH340/CP2102/PL2303）
- 3根杜邦线

### 接线方式

```
USB转TTL模块          STM32开发板
   VCC (不接)    
   GND   --------->   GND
   TXD   --------->   PA3 (USART2_RX)
   RXD   --------->   PA2 (USART2_TX)
     |
   USB线
     |
    电脑
```

**重要提示**：
- ✅ VCC **不需要接**（开发板自己供电）
- ✅ GND **必须接**（共地）
- ✅ 注意 TX/RX **交叉连接**

---

## ⚙️ 串口监视器设置

### 推荐软件
- Windows: PuTTY、串口助手、XCOM
- macOS/Linux: minicom、screen、CoolTerm

### 参数配置
```
波特率：  115200
数据位：  8
停止位：  1
校验位：  无
流控：    无
```

---

## 💻 代码使用

### 1. 初始化 USART2

```c
// 在 main() 函数开始处初始化
MX_USART2_UART_Init();
HAL_Delay(100);  // 等待串口稳定
```

### 2. 发送字符串

```c
// 方式1：发送普通字符串
USART2_Print("Hello World!\r\n");

// 方式2：发送格式化字符串（类似 printf）
int temperature = 25;
USART2_Printf("温度: %d°C\r\n", temperature);
```

### 3. 常用数据类型示例

```c
// 整数
int value = 123;
USART2_Printf("整数: %d\r\n", value);

// 浮点数
float temp = 25.6f;
USART2_Printf("温度: %.1f°C\r\n", temp);

// 字符
char cmd = 'F';
USART2_Printf("指令: %c\r\n", cmd);

// 字符串
const char* msg = "前进";
USART2_Printf("动作: %s\r\n", msg);

// 十六进制
uint8_t data = 0xAB;
USART2_Printf("数据: 0x%02X\r\n", data);

// 无符号长整型
uint32_t timestamp = HAL_GetTick();
USART2_Printf("时间戳: %lu ms\r\n", timestamp);
```

---

## 🎯 Demo 程序说明

当前 `main.cpp` 中的 Demo 程序会：

1. **启动时**：
   - 显示欢迎信息
   - 显示系统信息（MCU型号、时钟频率、波特率）
   - 演示各种数据类型的输出

2. **运行时**：
   - 每 1 秒输出一次计数
   - 每 5 秒输出一次标记
   - 每 10 秒输出一次系统状态

3. **串口监视器显示示例**：
```
========================================
   USART2 串口调试 Demo 启动成功！
========================================

MCU型号: STM32F103RC
系统时钟: 72 MHz
串口波特率: 115200

--- 数据类型演示 ---
整数: 12345
浮点数: 3.14
字符: A
字符串: Hello STM32
十六进制: 0xABCD

开始主循环...
每1秒输出一次计数

[1000 ms] 计数: 1
[2000 ms] 计数: 2
[3000 ms] 计数: 3
[4000 ms] 计数: 4
[5000 ms] 计数: 5
--- 5秒标记 ---
```

---

## 🔧 后续集成 E49 模块

当你接入 E49 无线模块后，可以这样使用：

```c
// 在 E49 的接收回调函数中
void onE49DataReceived(uint8_t data)
{
    // 打印收到的遥控指令
    USART2_Printf("收到遥控指令: %c\r\n", data);
    
    // 根据指令执行动作
    switch(data) {
        case 'F':
            USART2_Print("动作: 前进\r\n");
            break;
        case 'B':
            USART2_Print("动作: 后退\r\n");
            break;
        case 'L':
            USART2_Print("动作: 左转\r\n");
            break;
        case 'R':
            USART2_Print("动作: 右转\r\n");
            break;
        default:
            USART2_Printf("未知指令: %c\r\n", data);
            break;
    }
}
```

---

## ⚠️ 注意事项

### 1. 换行符
- Windows 系统：使用 `\r\n`
- Linux/macOS：使用 `\n`
- 建议统一使用 `\r\n`（兼容性最好）

### 2. 缓冲区大小
- `USART2_Printf` 内部缓冲区为 256 字节
- 单次输出不要超过 256 字节
- 如需更大缓冲区，修改 `usart.c` 中的 `buffer` 大小

### 3. 性能考虑
- 串口输出会占用 CPU 时间
- 避免在高频中断中使用 `USART2_Printf`
- 调试完成后可以用条件编译关闭串口输出：
```c
#ifdef DEBUG
    USART2_Printf("调试信息\r\n");
#endif
```

### 4. 浮点数输出
- 需要在编译器中启用浮点数支持
- 如果不显示浮点数，检查 PlatformIO 配置：
```ini
build_flags = 
    -u _printf_float
```

---

## 🚀 快速测试步骤

1. **接线**
   - USB转TTL 连接到 PA2/PA3/GND

2. **编译上传**
   - 用 ST-Link 烧录程序

3. **打开串口监视器**
   - 波特率设置为 115200
   - 连接对应的 COM 口

4. **复位开发板**
   - 按下 RESET 按钮
   - 查看串口监视器输出

5. **成功标志**
   - 看到欢迎信息
   - 看到系统信息
   - 看到每秒递增的计数

---

## 📝 API 参考

### `MX_USART2_UART_Init()`
- **功能**: 初始化 USART2
- **参数**: 无
- **返回**: 无
- **说明**: 配置波特率 115200, 8N1

### `USART2_Print(const char* str)`
- **功能**: 发送字符串
- **参数**: `str` - 要发送的字符串
- **返回**: 无
- **示例**: `USART2_Print("Hello\r\n");`

### `USART2_Printf(const char* format, ...)`
- **功能**: 发送格式化字符串
- **参数**: `format` - 格式化字符串，`...` - 可变参数
- **返回**: 无
- **示例**: `USART2_Printf("值: %d\r\n", 123);`

---

## ❓ 常见问题

### Q1: 串口监视器没有输出？
**A**: 检查：
- 波特率是否设置为 115200
- 接线是否正确（TX/RX 交叉）
- GND 是否连接
- COM 口是否选择正确

### Q2: 输出乱码？
**A**: 检查：
- 波特率是否匹配
- 数据位/停止位/校验位设置
- 接线是否稳定

### Q3: 浮点数显示为 0？
**A**: 需要在编译选项中添加 `-u _printf_float`

### Q4: 能同时用 USART1 和 USART2 吗？
**A**: 可以！
- USART1 给 E49 模块（9600）
- USART2 用于调试输出（115200）
- 互不干扰

---

## 📚 相关文档

- `E49_WIRELESS_README.md` - E49 无线模块使用指南
- `IMPLEMENTATION_PLAN.md` - 遥控系统实现计划

---

**最后更新**: 2024
**作者**: AI Assistant
