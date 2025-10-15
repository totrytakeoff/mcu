# 通信协议说明

## 概述

本文档描述ESP32-C3蓝牙透传模块的通信协议和数据格式。

## 数据流向

```
手机APP ←→ BLE ←→ ESP32-C3 ←→ UART ←→ STM32F103
```

## BLE协议

### 服务UUID

使用标准的Nordic UART Service (NUS)：

- **Service UUID**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- **RX Characteristic UUID**: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` (Write)
- **TX Characteristic UUID**: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` (Notify)

### 特征说明

#### RX特征（接收）
- **方向**: 手机 → ESP32
- **属性**: Write, Write Without Response
- **最大长度**: 512 字节
- **用途**: 接收手机发送的命令和数据

#### TX特征（发送）
- **方向**: ESP32 → 手机
- **属性**: Notify
- **最大长度**: 512 字节
- **用途**: 发送STM32的响应数据到手机

## UART协议

### 基本参数

- **波特率**: 115200 bps（可配置）
- **数据位**: 8
- **停止位**: 1
- **校验位**: None
- **流控**: None（可选RTS/CTS）

### 数据格式

UART传输的是原始字节流，没有额外的封装。所有从BLE接收的数据会直接转发到UART，反之亦然。

## 推荐的应用层协议

虽然底层是透传，但建议在应用层实现结构化协议：

### 方案1: 简单命令协议

**格式**: `命令字节 + 参数 + 结束符`

```
命令格式: [CMD][PARAM1][PARAM2]...[PARAM_N]\n

示例:
F100\n     - 前进，速度100
B050\n     - 后退，速度50
L\n        - 左转
R\n        - 右转
S\n        - 停止
```

**优点**: 简单易实现，人类可读
**缺点**: 效率较低，不适合高频数据

### 方案2: 二进制协议

**格式**: `帧头 + 长度 + 命令 + 数据 + 校验`

```c
typedef struct {
    uint8_t header[2];    // 帧头: 0xAA 0x55
    uint8_t length;       // 数据长度
    uint8_t cmd;          // 命令字
    uint8_t data[256];    // 数据
    uint8_t checksum;     // 校验和
} __attribute__((packed)) Protocol_Frame_t;
```

**示例帧**:
```
AA 55 03 01 64 00 C2
│  │  │  │  │  │  └─ 校验和
│  │  │  │  │  └──── 数据2
│  │  │  │  └─────── 数据1 (速度=100)
│  │  │  └────────── 命令 (01=前进)
│  │  └───────────── 长度 (3字节)
│  └──────────────── 帧头
└─────────────────── 帧头
```

**优点**: 高效，可靠，支持复杂数据
**缺点**: 实现复杂，调试困难

### 方案3: JSON协议

**格式**: JSON字符串

```json
{
  "cmd": "move",
  "direction": "forward",
  "speed": 100
}
```

**优点**: 灵活，易扩展，易调试
**缺点**: 数据量大，解析开销大

## 命令示例

### 电机控制命令

```c
// 命令定义
#define CMD_FORWARD     0x01    // 前进
#define CMD_BACKWARD    0x02    // 后退
#define CMD_TURN_LEFT   0x03    // 左转
#define CMD_TURN_RIGHT  0x04    // 右转
#define CMD_STOP        0x05    // 停止
#define CMD_SET_SPEED   0x06    // 设置速度

// 发送示例（手机端）
// 前进，速度100
uint8_t cmd[] = {0x01, 0x64};  // CMD_FORWARD, speed=100
ble_write(cmd, 2);

// 接收示例（STM32端）
void processCommand(uint8_t* data, uint16_t len) {
    if (len < 1) return;
    
    switch (data[0]) {
        case CMD_FORWARD:
            if (len >= 2) {
                motor_forward(data[1]);  // data[1] = speed
            }
            break;
        case CMD_STOP:
            motor_stop();
            break;
        // ...
    }
}
```

### 传感器数据上报

