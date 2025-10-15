/*
 * ESP32-C3 测试 Demo - 最简稳定版本
 * 功能：
 * 1. LED闪烁测试
 * 2. 串口输出信息
 */

#include <Arduino.h>

// LED引脚定义（ESP32-C3 DevKitM-1板载LED在GPIO8）
const int ledPin = 8;

// 计时器变量
unsigned long previousMillis = 0;
const long interval = 1000;  // LED闪烁间隔（毫秒）
int ledState = LOW;

void setup() {
  // 初始化串口
  Serial.begin(115200);
  delay(1000);  // 等待串口稳定
  
  Serial.println("\n=== ESP32-C3 测试 Demo 启动中 ===");
  
  // 设置LED引脚为输出
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // 打印ESP32-C3信息
  Serial.println("\n=== ESP32-C3 信息 ===");
  Serial.print("芯片型号: ");
  Serial.println(ESP.getChipModel());
  Serial.print("CPU频率: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Flash大小: ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.print("当前可用内存: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
  
  Serial.println("\nLED闪烁测试开始...");
}

void loop() {
  // 获取当前时间
  unsigned long currentMillis = millis();
  
  // LED闪烁逻辑
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // 切换LED状态
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    
    digitalWrite(ledPin, ledState);
    
    // 串口输出LED状态
    Serial.print("LED状态: ");
    Serial.println(ledState ? "ON" : "OFF");
    
    // 定期输出系统信息
    static unsigned long infoInterval = 0;
    if (currentMillis - infoInterval >= 10000) {  // 每10秒
      infoInterval = currentMillis;
      Serial.print("运行时间: ");
      Serial.print(currentMillis / 1000);
      Serial.println(" 秒");
      Serial.print("可用内存: ");
      Serial.print(ESP.getFreeHeap() / 1024);
      Serial.println(" KB");
    }
  }
  
  // 避免重启的延迟
  delay(1);
}
