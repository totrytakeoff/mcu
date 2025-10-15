#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// 定义红外接收器连接的引脚
const uint16_t kRecvPin = 14; // D5引脚 on NodeMCU

// 创建红外接收对象
IRrecv irrecv(kRecvPin);

// 存储解码结果的结构体
decode_results results;

void setup() {
  // 启动串口通信
  Serial.begin(115200);
  Serial.println("IR Receiver started...");

  // 初始化红外接收器
  irrecv.enableIRIn();
}

void loop() {
  // 检查是否接收到红外信号
  if (irrecv.decode(&results)) {
    // 打印接收到的红外代码
    Serial.print("Received IR code: ");
    Serial.println(results.value, HEX);

    // 打印红外协议类型
    Serial.print("Protocol: ");
    Serial.println(typeToString(results.decode_type));

    // 继续接收下一个值
    irrecv.resume();
  }

  // 短暂延迟
  delay(100);
}