```c
// 数据格式
typedef struct {
    uint8_t type;           // 数据类型
    uint32_t timestamp;     // 时间戳
    float value;            // 数值
} __attribute__((packed)) SensorData_t;

// 发送示例（STM32端）
void sendSensorData(uint8_t type, float value) {
    SensorData_t data;
    data.type = type;
    data.timestamp = HAL_GetTick();
    data.value = value;
    
    HAL_UART_Transmit(&huart1, (uint8_t*)&data, sizeof(data), 100);
}

// 接收示例（手机端）
// 解析二进制数据
```

## 错误处理

### 超时机制

```c
// STM32端实现超时检测
#define COMM_TIMEOUT_MS 1000

uint32_t lastReceiveTime = 0;

void checkTimeout() {
    if (HAL_GetTick() - lastReceiveTime > COMM_TIMEOUT_MS) {
        // 超时，执行安全操作
        motor_stop();
        // 可选：发送心跳请求
    }
}
```

### 心跳包

```c
// 定期发送心跳包
#define CMD_HEARTBEAT 0xFF

void sendHeartbeat() {
    uint8_t cmd = CMD_HEARTBEAT;
    HAL_UART_Transmit(&huart1, &cmd, 1, 100);
}

// 在主循环中每秒发送一次
```

### 数据校验

```c
// 简单校验和
uint8_t calculateChecksum(uint8_t* data, uint16_t len) {
    uint8_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// CRC16校验（更可靠）
uint16_t crc16(uint8_t* data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}
```

## 性能优化

### 批量发送

```c
// 不好的做法：逐字节发送
for (int i = 0; i < 100; i++) {
    HAL_UART_Transmit(&huart1, &data[i], 1, 100);
}

// 好的做法：批量发送
HAL_UART_Transmit(&huart1, data, 100, 100);
```

### 使用DMA

```c
// 使用DMA可以大幅提升性能，释放CPU
HAL_UART_Transmit_DMA(&huart1, data, length);
HAL_UART_Receive_DMA(&huart1, rxBuffer, RX_BUFFER_SIZE);
```

### 环形缓冲区

```c
// 实现环形缓冲区避免数据丢失
typedef struct {
    uint8_t buffer[1024];
    uint16_t head;
    uint16_t tail;
} RingBuffer_t;

void ringBufferPut(RingBuffer_t* rb, uint8_t data) {
    uint16_t next = (rb->head + 1) % 1024;
    if (next != rb->tail) {
        rb->buffer[rb->head] = data;
        rb->head = next;
    }
}

uint8_t ringBufferGet(RingBuffer_t* rb) {
    if (rb->head == rb->tail) return 0;
    uint8_t data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % 1024;
    return data;
}
```

## 调试技巧

### 数据包打印

```c
// 打印十六进制数据
void printHex(uint8_t* data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}
```

### 使用串口监视器

ESP32-C3会在USB串口输出调试信息：
```
BLE->STM32: 5 bytes
Data: 01 64 00 00 00
STM32->BLE: 3 bytes
Data: 02 01 00
```

### 使用逻辑分析仪

推荐使用逻辑分析仪监控UART通信：
- 可以准确看到每个字节
- 可以测量时序
- 可以检测通信错误

## 安全建议

1. **输入验证**: 始终验证接收到的数据
2. **边界检查**: 防止缓冲区溢出
3. **超时保护**: 实现看门狗和超时机制
4. **安全模式**: 通信异常时进入安全状态
5. **加密**: 如果传输敏感数据，考虑加密

## 扩展功能

### 固件升级（OTA）

可以通过蓝牙实现STM32固件升级：
1. 手机发送固件数据到ESP32
2. ESP32转发到STM32
3. STM32写入Flash
4. 完成后重启

### 多设备通信

如果需要连接多个从设备：
1. 使用设备地址或ID
2. 在协议中添加设备字段
3. ESP32根据ID路由数据

### 数据记录

ESP32-C3可以记录通信数据到Flash：
```cpp
#include <SPIFFS.h>

void logData(uint8_t* data, size_t len) {
    File file = SPIFFS.open("/log.bin", FILE_APPEND);
    file.write(data, len);
    file.close();
}
```
