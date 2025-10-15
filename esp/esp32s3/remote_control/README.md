# ESP32-S3 蓝牙通讯项目

这是一个基于 ESP32-S3 的蓝牙低功耗（BLE）通讯项目集合，包含多个应用场景。

## 📁 项目内容

### 1. 🎯 **BLE 手机通讯 Demo**（推荐入门）

一个简单易用的 BLE UART 通讯示例，可以直接与手机蓝牙 APP 进行双向通讯。

**适用场景**：
- 学习 ESP32-S3 BLE 开发
- 快速测试蓝牙功能
- 开发手机控制的物联网设备

**快速开始**：
```bash
# 编译并上传
pio run -t upload

# 查看串口输出
pio device monitor
```

**详细文档**：[BLE Demo 使用指南](docs/BLE_DEMO_GUIDE.md)

### 2. 🔄 **BLE-UART 透传模块**（已集成）

手机 → BLE → ESP32-S3 → UART(Serial1) → STM32。当前 demo 已实现：

- 手机发什么，BLE 原样回显给手机
- 同时原始字节通过 `Serial1` 转发给 STM32（默认波特率 115200）
- 心跳包：每 10s 向手机发送 `HEARTBEAT <seconds>s`，不影响透传

硬件连接（默认引脚）：

| ESP32-S3 | STM32F103 | 说明 |
|---|---|---|
| GPIO17 (TX) | PA2 (RX) | ESP32 → STM32 |
| GPIO18 (RX) | PA3 (TX) | STM32 → ESP32（当前未使用） |

如需修改波特率/引脚，编辑 `src/main.cpp` 顶部：

```cpp
#define STM32_UART_BAUD 115200
#define STM32_UART_TX_PIN 17
#define STM32_UART_RX_PIN 18
```

**适用场景**：
- 为不支持蓝牙的 MCU 添加蓝牙功能
- 机器人/小车蓝牙遥控
- 蓝牙数据透传

**详细说明**：见下文完整文档

---

## 功能特性

### 基础功能
- ✅ BLE蓝牙服务端（使用Nordic UART Service标准）
- ✅ 与手机直接通讯
- ✅ 双向数据传输
- ✅ 连接状态监控

### 高级功能（透传模式）
- ✅ 蓝牙数据自动转发到STM32（通过UART）
- ✅ STM32数据自动转发到蓝牙设备
- ✅ 支持双向透传通信

## 硬件连接

### ESP32-S3 与 STM32F103 连接通过usart2

| ESP32-S3    | STM32F103 | 说明                                  |
| ----------- | --------- | ------------------------------------- |
| GPIO17 (TX) | PA2 (RX) | ESP32发送 → STM32接收                |
| GPIO18 (RX) | PA3 (TX)  | ESP32接收 ← STM32发送                |
| GND         | GND       | 共地                                  |
| 3.3V        | 3.3V      | 电源（可选，如果ESP32独立供电则不连） |

**注意事项：**

- 确保两个设备共地（GND连接）
- ESP32-S3和STM32F103都是3.3V逻辑电平，可以直接连接
- 默认波特率：115200

## 软件配置

### ESP32-S3 端（本项目）

1. **安装PlatformIO**

   ```bash
   # 使用VSCode安装PlatformIO插件
   ```
2. **连接硬件**

   - 使用USB数据线连接ESP32-S3到电脑
3. **编译和上传**

   ```bash
   # 编译项目
   pio run

   # 上传到ESP32-C3
   pio run --target upload

   # 查看串口监视器
   pio device monitor

   # 一条命令完成（推荐）
   pio run -t upload && pio device monitor
   ```
4. **烧录故障排除**

   ```bash
   # 如果自动烧录失败，尝试降低速率
   pio run --target upload --upload-speed 115200

   # 手动指定端口（Windows示例）
   pio run --target upload --upload-port COM3

   # 查看可用端口
   pio device list
   ```
5. **配置说明**

   - 蓝牙设备名称：`ESP32-S3-UART`
   - BLE服务UUID：`6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
   - UART波特率：115200
   - TX引脚：GPIO17
   - RX引脚：GPIO18

### STM32F103 端配置

在STM32端，你需要配置串口（通常是USART1）来接收ESP32转发的数据：

```c
// STM32 USART1配置示例（使用HAL库）
void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
}

