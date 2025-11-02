// STM32F103C8T6 LED闪烁程序
// 使用PC13引脚，该引脚通常连接到板载LED

#include <Arduino.h>

#define LED_PIN PC13

void setup() {
  // 设置PC13引脚为输出模式
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // LED亮
  digitalWrite(LED_PIN, LOW);  // STM32中LOW为LED亮，HIGH为灭
  delay(500);                   // 延时500ms
  
  // LED灭
  digitalWrite(LED_PIN, HIGH);
  delay(500);                   // 延时500ms
}
