# ESP32-C3 测试 Demo

这是一个基于ESP32-C3的开发板测试程序，演示了ESP32-C3的基本功能。

## 功能特性

1. **LED闪烁测试** - 板载LED每秒闪烁一次
2. **串口输出** - 通过串口输出系统信息和状态
3. **WiFi连接** - 连接到指定的WiFi网络
4. **Web服务器** - 提供简单的Web界面控制LED和查看系统状态

## 硬件要求

- ESP32-C3 DevKitM-1开发板
- USB数据线
- WiFi网络

## 软件要求

- PlatformIO IDE
- Arduino框架支持

## 使用方法

### 1. 配置WiFi

打开 `src/main.cpp` 文件，修改以下WiFi配置：

```cpp
const char* ssid = "YourWiFiSSID";        // 替换为你的WiFi名称
const char* password = "YourWiFiPassword";  // 替换为你的WiFi密码
```

### 2. 编译和上传

1. 使用PlatformIO打开项目
2. 点击"Build"按钮编译代码
3. 点击"Upload"按钮将代码上传到ESP32-C3开发板

### 3. 测试功能

#### 串口监视

- 打开串口监视器（波特率：115200）
- 观察启动信息和系统状态输出

#### Web界面

1. 连接WiFi后，串口会显示ESP32-C3的IP地址
2. 在浏览器中输入该IP地址
3. 可以通过网页按钮控制LED开关
4. 查看系统状态信息

## 代码结构

- `main.cpp` - 主程序文件，包含所有功能实现
- `platformio.ini` - 项目配置文件

## 功能说明

### LED控制

- 板载LED连接到GPIO8
- 自动闪烁（1秒间隔）
- 可通过Web界面控制

### WiFi功能

- 自动连接到配置的WiFi网络
- 提供IP地址显示
- 创建Web服务器

### Web界面

- 显示系统状态（IP地址、WiFi信号、运行时间、内存使用）
- LED控制按钮（开启、关闭、切换）
- 响应式设计，适配移动设备

## 故障排除

### 1. WiFi连接失败

- 检查WiFi名称和密码是否正确
- 确保路由器支持2.4GHz频段
- 检查ESP32-C3是否在路由器的有效范围内

### 2. 串口无输出

- 检查串口监视器波特率是否设置为115200
- 确认USB连接正常
- 尝试重置开发板

### 3. Web页面无法访问

- 确保WiFi连接成功
- 检查防火墙设置
- 尝试使用IP地址直接访问

## 开发板信息

- **芯片**: ESP32-C3 RISC-V处理器
- **频率**: up to 160MHz
- **Flash**: 4MB (可配置)
- **RAM**: 320KB SRAM
- **WiFi**: 802.11 b/g/n
- **蓝牙**: Bluetooth 5.0 LE

## 许可证

本项目基于MIT许可证开源。
