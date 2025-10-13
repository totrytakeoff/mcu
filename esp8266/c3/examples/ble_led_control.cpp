/**
 * ESP32-S3 BLE LED 控制示例
 * 
 * 功能：通过手机蓝牙 APP 控制 LED 开关和亮度
 * 
 * 支持命令：
 * - LED:ON     - 打开 LED
 * - LED:OFF    - 关闭 LED
 * - LED:BLINK  - LED 闪烁
 * - LED:PWM:50 - 设置亮度为 50% (0-100)
 * - STATUS     - 查询当前状态
 * - HELP       - 显示帮助信息
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

// BLE UUID
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define DEVICE_NAME "ESP32-LED-Control"

// GPIO 配置
#define LED_PIN 2  // 板载 LED，可以改为其他 GPIO

// 全局变量
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pTxCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// LED 状态
enum LedMode {
    LED_OFF,
    LED_ON,
    LED_BLINK,
    LED_PWM
};

LedMode currentMode = LED_OFF;
int pwmValue = 255;  // 0-255
bool blinkState = false;
unsigned long lastBlinkTime = 0;

// 发送消息到手机
void sendMessage(String msg) {
    if (deviceConnected && pTxCharacteristic != nullptr) {
        pTxCharacteristic->setValue(msg.c_str());
        pTxCharacteristic->notify();
    }
    Serial.println("📤 " + msg);
}

// 处理 LED 控制
void handleLedControl(String command) {
    command.trim();
    command.toUpperCase();
    
    if (command == "LED:ON") {
        currentMode = LED_ON;
        digitalWrite(LED_PIN, HIGH);
        sendMessage("✅ LED 已打开");
        
    } else if (command == "LED:OFF") {
        currentMode = LED_OFF;
        digitalWrite(LED_PIN, LOW);
        sendMessage("✅ LED 已关闭");
        
    } else if (command == "LED:BLINK") {
        currentMode = LED_BLINK;
        sendMessage("✅ LED 闪烁模式");
        
    } else if (command.startsWith("LED:PWM:")) {
        String pwmStr = command.substring(8);
        int percent = pwmStr.toInt();
        
        if (percent >= 0 && percent <= 100) {
            currentMode = LED_PWM;
            pwmValue = map(percent, 0, 100, 0, 255);
            analogWrite(LED_PIN, pwmValue);
            sendMessage("✅ LED 亮度设置为 " + String(percent) + "%");
        } else {
            sendMessage("❌ 错误：亮度范围 0-100");
        }
        
    } else if (command == "STATUS") {
        String status = "📊 当前状态：\n";
        switch (currentMode) {
            case LED_OFF:
                status += "LED: 关闭";
                break;
            case LED_ON:
                status += "LED: 打开";
                break;
            case LED_BLINK:
                status += "LED: 闪烁模式";
                break;
            case LED_PWM:
                status += "LED: PWM模式 (" + String(map(pwmValue, 0, 255, 0, 100)) + "%)";
                break;
        }
        sendMessage(status);
        
    } else if (command == "HELP") {
        String help = "📖 命令帮助：\n";
        help += "LED:ON - 打开LED\n";
        help += "LED:OFF - 关闭LED\n";
        help += "LED:BLINK - 闪烁模式\n";
        help += "LED:PWM:50 - 设置亮度(0-100)\n";
        help += "STATUS - 查询状态\n";
        help += "HELP - 显示帮助";
        sendMessage(help);
        
    } else {
        sendMessage("❌ 未知命令：" + command + "\n发送 HELP 查看帮助");
    }
}

// 服务器回调
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("📱 设备已连接");
    }

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("📱 设备已断开");
    }
};

// 接收特征回调
class RxCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() > 0) {
            String command = String(rxValue.c_str());
            Serial.println("📥 收到命令: " + command);
            handleLedControl(command);
        }
    }
};

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32-S3 BLE LED 控制示例           ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    // 初始化 LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.println("✅ LED 引脚初始化完成 (GPIO " + String(LED_PIN) + ")");
    
    // 初始化 BLE
    Serial.println("🔧 正在初始化 BLE...");
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
    
    Serial.println("✅ BLE 初始化完成");
    Serial.println("🔍 设备名称: " + String(DEVICE_NAME));
    Serial.println("📡 等待连接...\n");
    Serial.println("💡 可用命令：");
    Serial.println("   LED:ON, LED:OFF, LED:BLINK");
    Serial.println("   LED:PWM:50, STATUS, HELP\n");
}

void loop() {
    // 处理连接状态变化
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        delay(100);
        sendMessage("🎉 欢迎！发送 HELP 查看命令");
    }
    
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("📡 重新开始广播...");
        oldDeviceConnected = deviceConnected;
        
        // 断开连接时关闭 LED
        currentMode = LED_OFF;
        digitalWrite(LED_PIN, LOW);
    }
    
    // 处理闪烁模式
    if (currentMode == LED_BLINK) {
        if (millis() - lastBlinkTime >= 500) {
            lastBlinkTime = millis();
            blinkState = !blinkState;
            digitalWrite(LED_PIN, blinkState);
        }
    }
    
    delay(10);
}
