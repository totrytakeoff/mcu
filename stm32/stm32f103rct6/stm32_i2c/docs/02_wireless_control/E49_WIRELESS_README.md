# E49_Wireless 类使用说明

## 📚 概述

`E49_Wireless` 是 E49-400T20S 无线模块的 C++ 封装类，提供简洁的接口用于：
- GPIO 配置和模式控制
- 数据收发
- 状态检查
- 回调机制

---

## 🔌 硬件连接

```
E49-400T20S → STM32F103RC
━━━━━━━━━━━━━━━━━━━━━━
M0   → PA6  (模式控制)
M1   → PA7  (模式控制)
AUX  → PA12 (状态指示)
TXD  → PA10 (USART1_RX)
RXD  → PA9  (USART1_TX)
VCC  → 3.3V
GND  → GND
```

---

## 💻 基本使用

### 1. 创建对象并初始化

```cpp
#include "e49_wireless.hpp"

E49_Wireless wireless;

void setup() {
    // 初始化 USART（必须先初始化）
    MX_USART1_UART_Init();
    
    // 初始化 E49 模块
    wireless.init();
    
    // 检查是否就绪
    if (wireless.isReady()) {
        // 模块已就绪
    }
}
```

### 2. 发送数据

```cpp
// 发送单字节
wireless.send('A');

// 发送字符串
wireless.sendString("Hello World\r\n");

// 发送字节数组
uint8_t data[] = {0x01, 0x02, 0x03};
wireless.send(data, 3);
```

### 3. 接收数据（回调方式）

```cpp
// 注册回调函数
wireless.setDataReceivedCallback([](uint8_t data) {
    // 处理接收到的数据
    processData(data);
});

// 在 UART 中断中调用
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        wireless.onDataReceived(rxByte);
        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}
```

### 4. 模式切换

```cpp
// 设置为透传模式（默认）
wireless.setMode(E49_Wireless::Mode::Transparent);

// 设置为配置模式（用于 AT 命令）
wireless.setMode(E49_Wireless::Mode::Config);

// 设置为省电模式
wireless.setMode(E49_Wireless::Mode::PowerSave);

// 获取当前模式
auto mode = wireless.getMode();
```

---

## 🎯 工作模式说明

| 模式 | M1 | M0 | 说明 | 用途 |
|------|----|----|------|------|
| Transparent | 0 | 0 | 透传模式 | 正常通信 |
| Wakeup | 0 | 1 | 唤醒模式 | 低功耗唤醒 |
| PowerSave | 1 | 0 | 省电模式 | 待机省电 |
| Config | 1 | 1 | 配置模式 | AT 命令配置 |

---

## 🔄 完整示例

```cpp
#include "e49_wireless.hpp"
#include "usart.h"

E49_Wireless wireless;
uint8_t rxByte;

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        wireless.onDataReceived(rxByte);
        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}

extern "C" int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    // 初始化 E49
    wireless.init();
    
    // 注册回调
    wireless.setDataReceivedCallback([](uint8_t data) {
        // 回显数据
        wireless.send(data);
    });
    
    // 启动接收
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    
    // 发送就绪消息
    wireless.sendString("Ready!\r\n");
    
    while(1) {
        HAL_Delay(10);
    }
}
```

---

## 📋 API 参考

### 构造/析构
```cpp
E49_Wireless();   // 构造函数
~E49_Wireless();  // 析构函数
```

### 初始化
```cpp
void init();                              // 初始化模块
bool waitReady(uint32_t timeout_ms);      // 等待就绪（阻塞）
bool isReady() const;                     // 检查是否就绪
```

### 模式控制
```cpp
void setMode(Mode mode);      // 设置工作模式
Mode getMode() const;         // 获取当前模式
```

### 数据发送
```cpp
void send(uint8_t data);                          // 发送单字节
void send(const uint8_t* data, uint16_t length);  // 发送多字节
void sendString(const char* str);                 // 发送字符串
```

### 数据接收
```cpp
void setDataReceivedCallback(std::function<void(uint8_t)> callback);  // 设置回调
void onDataReceived(uint8_t data);  // 接收处理（中断中调用）
```

---

## ⚠️ 注意事项

1. **必须先初始化 USART**
   ```cpp
   MX_USART1_UART_Init();  // 必须在 wireless.init() 之前调用
   wireless.init();
   ```

2. **中断接收必须启动**
   ```cpp
   HAL_UART_Receive_IT(&huart1, &rxByte, 1);
   ```

3. **在中断中调用 onDataReceived**
   ```cpp
   extern "C" void HAL_UART_RxCpltCallback(...) {
       wireless.onDataReceived(rxByte);
       HAL_UART_Receive_IT(&huart1, &rxByte, 1);  // 继续接收
   }
   ```

4. **模式切换需要时间**
   - 切换模式后会自动延时 10ms
   - 可以用 `waitReady()` 等待模块稳定

5. **AUX 引脚状态**
   - 高电平：模块就绪，可以通信
   - 低电平：模块忙碌，正在处理数据

---

## 🧪 测试方法

### 测试 1：回环测试
```cpp
// 发送什么就收到什么
wireless.setDataReceivedCallback([](uint8_t data) {
    wireless.send(data);  // 回显
});
```

### 测试 2：LED 指示
```cpp
wireless.setDataReceivedCallback([](uint8_t data) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // LED 闪烁
});
```

### 测试 3：无线通信
- 两块板，板 A 发送，板 B 应该收到
- 验证无线收发正常

---

## 📁 相关文件

- `include/e49_wireless.hpp` - 类声明
- `src/e49_wireless.cpp` - 类实现
- `examples/e49_test.cpp` - 测试示例
- `include/usart.h` - USART 配置（依赖）
- `src/usart.c` - USART 实现

---

## 🚀 下一步

E49_Wireless 类已经完成，接下来可以：
1. ✅ 基于此类实现 `RemoteControl` 类
2. ✅ 集成差速转向控制
3. ✅ 实现完整的遥控功能

**E49 模块封装完成！** 🎉