// 接收数据示例
uint8_t rxBuffer[256];
HAL_UART_Receive(&huart1, rxBuffer, sizeof(rxBuffer), HAL_MAX_DELAY);

// 发送数据示例（会自动转发到蓝牙）
uint8_t txData[] = "Hello from STM32";
HAL_UART_Transmit(&huart1, txData, sizeof(txData), HAL_MAX_DELAY);
```

## 蓝牙遥控器通信协议

本项目支持高效的蓝牙遥控器协议，适用于小车、机器人等实时控制应用。

### **1. 按键模式**

**格式**：单字母命令

- `F` = 前进
- `B` = 后退 (Backward)
- `L` = 左转 (Left)
- `R` = 右转 (Right)
- `U` = 向上
- `D` = 向下
- `S` = 停止 (Stop)

**规则**：

- 按住按键时每100ms重复发送
- 松开按键停止发送
- 数据量：1字节/命令
- 延迟：<100ms

**示例**：

```c
// STM32接收处理
if (rxData == 'F') {
    motor_forward();
} else if (rxData == 'B') {
    motor_backward();
} else if (rxData == 'S') {
    motor_stop();
}
```

### **2. 摇杆模式**

**格式**：`A[角度000-359]P[力度00-99]\n`

**参数说明**：

- **角度**：000-359度（3位数字，前导0补齐）
  - 000° = 正右方向
  - 090° = 正上方向
  - 180° = 正左方向
  - 270° = 正下方向
- **力度**：00-99（2位数字，前导0补齐）
  - 00 = 停止
  - 50 = 50%力度
  - 99 = 最大力度

**示例**：

- `A090P50\n` = 正上方向，50%力度
- `A000P99\n` = 正右方向，99%力度
- `A180P30\n` = 正左方向，30%力度

**停止命令**：

- `S\n` = 摇杆回中时发送

**频率**：

- 摇杆移动时每50ms发送一次
- 数据量：8字节/命令
- 延迟：<50ms

**解析示例**：

```c
// STM32解析摇杆数据
void parseJoystick(char* data) {
    if (data[0] == 'A') {
        // 解析角度：A后面3位
        int angle = (data[1]-'0')*100 + (data[2]-'0')*10 + (data[3]-'0');
      
        // 解析力度：P后面2位
        int power = (data[5]-'0')*10 + (data[6]-'0');
      
        // 控制电机
        motor_control(angle, power);
    } else if (data[0] == 'S') {
        motor_stop();
    }
}
```

### **3. 协议性能优势**

| 特性     | 按键模式 | 摇杆模式 |
| -------- | -------- | -------- |
| 数据量   | 1字节    | 8字节    |
| 发送频率 | 100ms    | 50ms     |
| 延迟     | <100ms   | <50ms    |
| 适用场景 | 简单控制 | 精确控制 |

### **4. 调试建议**

**串口监视器设置**：

- 波特率：115200（ESP32-S3端）
- 显示模式：HEX + ASCII
- 换行符：CR+LF

**常见问题**：

- **数据乱码**：检查波特率是否匹配（115200）
- **响应延迟**：确认发送频率和缓冲区大小
- **丢包**：降低发送频率或增大接收缓冲区

### **5. 完整STM32示例**

```c
// 接收缓冲区
uint8_t rxBuffer[32];
uint8_t rxIndex = 0;

