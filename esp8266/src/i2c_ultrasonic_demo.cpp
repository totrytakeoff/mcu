#include <Arduino.h>
#include <Wire.h>

// I2C超声波传感器地址 (需要根据具体传感器型号确定)
#define ULTRASONIC_I2C_ADDR 0x70  // 这是一个常见的地址，实际地址请查阅传感器手册

// I2C引脚定义
#define SCL_PIN D1  // GPIO5
#define SDA_PIN D2  // GPIO4

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  Serial.println("I2C Ultrasonic Sensor Demo");

  // 初始化I2C接口
  Wire.begin(SDA_PIN, SCL_PIN);

  // 检查传感器连接
  Wire.beginTransmission(ULTRASONIC_I2C_ADDR);
  int error = Wire.endTransmission();
  if (error == 0) {
    Serial.println("Ultrasonic sensor connected");
  } else {
    Serial.println("Ultrasonic sensor connection failed");
  }
}

void loop() {
  // 读取距离值 (具体命令需要根据传感器手册确定)
  Wire.beginTransmission(ULTRASONIC_I2C_ADDR);
  Wire.write(0x00); // 发送读取距离的命令 (需要根据具体传感器确定)
  Wire.endTransmission();

  // 请求2个字节的数据
  Wire.requestFrom(ULTRASONIC_I2C_ADDR, 2);

  if (Wire.available() >= 2) {
    // 读取距离数据
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();

    // 合并数据 (具体格式需要根据传感器手册确定)
    uint16_t distance = (highByte << 8) | lowByte;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Failed to read data from sensor");
  }

  delay(1000);
}