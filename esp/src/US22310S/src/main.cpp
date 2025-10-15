/**
 * @file main.cpp
 * @brief ESP8266 NodeMCU 使用 US22310S 超声波测距模块 Demo
 * 
 * 本程序演示如何使用ESP8266 NodeMCU通过I2C接口控制US22310S超声波测距模块
 * 
 * 硬件连接：
 * - VCC: 5V
 * - GND: GND
 * - SCL: D1 (GPIO5)
 * - SDA: D2 (GPIO4)
 * - INT: 不连接（本Demo使用单次测量模式）
 * 
 * 功能：
 * 1. 初始化I2C通信
 * 2. 检测超声波模块是否在线
 * 3. 启动单次测量
 * 4. 读取距离值
 * 5. 通过串口输出结果
 */

#include <Arduino.h>
#include <Wire.h>
#include "iic.h"

// 创建超声波模块对象
US22310S_I2C ultrasonic;

// 测量间隔时间（毫秒）
const unsigned long MEASURE_INTERVAL = 1000;  // 1秒测量一次

// 上一次测量时间
unsigned long lastMeasureTime = 0;

// I2C扫描函数
void scanI2CDevices() {
    Serial.println("\n开始扫描I2C总线...");
    Serial.println("扫描地址范围: 0x01 到 0x7F");
    
    byte count = 0;
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        byte error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.print("发现I2C设备，地址: 0x");
            if (addr < 16) Serial.print("0");
            Serial.print(addr, HEX);
            Serial.print(" (十进制: ");
            Serial.print(addr);
            Serial.println(")");
            count++;
        }
    }
    
    if (count == 0) {
        Serial.println("警告: 未发现任何I2C设备！");
        Serial.println("可能的原因：");
        Serial.println("1. 接线错误");
        Serial.println("2. 设备未供电");
        Serial.println("3. SCL/SDA引脚配置错误");
        Serial.println("4. 上拉电阻缺失（某些模块需要）");
    } else {
        Serial.print("扫描完成，共发现 ");
        Serial.print(count);
        Serial.println(" 个I2C设备");
    }
    Serial.println();
}

void setup() {
    // 初始化串口通信
    Serial.begin(115200);
    Serial.println("ESP8266 NodeMCU US22310S 超声波测距 Demo");
    Serial.println("==========================================");
    
    // 等待串口稳定
    delay(1000);
    
    // 初始化I2C
    ultrasonic.begin();
    Serial.println("I2C 初始化完成");
    Serial.print("SCL引脚: D1 (GPIO5), SDA引脚: D2 (GPIO4)");
    Serial.println();
    
    // 先扫描I2C总线
    scanI2CDevices();
    
    // 检查设备是否在线
    Serial.print("尝试连接US22310S (地址 0x");
    Serial.print(US22310S_READ_ADDR, HEX);
    Serial.println(")...");
    
    if (ultrasonic.checkDevice()) {
        Serial.println("US22310S 超声波模块检测成功！");
        Serial.print("I2C 地址: 0x");
        Serial.println(US22310S_READ_ADDR, HEX);
    } else {
        Serial.println("错误: 无法检测到US22310S超声波模块！");
        Serial.println("请检查接线：");
        Serial.println("- VCC: 5V");
        Serial.println("- GND: GND");
        Serial.println("- SCL: D1 (GPIO5)");
        Serial.println("- SDA: D2 (GPIO4)");
        Serial.println("\n提示: 请查看上方的I2C扫描结果");
        while (1) {
            delay(1000); // 停止程序执行
        }
    }
    
    Serial.println("系统初始化完成，开始测量...");
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // 检查是否到达测量间隔
    if (currentTime - lastMeasureTime >= MEASURE_INTERVAL) {
        lastMeasureTime = currentTime;
        
        // 启动单次测量
        if (ultrasonic.startSingleMeasurement()) {
            // 等待测量完成
            // 超声波测量时间 = 距离 / 声速
            // 最大距离1000mm往返需要约6ms，加上处理时间，建议等待至少50-100ms
            delay(80);
            
            // 多次尝试读取，直到获得有效数据
            uint8_t distance_mm = 255;
            bool valid_read = false;
            
            for (int retry = 0; retry < 5; retry++) {
                if (ultrasonic.readDistance(&distance_mm)) {
                    // 如果不是0xFF，说明读取到了数据
                    if (distance_mm != 255 && distance_mm != 0) {
                        valid_read = true;
                        break;
                    }
                }
                // 如果第一次读到255，再等待一小段时间重试
                if (retry < 4) {
                    delay(20);
                }
            }
            
            if (valid_read) {
                // 检查测量值是否在有效范围内
                if (distance_mm >= 15 && distance_mm <= 254) {
                    Serial.print("✓ 距离: ");
                    Serial.print(distance_mm);
                    Serial.print(" mm (");
                    float distance_cm = distance_mm / 10.0;
                    Serial.print(distance_cm, 1);
                    Serial.println(" cm)");
                } else {
                    Serial.print("? 数据异常: ");
                    Serial.print(distance_mm);
                    Serial.println(" mm");
                }
            } else {
                Serial.println("✗ 测量失败 - 持续返回0xFF (可能物体太远或太近)");
            }
        } else {
            Serial.println("✗ 无法发送测量命令");
        }
        
        Serial.println();  // 空行分隔
    }
    
    // 可以在这里添加其他功能
    // 例如：检测INT引脚状态等
}

/**
 * @brief 测试函数：演示阈值中断模式的使用
 * 
 * 注意：此函数需要手动调用，不适合在loop()中自动执行
 */
void testThresholdMode() {
    Serial.println("测试阈值中断模式...");
    
    // 设置阈值为100mm
    if (ultrasonic.setThresholdMode(100)) {
        Serial.println("阈值设置成功: 100mm");
        Serial.println("当测量距离 <= 100mm 时，INT引脚输出高电平");
        Serial.println("当测量距离 > 100mm 时，INT引脚输出低电平");
    } else {
        Serial.println("错误: 无法设置阈值");
    }
}
