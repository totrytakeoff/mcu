# ESP32-S3 BLE 快速上手指南

## ⚡ 5分钟快速开始

### 第一步：确认硬件

确保你有：
- ✅ ESP32-S3-N16R8 开发板
- ✅ USB Type-C 数据线
- ✅ 安装了蓝牙的手机

### 第二步：上传代码

```bash
# 在项目根目录执行
pio run -t upload && pio device monitor
```

如果遇到端口问题，先查看可用端口：
```bash
pio device list
```

然后指定端口上传（Windows 示例）：
```bash
pio run -t upload --upload-port COM3
```

### 第三步：查看串口输出

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

### 第四步：连接手机

#### Android 用户（推荐 Serial Bluetooth Terminal）:

1. 在 Google Play 下载 "Serial Bluetooth Terminal"
2. 打开 APP，点击右上角的 🔍 搜索图标
3. 找到 **ESP32-S3-BLE** 并点击连接
4. 连接成功！

#### iOS 用户（推荐 nRF Connect）:

1. 在 App Store 下载 "nRF Connect for Mobile"
2. 打开 APP，点击 SCAN
3. 找到 **ESP32-S3-BLE**，点击 CONNECT
4. 展开 "Nordic UART Service"
5. 点击 TX 特征的 📥 图标启用通知
6. 点击 RX 特征的 ↑ 图标发送数据

### 第五步：测试通讯

在手机 APP 中发送：
```
Hello ESP32
```

你会看到：

**串口监视器输出**：
```
📥 接收到数据: Hello ESP32
📤 已回复: ESP32收到: Hello ESP32
```

**手机收到**：
```
ESP32收到: Hello ESP32
```

## 🎉 成功！

你已经成功建立了 ESP32-S3 与手机的蓝牙通讯！

## 🔥 下一步

### 1. 修改设备名称

编辑 `src/main.cpp`，找到：
```cpp
#define DEVICE_NAME "ESP32-S3-BLE"
```
改为你想要的名称，然后重新上传。

### 2. 添加 LED 控制

在 `RxCallbacks::onWrite()` 中添加：

```cpp
void onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    
    // LED 控制
    if (rxValue == "LED_ON") {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("💡 LED 已打开");
    } else if (rxValue == "LED_OFF") {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("💡 LED 已关闭");
    }
}
```

在 `setup()` 中添加：
```cpp
pinMode(LED_BUILTIN, OUTPUT);
```

### 3. 发送传感器数据

在 `loop()` 中添加：

```cpp
// 每2秒发送一次传感器数据
static unsigned long lastSensorRead = 0;
if (deviceConnected && (millis() - lastSensorRead > 2000)) {
    lastSensorRead = millis();
    
    // 读取传感器（示例：假设温度传感器）
    float temperature = 25.5;  // 替换为实际读取
    
    String data = "温度: " + String(temperature) + "°C\n";
    pTxCharacteristic->setValue(data.c_str());
    pTxCharacteristic->notify();
}
```

## 🛠️ 常见问题

### Q: 找不到设备？
A: 
1. 检查串口输出是否正常
2. 重启 ESP32-S3
3. 确保手机蓝牙已打开
4. 尝试用其他手机测试

### Q: 连接后立即断开？
A:
1. 检查电源是否稳定
2. 更换 USB 数据线
3. 使用外部 5V 电源

### Q: 收不到数据？
A:
1. 确认已启用通知（nRF Connect）
2. 检查发送格式是否正确
3. 查看串口监视器确认 ESP32 是否收到

### Q: 编译错误？
A:
```bash
# 清理并重新编译
pio run -t clean
pio run
```

## 📱 推荐 APP

### Android
- **Serial Bluetooth Terminal** ⭐⭐⭐⭐⭐ (最简单)
- **nRF Connect** ⭐⭐⭐⭐⭐ (功能强大)
- **BLE Scanner** ⭐⭐⭐⭐

### iOS
- **nRF Connect** ⭐⭐⭐⭐⭐
- **LightBlue** ⭐⭐⭐⭐⭐
- **BLE Terminal** ⭐⭐⭐⭐

## 📚 更多资料

- [完整使用指南](BLE_DEMO_GUIDE.md)
- [ESP32-S3 官方文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)

---

**遇到问题？** 请检查串口输出的调试信息，大部分问题都能从日志中找到原因。

**祝你使用愉快！** 🚀
