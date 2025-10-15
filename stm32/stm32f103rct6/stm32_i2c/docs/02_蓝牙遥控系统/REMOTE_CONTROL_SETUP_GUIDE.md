# 遥控器与小车通信配置指南

## 📡 系统架构确认

```
TLE100遥控器（出厂固件ex3.c）
    ↓ 
  无线433MHz
    ↓
E49-400T20S模块（在小车开发板上）
    ↓
  UART串口（9600波特率）
    ↓
STM32F103RC主控芯片
```

---

## ❓ 问题1：小车上的无线模块要怎么配置以连接上遥控器

### E49-400T20S模块的模式配置

从遥控器端代码 `ex3.c` 第181-182行可以看到：

```c
WX_M0=0;  // 模式引脚M0设为低电平
WX_M1=0;  // 模式引脚M1设为低电平
```

**E49-400T20S的模式选择表**：

| M1 | M0 | 工作模式 | 说明 |
|----|----|---------|------|
| 0  | 0  | **透传模式** | 串口数据直接无线发送/接收，无封装 |
| 0  | 1  | 唤醒模式 | 支持低功耗唤醒 |
| 1  | 0  | 省电模式 | 休眠模式 |
| 1  | 1  | 配置模式 | 通过AT命令配置参数 |

### 小车端配置方案

**方案A：硬件直接配置（推荐，最简单）**

如果你的小车开发板上E49模块的M0/M1引脚：
- 已经**固定接GND** → 已经是透传模式，无需任何配置！✅
- 通过**跳线帽**可选 → 确保都跳到GND（0）位置

**方案B：软件GPIO控制**

如果M0/M1连接到STM32的GPIO（如官方Demo的PA6/PA7），需要代码配置：

```c
// HAL库配置E49模式引脚
GPIO_InitTypeDef GPIO_InitStruct = {0};

// 假设M0连PA6，M1连PA7（根据你的实际硬件修改）
GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

// 设置为透传模式：M0=0, M1=0
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);  // M0=0
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);  // M1=0
```

### 配对/连接说明

E49-400T20S模块在透传模式下**不需要配对**！
- 只要遥控器和接收模块的**频道参数**一致（出厂默认相同）
- 遥控器发送数据 → 所有同频道的接收模块都会收到
- 类似于对讲机的原理

---

## ❓ 问题2：小车要如何接收处理遥控器发送的指令

### 遥控器发送的数据格式（已确认）

从 `ex3.c` 代码可以100%确认：

| 按键 | 发送字符 | ASCII码 | 功能说明 |
|------|---------|---------|---------|
| Left | `'L'` | 0x4C | 左转 |
| Right | `'R'` | 0x52 | 右转 |
| Forward | `'F'` | 0x46 | 前进 |
| Back | `'B'` | 0x42 | 后退 |
| UpSpeed | `'U'` | 0x55 | 加速 |
| DownSpeed | `'D'` | 0x44 | 减速 |
| F1 | `'W'` | 0x57 | 功能键1 |
| F2 | `'X'` | 0x58 | 功能键2 |
| F3 | `'Y'` | 0x59 | 功能键3 |
| F4 | `'Z'` | 0x5A | 功能键4 |

**重要特性**：
1. **单字符ASCII**：每按一次键发送1个字节
2. **持续发送**：按键按下时每200ms发送一次（见236行`delaynms(200)`）
3. **无帧头帧尾**：透传模式，直接发送字符

### STM32接收处理逻辑

```c
// 伪代码逻辑
while(1) {
    if (收到串口数据) {
        uint8_t cmd = 接收到的字节;
        
        switch(cmd) {
            case 'F':  // 前进
                robot.drive(50, 0);
                break;
            case 'B':  // 后退
                robot.drive(-50, 0);
                break;
            case 'L':  // 左转
                robot.drive(0, 50);
                break;
            case 'R':  // 右转
                robot.drive(0, -50);
                break;
            // ... 其他按键
        }
        
        更新最后接收时间;  // 用于看门狗
    }
    
    // 看门狗：超过一定时间没收到数据就停车
    if (当前时间 - 最后接收时间 > 500ms) {
        robot.stop();
    }
}
```

**关键点**：
- ✅ 简单：直接判断字符即可
- ⚠️ 需要看门狗：按键松开后200ms就不再发送，需要自动停车
- ⚠️ 组合按键：如果同时按F+L，会收到两个字符（先F后L或先L后F）

