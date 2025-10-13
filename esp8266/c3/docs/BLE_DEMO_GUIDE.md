# ESP32-S3 BLE 蓝牙通讯 Demo 使用指南

## 📋 项目简介

这是一个基于 ESP32-S3-N16R8 开发板的蓝牙低功耗（BLE）UART 通讯示例程序。它实现了一个标准的 Nordic UART Service (NUS)，可以与各种手机蓝牙串口 APP 进行双向通讯。

## 🔧 硬件要求

- **开发板**: ESP32-S3-N16R8 (16MB Flash, 8MB PSRAM)
- **USB 数据线**: Type-C 接口
- **手机**: 支持蓝牙 4.0+ 的 Android 或 iOS 设备

## 📱 支持的手机 APP

### Android 推荐：
1. **Serial Bluetooth Terminal** (推荐)
   - Google Play 下载
   - 简单易用，支持 BLE UART

2. **nRF Connect**
   - Nordic 官方 APP
   - 功能强大，适合调试

3. **LightBlue**
   - 简洁的 BLE 扫描工具

### iOS 推荐：
1. **nRF Connect**
2. **LightBlue**
3. **BLE Terminal**

## 🚀 快速开始

### 1. 编译和上传

```bash
# 使用 PlatformIO CLI
pio run -t upload

# 或使用 VSCode PlatformIO 插件
# 点击底部工具栏的 "Upload" 按钮
```

### 2. 查看串口输出

```bash
# 使用 PlatformIO 串口监视器
pio device monitor

# 或在 VSCode 中点击 "Serial Monitor" 按钮
```

你应该看到类似输出：

```
╔════════════════════════════════════════╗
║   ESP32-S3 BLE UART 通讯示例          ║
╚════════════════════════════════════════╝

🔧 正在初始化 BLE...
✅ BLE 初始化完成！
🔍 设备名称: ESP32-S3-BLE
📡 等待手机连接...
```

### 3. 手机连接步骤

#### 使用 Serial Bluetooth Terminal (Android):

1. 打开 APP，点击右上角的搜索图标
2. 在设备列表中找到 **ESP32-S3-BLE**
3. 点击连接
4. 连接成功后，串口监视器会显示：
   ```
   📱 设备已连接
   📤 已发送欢迎消息
   ```
5. 在 APP 中输入文本并发送
6. ESP32 会接收数据并回显

#### 使用 nRF Connect:

1. 打开 APP，扫描设备
2. 找到 **ESP32-S3-BLE**，点击 CONNECT
3. 展开 "Nordic UART Service"
4. 找到 RX 特征（带箭头向下图标）
5. 点击向上箭头按钮发送数据
6. 在 TX 特征（带通知图标）上启用通知以接收数据

## 💡 功能特性

### ✅ 已实现功能

- [x] BLE UART 服务 (Nordic UART Service 标准)
- [x] 接收手机发送的数据
- [x] 向手机发送数据
- [x] 数据回显功能
- [x] 连接状态监控
- [x] 自动重连机制
- [x] 心跳数据发送（每10秒）
- [x] 欢迎消息发送

### 📊 技术参数

- **BLE 版本**: Bluetooth 5.0
- **服务 UUID**: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
- **RX UUID**: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
- **TX UUID**: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E
- **最大数据长度**: 512 字节/次
- **串口波特率**: 115200

## 🔨 自定义开发

### 修改设备名称

在 `main.cpp` 中修改：

```cpp
#define DEVICE_NAME "你的设备名称"
```

### 修改心跳间隔

在 `loop()` 函数中修改：

```cpp
if (deviceConnected && (millis() - lastHeartbeat > 10000)) {  // 改为你想要的毫秒数
    // ...
}
```

### 添加自定义数据处理

在 `RxCallbacks::onWrite()` 方法中添加你的逻辑：

```cpp
void onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    
    // 你的自定义处理
    if (rxValue == "LED_ON") {
        // 打开 LED
    } else if (rxValue == "LED_OFF") {
        // 关闭 LED
    }
}
```

## 📝 测试示例

### 1. 基本通讯测试

**发送**: `Hello ESP32`  
**接收**: `ESP32收到: Hello ESP32`

### 2. 命令控制测试

可以扩展代码实现命令解析，例如：

```
发送: LED:ON
发送: LED:OFF
发送: STATUS
发送: RESET
```

### 3. 性能测试

发送长文本测试数据传输：

```
abcdefghijklmnopqrstuvwxyz0123456789...
```

## 🐛 常见问题

### 1. 找不到设备

**解决方案**：
- 确保 ESP32 已正确上电并运行
- 检查串口输出是否显示 "等待手机连接"
- 手机蓝牙是否已打开
- 尝试重启 ESP32 和手机蓝牙

### 2. 连接后立即断开

**解决方案**：
- 检查手机 APP 是否支持 BLE UART
- 尝试使用 nRF Connect 测试
- 查看串口输出的错误信息

### 3. 接收不到数据

**解决方案**：
- 确认已启用 TX 特征的通知功能
- 检查数据格式是否正确
- 查看串口监视器确认 ESP32 是否发送了数据

### 4. 编译错误

**解决方案**：
- 确保 PlatformIO 已安装 ESP32 平台
- 检查 `platformio.ini` 配置是否正确
- 清理并重新编译：`pio run -t clean && pio run`

## 📚 参考资料

- [ESP32-S3 技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_cn.pdf)
- [NimBLE-Arduino 文档](https://github.com/h2zero/NimBLE-Arduino)
- [Nordic UART Service 规范](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/bluetooth_services/services/nus.html)

## 📄 许可证

本项目采用 MIT 许可证。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

**祝你使用愉快！** 🎉
