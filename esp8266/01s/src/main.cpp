/*
 * ESP-01S WiFi透传模块
 * 功能：通过WiFi接收数据并转发给STM32主控
 * 
 * 硬件连接：
 * ESP-01S TX -> STM32 RX
 * ESP-01S RX -> STM32 TX
 * ESP-01S VCC -> 3.3V
 * ESP-01S GND -> GND
 * 
 * 使用说明：
 * 1. 修改下方的WiFi SSID和密码
 * 2. ESP-01S将创建TCP服务器，监听8080端口
 * 3. 客户端连接后发送的数据将转发给STM32
 * 4. STM32发送的数据将转发给客户端
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>

// WiFi配置 - 请修改为你的WiFi信息
const char* ssid = "YOUR_WIFI_SSID";        // 修改为你的WiFi名称
const char* password = "YOUR_WIFI_PASSWORD"; // 修改为你的WiFi密码

// TCP服务器端口
const uint16_t serverPort = 8080;

// 创建WiFi服务器
WiFiServer server(serverPort);
WiFiClient client;

// 串口配置（与STM32通信）
#define STM32_SERIAL_BAUD 115200  // STM32通信波特率，根据实际情况修改

// 调试开关
#define DEBUG_ENABLED true

// 缓冲区
#define BUFFER_SIZE 512
uint8_t wifiBuffer[BUFFER_SIZE];
uint8_t serialBuffer[BUFFER_SIZE];

void setup() {
  // 初始化串口（用于与STM32通信和调试）
  Serial.begin(STM32_SERIAL_BAUD);
  delay(100);
  
  if (DEBUG_ENABLED) {
    Serial.println("\n\n=================================");
    Serial.println("ESP-01S WiFi透传模块启动");
    Serial.println("=================================");
  }
  
  // 连接WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  if (DEBUG_ENABLED) {
    Serial.print("正在连接WiFi: ");
    Serial.println(ssid);
  }
  
  // 等待WiFi连接
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    if (DEBUG_ENABLED) {
      Serial.print(".");
    }
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    if (DEBUG_ENABLED) {
      Serial.println("\nWiFi连接成功！");
      Serial.print("IP地址: ");
      Serial.println(WiFi.localIP());
      Serial.print("TCP服务器端口: ");
      Serial.println(serverPort);
      Serial.println("=================================");
      Serial.println("等待客户端连接...");
      Serial.println("=================================\n");
    }
    
    // 启动TCP服务器
    server.begin();
    server.setNoDelay(true);
  } else {
    if (DEBUG_ENABLED) {
      Serial.println("\nWiFi连接失败！");
      Serial.println("请检查WiFi配置后重启设备");
    }
  }
}

void loop() {
  // 检查是否有新客户端连接
  if (server.hasClient()) {
    // 如果已有客户端连接，断开旧连接
    if (client && client.connected()) {
      if (DEBUG_ENABLED) {
        Serial.println("[INFO] 断开旧客户端连接");
      }
      client.stop();
    }
    
    // 接受新客户端
    client = server.available();
    if (DEBUG_ENABLED) {
      Serial.println("[INFO] 新客户端已连接");
      Serial.print("[INFO] 客户端IP: ");
      Serial.println(client.remoteIP());
    }
  }
  
  // 如果有客户端连接
  if (client && client.connected()) {
    // 1. 从WiFi客户端接收数据，转发给STM32
    int wifiAvailable = client.available();
    if (wifiAvailable > 0) {
      int bytesToRead = min(wifiAvailable, BUFFER_SIZE);
      int bytesRead = client.read(wifiBuffer, bytesToRead);
      
      if (bytesRead > 0) {
        // 调试输出：显示接收到的WiFi数据
        if (DEBUG_ENABLED) {
          Serial.print("[WiFi->STM32] 接收 ");
          Serial.print(bytesRead);
          Serial.print(" 字节: ");
          
          // 以十六进制显示
          for (int i = 0; i < bytesRead; i++) {
            if (wifiBuffer[i] < 0x10) Serial.print("0");
            Serial.print(wifiBuffer[i], HEX);
            Serial.print(" ");
          }
          Serial.print("| ASCII: ");
          
          // 以ASCII显示（可打印字符）
          for (int i = 0; i < bytesRead; i++) {
            if (wifiBuffer[i] >= 32 && wifiBuffer[i] <= 126) {
              Serial.write(wifiBuffer[i]);
            } else {
              Serial.print(".");
            }
          }
          Serial.println();
        }
        
        // 转发给STM32（通过串口）
        Serial.write(wifiBuffer, bytesRead);
        Serial.flush();
      }
    }
    
    // 2. 从STM32接收数据（串口），转发给WiFi客户端
    int serialAvailable = Serial.available();
    if (serialAvailable > 0) {
      int bytesToRead = min(serialAvailable, BUFFER_SIZE);
      int bytesRead = Serial.readBytes(serialBuffer, bytesToRead);
      
      if (bytesRead > 0) {
        // 转发给WiFi客户端
        client.write(serialBuffer, bytesRead);
        client.flush();
        
        // 调试输出：显示转发的数据
        if (DEBUG_ENABLED) {
          Serial.print("[STM32->WiFi] 转发 ");
          Serial.print(bytesRead);
          Serial.print(" 字节: ");
          
          // 以十六进制显示
          for (int i = 0; i < bytesRead; i++) {
            if (serialBuffer[i] < 0x10) Serial.print("0");
            Serial.print(serialBuffer[i], HEX);
            Serial.print(" ");
          }
          Serial.print("| ASCII: ");
          
          // 以ASCII显示（可打印字符）
          for (int i = 0; i < bytesRead; i++) {
            if (serialBuffer[i] >= 32 && serialBuffer[i] <= 126) {
              Serial.write(serialBuffer[i]);
            } else {
              Serial.print(".");
            }
          }
          Serial.println();
        }
      }
    }
  } else {
    // 检查客户端是否断开
    if (client) {
      if (DEBUG_ENABLED) {
        Serial.println("[INFO] 客户端已断开");
      }
      client.stop();
    }
  }
  
  // 短暂延时，避免CPU占用过高
  delay(1);
}