---

## ❓ 问题3：无线模块接收到的信息是通过串口方式传输到STM32的吗？

### 答案：是的！100%确定是UART串口

### E49-400T20S的数据流

```
遥控器 → 串口发送'F' → E49发射端 → 433MHz无线 
       → E49接收端 → 串口输出'F' → STM32 UART接收
```

**E49模块就是一个"无线串口"**：
- 把有线串口变成无线
- 对STM32来说，就像直接连了一根串口线
- 透传模式下，发送什么就接收什么

### 串口参数配置（关键！必须匹配）

从 `ex3.c` 的 `init_9600()` 函数可以看到：

```c
// 遥控器端串口配置（51单片机标准库）
TMOD = 0x20;      // 定时器1模式2
TH1 = 0xFD;       // 波特率9600
TL1 = 0xFD;
SCON = 0x50;      // 8位数据，无校验
```

**对应的STM32 HAL库配置**：

```c
UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    
    // 必须与遥控器完全一致！
    huart1.Init.BaudRate = 9600;              // ✅ 波特率9600
    huart1.Init.WordLength = UART_WORDLENGTH_8B;  // ✅ 8位数据
    huart1.Init.StopBits = UART_STOPBITS_1;   // ✅ 1位停止位
    huart1.Init.Parity = UART_PARITY_NONE;    // ✅ 无校验
    huart1.Init.Mode = UART_MODE_TX_RX;       // 收发都开
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // 无流控
    
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}
```

### GPIO引脚配置

STM32F103RC的USART1引脚：

```
PA9  → USART1_TX （发送）→ 连接E49的RXD
PA10 → USART1_RX （接收）→ 连接E49的TXD
```

**注意交叉连接**：
- STM32的TX连E49的RX
- STM32的RX连E49的TX

HAL库GPIO配置：

```c
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(uartHandle->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        // PA9: USART1_TX - 复用推挽输出
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        // PA10: USART1_RX - 浮空输入
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}
```

---

## 📋 完整配置检查清单

### 硬件连接检查

- [ ] E49模块供电正常（3.3V或5V，看模块规格）
- [ ] E49的TXD → STM32的PA10 (USART1_RX)
- [ ] E49的RXD → STM32的PA9 (USART1_TX)
- [ ] E49的GND → STM32的GND（共地！）
- [ ] E49的M0和M1都接GND（透传模式）

### 软件配置检查

- [ ] USART1波特率设置为9600
- [ ] 8位数据位，1位停止位，无校验
- [ ] PA9/PA10 GPIO配置正确
- [ ] 使能USART1中断（用于接收数据）

### 测试方法

**第一步：串口回环测试**

```c
uint8_t rxData;

while(1) {
    // 接收1字节
    if(HAL_UART_Receive(&huart1, &rxData, 1, 100) == HAL_OK) {
        // 立即回显（通过E49发回遥控器，或LED指示）
        HAL_UART_Transmit(&huart1, &rxData, 1, 10);
    }
}
```

**验证**：
- 按遥控器'F'键 → STM32应该收到 0x46
- LED闪烁或通过调试器看到rxData = 'F'

**第二步：按键功能测试**

```c
switch(rxData) {
    case 'F': 
        // LED1亮
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        break;
    case 'B':
        // LED2亮
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
        break;
    // ... 其他按键
}
```

---

## 🎯 总结答案

### 问题1：无线模块配置

**答**：E49模块的M0和M1引脚都设为0（接GND），进入透传模式。如果硬件已固定，无需软件配置；如果连GPIO，用代码拉低。

### 问题2：如何接收处理指令

**答**：遥控器按键会发送单个ASCII字符（'F'/'B'/'L'/'R'等），STM32通过串口接收这些字符，用switch-case判断并执行对应动作（如robot.drive()）。需要添加看门狗超时停车。

### 问题3：是串口传输吗？如何配置？

**答**：是的，100%是UART串口。配置参数必须为：
- 波特率：9600
- 数据位：8位
- 停止位：1位
- 校验位：无
- 引脚：PA9(TX)连E49_RX，PA10(RX)连E49_TX

---

## 🚀 下一步行动

现在理清楚了，可以开始写代码了：

1. **配置USART1**（9600, 8N1）
2. **配置E49模式引脚**（如果需要）
3. **实现串口接收中断**
4. **实现按键映射逻辑**
5. **添加看门狗超时保护**

准备好了吗？我可以开始写代码了！
