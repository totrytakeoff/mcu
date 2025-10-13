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
#include <ctype.h>

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

static inline bool isAllowedKey(char c) {
    switch (c) {
        case 'F': case 'B': case 'L': case 'R':
        case 'W': case 'X': case 'Y': case 'Z':
        case 'U': case 'S': case 'D':
            return true;
        default:
            return false;
    }
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
        if (rxValue.empty()) return;

        // 在USB串口打印可见ASCII（仅调试）
        Serial.print("RX: ");
        for (size_t i = 0; i < rxValue.size(); ++i) {
            char c = rxValue[i];
            if (c < 0x20 || c > 0x7E) c = '.';
            Serial.print(c);
        }
        Serial.println();

        // 行级预处理：按行解析，兼容 APP 注入的标签/计数
        static std::string acc;
        acc.append(rxValue);
        auto upper = [](char c){ return (char)toupper((unsigned char)c); };
        auto isDigit = [](char c){ return c >= '0' && c <= '9'; };

        std::string out; // 仅输出规范化的协议内容

        while (true) {
            size_t posN = acc.find('\n');
            size_t posR = acc.find('\r');
            size_t pos = std::min(posN == std::string::npos ? acc.size() : posN,
                                   posR == std::string::npos ? acc.size() : posR);
            if (pos == std::string::npos || pos >= acc.size()) break; // 无完整行

            std::string line = acc.substr(0, pos);
            acc.erase(0, pos + 1);

            // 去除可见外的噪声与两端空白
            std::string vis;
            for (char ch : line) vis.push_back((ch >= 0x20 && ch <= 0x7E) ? ch : ' ');
            // 复制一份大写版本用于匹配
            std::string u; u.reserve(vis.size());
            for (char ch : vis) u.push_back(upper(ch));

            // 1) 摇杆：从整行内提取 A###P##
            {
                size_t i = 0;
                while (i < u.size()) {
                    if (u[i] != 'A') { ++i; continue; }
                    size_t j = i + 1;
                    char ang[3]; int ad = 0;
                    for (; j < u.size() && ad < 3; ++j) if (isDigit(u[j])) ang[ad++] = u[j];
                    if (ad == 3) {
                        while (j < u.size() && u[j] != 'P') ++j;
                        if (j < u.size() && u[j] == 'P') {
                            ++j; char powv[2]; int pd = 0;
                            for (; j < u.size() && pd < 2; ++j) if (isDigit(u[j])) powv[pd++] = u[j];
                            if (pd == 2) {
                                out.push_back('A');
                                out.append(ang, ang + 3);
                                out.push_back('P');
                                out.append(powv, powv + 2);
                                out.push_back('\n');
                                break; // 一行只取一帧
                            }
                        }
                    }
                    ++i;
                }
            }

            // 2) 单键：若整行去除空白后仅1字符且在白名单，输出该键
            {
                std::string compact;
                for (char ch : u) if (ch != ' ' && ch != '\t') compact.push_back(ch);
                if (compact.size() == 1 && isAllowedKey(compact[0])) {
                    out.push_back(compact[0]);
                }
            }
        }

        if (!out.empty()) {
            size_t written = Serial1.write((const uint8_t*)out.data(), out.size());
            Serial.print("UART->STM32 CLEAN bytes: "); Serial.println((int)written);
        }
    }
};

// 初始化 BLE 服务（稳定版）
void initBLE() {
    Serial.println("BLE init...");
    NimBLEDevice::init(DEVICE_NAME);
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    NimBLEService* pService = pServer->createService(SERVICE_UUID);
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    NimBLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new RxCallbacks());
    pService->start();
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
    Serial.begin(115200);
    delay(1000);
    Serial1.begin(STM32_UART_BAUD, SERIAL_8N1, STM32_UART_RX_PIN, STM32_UART_TX_PIN);
    Serial.println("STM32 UART ready on TX=17 RX=18 @115200");
    Serial.println("\n\nESP32-S3 BLE UART Demo");
    Serial.println("Initializing...");
    initBLE();
    Serial.println("\nTips:");
    Serial.println("  1. Open BLE terminal app on the phone");
    Serial.println("  2. Connect to '" + String(DEVICE_NAME) + "'");
    Serial.println("  3. Send protocol-only: F/L/R/B/S or A090P50");
    Serial.println();
}

void loop() {
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        delay(100);
        String welcome = "Welcome to ESP32-S3!\n";
        bleSend((const uint8_t*)welcome.c_str(), welcome.length());
        Serial.println("TX: welcome sent");
    }
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("Advertising restarted...");
        oldDeviceConnected = deviceConnected;
    }
    static unsigned long lastHeartbeat = 0;
    if (deviceConnected && (millis() - lastHeartbeat > 10000)) {
        lastHeartbeat = millis();
        String heartbeat = "HEARTBEAT " + String(millis() / 1000) + "s\n";
        bleSendText(heartbeat);
        Serial.println("TX: heartbeat sent");
    }
    delay(100);
}
