#include <Arduino.h>

// 超声波传感器引脚定义
const int TRIG_PIN = D2;  // Trig引脚连接到NodeMCU的D2
const int ECHO_PIN = D3;  // Echo引脚连接到NodeMCU的D3

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  Serial.println("US-22310S Ultrasonic Sensor Demo");

  // 设置引脚模式
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // 测量距离
  long duration, distance;

  // 产生10微秒的触发脉冲
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 读取回波脉冲的持续时间
  duration = pulseIn(ECHO_PIN, HIGH, 30000); // 超时设置为30毫秒

  // 计算距离（声速为343米/秒，即29.1微秒/厘米）
  // 距离 = (持续时间 / 2) / 29.1
  if (duration > 0) {
    distance = duration / 58; // 简化计算：距离（厘米）= 持续时间 / 58
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Out of range");
  }

  delay(1000); // 每秒测量一次
}