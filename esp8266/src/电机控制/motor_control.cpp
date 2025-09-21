// const int motorPin = D5;  // 信号引脚（示例用D5）
#include <Arduino.h>

#define motorPin D1


// 核心控制函数：设置Tsig（单位：微秒）
void setMotor(int tsig_us) {
  tsig_us = constrain(tsig_us, 0, 20000); // 限制在0~20000us内
  
  // 计算PWM占空比（高电平时间占比）
  float high_time_us = 20000 - tsig_us; // 周期20ms=20000us
  int duty = (high_time_us / 20000) * 1023; // 转换为10位值
  
  analogWrite(motorPin, duty);
}

// 简化控制函数
void forwardMax() { setMotor(1800); }  // Tsig>1750us（正转最大）
void reverseMax() { setMotor(1000); }  // Tsig<1250us（反转最大）
void stopMotor()  { setMotor(1500); }  // 中间值（停止）



void setup() {
  pinMode(motorPin, OUTPUT);
  analogWriteFreq(50);    // 设置PWM频率=50Hz（周期20ms）
  analogWriteRange(1023); // 10位分辨率（0-1023）
  stopMotor();            // 初始状态停止
}

void loop() {
  // 示例动作序列
  forwardMax();
  delay(2000);      // 正转2秒
  stopMotor();
  delay(500);       // 停止0.5秒
  reverseMax();
  delay(2000);      // 反转2秒
  stopMotor();
  delay(2000);      // 停止2秒
}