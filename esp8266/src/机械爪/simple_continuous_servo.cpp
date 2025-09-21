#include <Arduino.h>
#include <Servo.h>
#include "core_esp8266_features.h"

// 舵机控制参数
const int SERVO_PIN = D1;  // 舵机连接引脚

// 创建舵机对象
Servo continuousServo;

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  Serial.println("Continuous Rotation Servo Demo");

  // 初始化舵机
  continuousServo.attach(SERVO_PIN);

  // 初始停止
  continuousServo.write(90);
  Serial.println("Servo stopped (90 degrees)");
  delay(2000);
}

void loop() {
 continuousServo.write(0);
  delay(1000);
  continuousServo.write(180);
  delay(1000);
  continuousServo.write(360);
  delay(1000);

}