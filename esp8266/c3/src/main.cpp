/**
 * ESP32-S3 BLE UART 通讯示例
 * 
 * 功能说明：
 * - 创建一个 BLE UART 服务，可与手机蓝牙串口 APP 通讯
 * - 接收手机发送的数据并通过串口打印
 * - 将接收到的数据回显给手机
 * - 支持连接状态监控
 * 
 * 使用方法：
 * 1. 上传代码到 ESP32-S3
 * 2. 打开串口监视器（115200 波特率）
 * 3. 在手机上使用蓝牙串口 APP（如 Serial Bluetooth Terminal）搜索设备
 * 4. 连接名为 "ESP32-S3-BLE" 的设备
 * 5. 发送数据即可进行双向通讯
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

// BLE 服务和特征 UUID（使用 Nordic UART Service 标准）
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // 接收特征（手机写入）
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // 发送特征（手机读取/通知）

#define DEVICE_NAME "ESP32-S3-BLE"  // 蓝牙设备名称

// UART configuration for STM32 passthrough
#define STM32_UART_BAUD 115200
#define STM32_UART_TX_PIN 17  // ESP32 -> STM32 RX
#define STM32_UART_RX_PIN 18  // ESP32 <- STM32 TX (unused in this one-way demo)

// 全局变量
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pTxCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// BLE send interfaces for future use
void bleSend(const uint8_t* data, size_t len) {
    if (deviceConnected && pTxCharacteristic != nullptr && data != nullptr && len > 0) {
        pTxCharacteristic->setValue(data, len);
        pTxCharacteristic->notify();
    }
}

void bleSendText(const String& text) {
    bleSend((const uint8_t*)text.c_str(), text.length());
}

// 服务器回调类：处理连接和断开事件（稳定版）
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Client connected");
    }

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Client disconnected");
    }
};

// 接收特征回调类：处理从手机接收到的数据
class RxCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        // 获取接收到的数据
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() > 0) {
            // Log to serial (ASCII only)
            Serial.print("RX: ");
            for (int i = 0; i < rxValue.length(); i++) {
                char c = rxValue[i];
                // Replace non-printable with '.' to keep ASCII-only logs
                if (c < 0x20 || c > 0x7E) c = '.';
                Serial.print(c);
            }
            Serial.println();

            // Forward raw bytes to STM32 over UART (no modification)
            size_t written = Serial1.write((const uint8_t*)rxValue.data(), rxValue.size());
            Serial.print("UART TX -> STM32 bytes: ");
            Serial.println((int)written);

            // Echo disabled per requirement; BLE send interface retained for future use
        }
    }
};

// 初始化 BLE 服务（稳定版）
void initBLE() {
    Serial.println("BLE init...");
    
    // 初始化 NimBLE 设备
    NimBLEDevice::init(DEVICE_NAME);
    
    // 创建 BLE 服务器
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    // 创建 BLE 服务
    NimBLEService* pService = pServer->createService(SERVICE_UUID);
    
    // 创建 TX 特征（用于发送数据到手机）
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    
    // 创建 RX 特征（用于接收手机的数据）
    NimBLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new RxCallbacks());
    
    // 启动服务
    pService->start();
    
    // 开始广播
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    NimBLEDevice::startAdvertising();
    
    Serial.println("BLE ready");
    Serial.println("Device: " + String(DEVICE_NAME));
    Serial.println("Waiting for phone connection...");
}

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);

    // Initialize UART to STM32
    Serial1.begin(STM32_UART_BAUD, SERIAL_8N1, STM32_UART_RX_PIN, STM32_UART_TX_PIN);
    Serial.println("STM32 UART ready on TX=17 RX=18 @115200");
    
    Serial.println("\n\nESP32-S3 BLE UART Demo");
    Serial.println("Initializing...");
    
    // 初始化 BLE
    initBLE();
    
    Serial.println("\nTips:");
    Serial.println("  1. Open BLE terminal app on the phone");
    Serial.println("  2. Connect to '" + String(DEVICE_NAME) + "'");
    Serial.println("  3. Send text to test echo");
    Serial.println();
}

void loop() {
    // 处理连接状态变化
    if (deviceConnected && !oldDeviceConnected) {
        // 新连接建立
        oldDeviceConnected = deviceConnected;
        
        // 发送欢迎消息
        delay(100);
        String welcome = "Welcome to ESP32-S3!\n";
        bleSend((const uint8_t*)welcome.c_str(), welcome.length());
        Serial.println("TX: welcome sent");
    }
    
    if (!deviceConnected && oldDeviceConnected) {
        // 连接断开
        delay(500);  // 给蓝牙栈时间准备
        pServer->startAdvertising();  // 重新开始广播
        Serial.println("Advertising restarted...");
        oldDeviceConnected = deviceConnected;
    }
    
    // 示例：定期发送心跳数据
    static unsigned long lastHeartbeat = 0;
    if (deviceConnected && (millis() - lastHeartbeat > 10000)) {
        lastHeartbeat = millis();
        String heartbeat = "HEARTBEAT " + String(millis() / 1000) + "s\n";
        bleSendText(heartbeat);
        Serial.println("TX: heartbeat sent");
    }
    
    delay(100);
}