// 串口接收中断
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uint8_t data = rxBuffer[rxIndex];
      
        // 按键模式
        if (data >= 'A' && data <= 'Z') {
            switch(data) {
                case 'F': motor_forward(); break;
                case 'B': motor_backward(); break;
                case 'L': motor_turn_left(); break;
                case 'R': motor_turn_right(); break;
                case 'S': motor_stop(); break;
            }
            rxIndex = 0;
        }
        // 摇杆模式（以A开头）
        else if (data == 'A') {
            rxIndex = 0;
            rxBuffer[rxIndex++] = data;
        }
        // 继续接收摇杆数据
        else if (rxIndex > 0 && rxIndex < 8) {
            rxBuffer[rxIndex++] = data;
          
            // 接收完整（A090P50\n = 8字节）
            if (data == '\n') {
                int angle = (rxBuffer[1]-'0')*100 + 
                           (rxBuffer[2]-'0')*10 + 
                           (rxBuffer[3]-'0');
                int power = (rxBuffer[5]-'0')*10 + 
                           (rxBuffer[6]-'0');
              
                motor_control(angle, power);
                rxIndex = 0;
            }
        }
      
        // 继续接收
        HAL_UART_Receive_IT(&huart1, &rxBuffer[rxIndex], 1);
    }
}
```

## 使用方法

### 1. 启动系统

1. 给ESP32-S3和STM32供电
2. ESP32-S3会自动启动BLE广播
3. 串口监视器会显示：
   ```
   ESP32-S3 BLE-UART Bridge Starting...
   UART initialized for STM32 communication
   BLE initialized and advertising
   BLE-UART Bridge Ready!
   BLE Device Name: ESP32-S3-UART
   ```

### 2. 连接蓝牙设备

**使用手机APP：**

- Android: 推荐使用 "Serial Bluetooth Terminal" 或 "nRF Connect"
- iOS: 推荐使用 "LightBlue" 或 "nRF Connect"

**连接步骤：**

1. 打开蓝牙APP，搜索设备
2. 找到 `ESP32-S3-UART` 设备并连接
3. 在nRF Connect中，找到Nordic UART Service
4. 启用TX特征的通知功能
5. 通过RX特征发送数据

**测试遥控器协议：**

- 按键测试：发送 `F`（前进）、`B`（后退）、`S`（停止）
- 摇杆测试：发送 `A090P50\n`（向上50%力度）

### 3. 数据流向

```
手机APP → BLE → ESP32-S3 → UART → STM32
STM32 → UART → ESP32-S3 → BLE → 手机APP
```

### 4. 调试信息

通过USB连接ESP32-S3到电脑，使用串口监视器（115200波特率）可以看到：

- 连接状态
- 数据传输方向和字节数
- 数据内容（十六进制格式）

示例输出：

```
BLE device connected
BLE->STM32: 5 bytes
Data: 48 65 6C 6C 6F 
STM32->BLE: 8 bytes
```

## 通信协议

### BLE特征说明

- **TX特征** (UUID: 6E400003-...): ESP32 → 手机（通知）
- **RX特征** (UUID: 6E400002-...): 手机 → ESP32（写入）

### 数据格式

- 支持任意二进制数据
- 单次传输最大512字节
- 超过512字节会自动分包

## 自定义配置

如果需要修改配置，编辑 `include/ble_uart_bridge.h`：

```cpp
// 修改蓝牙设备名称
#define BLE_DEVICE_NAME "你的设备名"

// 修改UART引脚
#define UART_TX_PIN 21  // 改为你的TX引脚
#define UART_RX_PIN 20  // 改为你的RX引脚

// 修改波特率
#define UART_BAUD_RATE 115200  // 改为你需要的波特率
```

## 故障排除

### 1. 无法找到蓝牙设备

- 确认ESP32-C3已上电并正常运行
- 检查串口监视器是否显示 "BLE initialized and advertising"
- 尝试重启ESP32-C3
- 确认手机蓝牙已开启

### 2. 连接后立即断开

- 检查电源是否稳定
- 尝试增加延迟时间
- 检查是否有其他设备同时连接

### 3. STM32收不到数据

- 检查硬件连接（TX-RX交叉连接）
- 确认GND已连接
- 检查波特率是否匹配（115200）
- 使用逻辑分析仪或示波器检查信号

### 4. 数据传输不完整

- 检查STM32的接收缓冲区大小
- 降低数据发送速率
- 增加接收处理的优先级

## 性能参数

- **BLE传输速率**: 约20-30KB/s
- **UART波特率**: 115200 (约14KB/s)
- **延迟**: <50ms
- **最大连接距离**: 约10米（空旷环境）

## 扩展功能

可以在此基础上添加：

- AT命令解析
- 数据加密
- 多设备连接
- 低功耗模式
- OTA固件升级

## 许可证

MIT License

## 技术支持

如有问题，请检查：

1. 硬件连接是否正确
2. 串口监视器的调试信息
3. STM32端的串口配置

## 更新日志

### v1.0.0 (2025-10-11)

- 初始版本
- 实现BLE-UART双向透传
- 支持Nordic UART Service标准协议
