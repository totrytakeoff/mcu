# ESP-01S WiFi透传模块

## 项目说明

本项目将ESP-01S配置为WiFi透传模块，用于扩展STM32F103的无线通信功能。

**重要提示**：ESP8266（ESP-01S）不支持蓝牙功能，仅支持WiFi。如果需要蓝牙功能，请使用ESP32系列模块。

## 功能特性

- ✅ WiFi连接
- ✅ TCP服务器模式
- ✅ 双向数据透传（WiFi ↔ STM32）
- ✅ 串口调试输出
- ✅ 十六进制和ASCII数据显示
- ✅ 自动重连机制

## 硬件连接

### ESP-01S 引脚定义
```
     [ESP-01S]
     ┌─────────┐
  TX │1      8 │ VCC (3.3V)
  RX │2      7 │ GPIO2
CH_PD│3      6 │ GPIO0
 GND │4      5 │ RST
     └─────────┘
```

### 与STM32连接
```
ESP-01S          STM32F103
───────          ─────────
TX (GPIO1)   →   RX (PA10或其他UART RX)
RX (GPIO3)   ←   TX (PA9或其他UART TX)
VCC          →   3.3V
GND          →   GND
CH_PD        →   3.3V (使能芯片)
```

**注意事项**：
1. ESP-01S工作电压为3.3V，不能直接连接5V电源
2. ESP-01S的GPIO口电平为3.3V，STM32的5V容忍IO口可以直接连接
3. 如果STM32的TX是5V输出，建议加分压电阻保护ESP-01S的RX引脚
4. ESP-01S启动电流较大（峰值可达300mA），建议使用独立稳压模块供电

## 软件配置

### 1. 修改WiFi配置

编辑 `src/main.cpp` 文件，修改以下内容：

```cpp
const char* ssid = "YOUR_WIFI_SSID";        // 修改为你的WiFi名称
const char* password = "YOUR_WIFI_PASSWORD"; // 修改为你的WiFi密码
```

### 2. 修改串口波特率（可选）

如果STM32使用的波特率不是115200，请修改：

```cpp
#define STM32_SERIAL_BAUD 115200  // 修改为STM32的波特率
```

### 3. 修改TCP端口（可选）

默认使用8080端口，可以修改：

```cpp
const uint16_t serverPort = 8080;  // 修改为你想要的端口
```

### 4. 关闭调试输出（可选）

如果不需要调试信息，可以关闭：

```cpp
#define DEBUG_ENABLED false  // 改为false关闭调试输出
```

**注意**：关闭调试输出后，串口将完全用于与STM32通信，不会有任何额外的调试信息干扰。

## 编译和上传

### 使用PlatformIO

```bash
# 编译
pio run

# 上传到ESP-01S
pio run --target upload

# 查看串口输出
pio device monitor
```

### 上传模式

ESP-01S进入下载模式的方法：
1. GPIO0接GND
2. 重启ESP-01S（RST接GND后松开，或断电重启）
3. 上传程序
4. 上传完成后，GPIO0断开GND，重启设备

## 使用方法

### 1. 启动设备

上传程序后，ESP-01S会自动连接WiFi并启动TCP服务器。

### 2. 查看IP地址

通过串口监视器查看ESP-01S获取的IP地址：

```
=================================
ESP-01S WiFi透传模块启动
=================================
正在连接WiFi: YourWiFi
...
WiFi连接成功！
IP地址: 192.168.1.100
TCP服务器端口: 8080
=================================
等待客户端连接...
=================================
```

### 3. 连接测试

使用TCP客户端工具连接到ESP-01S：

**Windows - 使用PowerShell**：
```powershell
$client = New-Object System.Net.Sockets.TcpClient("192.168.1.100", 8080)
$stream = $client.GetStream()
$writer = New-Object System.IO.StreamWriter($stream)
$writer.WriteLine("Hello STM32")
$writer.Flush()
```

**Linux/Mac - 使用netcat**：
```bash
nc 192.168.1.100 8080
```

**Python测试脚本**：
```python
import socket

# 连接到ESP-01S
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(("192.168.1.100", 8080))

# 发送数据
client.send(b"Hello STM32\n")

# 接收响应
response = client.recv(1024)
print(f"收到: {response}")

client.close()
```

### 4. STM32端代码示例

```c
// STM32端串口初始化（以USART1为例）
void USART1_Init(void) {
    // 配置USART1: 115200, 8N1
    // PA9: TX, PA10: RX
    // ... 初始化代码 ...
}

// 接收ESP-01S转发的数据
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART1);
        // 处理接收到的数据
        // 这里的数据来自WiFi客户端
    }
}

// 发送数据到ESP-01S（转发给WiFi客户端）
void SendToWiFi(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        USART_SendData(USART1, data[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}
```

## 调试输出示例

当 `DEBUG_ENABLED` 为 `true` 时，串口会输出详细的调试信息：

```
[INFO] 新客户端已连接
[INFO] 客户端IP: 192.168.1.50
[WiFi->STM32] 接收 13 字节: 48 65 6C 6C 6F 20 53 54 4D 33 32 0D 0A | ASCII: Hello STM32..
[STM32->WiFi] 转发 5 字节: 4F 4B 0D 0A | ASCII: OK..
[INFO] 客户端已断开
```

## 如果需要蓝牙功能

ESP8266不支持蓝牙，如果需要蓝牙功能，建议使用以下方案：

### 方案1：使用ESP32模块（推荐）

ESP32同时支持WiFi和蓝牙（经典蓝牙和BLE），推荐使用：
- ESP32-WROOM-32
- ESP32-C3
- ESP32-S3

### 方案2：使用独立蓝牙模块

如HC-05、HC-06等蓝牙串口模块，直接连接STM32。

### 方案3：STM32直接支持蓝牙

使用带蓝牙功能的STM32，如STM32WB系列。

## 故障排除

### 1. WiFi连接失败
- 检查WiFi SSID和密码是否正确
- 确认WiFi信号强度足够
- 检查路由器是否开启了MAC地址过滤

### 2. 无法上传程序
- 确认GPIO0已接GND
- 检查串口连接是否正确
- 尝试降低上传波特率

### 3. 串口通信异常
- 检查波特率设置是否一致
- 确认TX/RX是否交叉连接
- 检查电平是否匹配（3.3V/5V）

### 4. 设备频繁重启
- 检查供电是否稳定
- ESP-01S需要足够的电流（峰值300mA）
- 建议使用独立的3.3V稳压模块

## 技术参数

- **芯片**：ESP8266
- **工作电压**：3.3V
- **工作电流**：平均80mA，峰值300mA
- **WiFi标准**：802.11 b/g/n
- **频率范围**：2.4GHz
- **串口波特率**：可配置（默认115200）
- **TCP连接**：支持1个客户端同时连接

## 许可证

MIT License
