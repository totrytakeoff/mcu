#include <Servo.h>
#include "claw_control.h"

// 创建舵机对象
Servo clawServo;

// 爪子状态变量
static int clawPosition = DEFAULT_POSITION;
static ClawState currentState = CLAW_STOPPED;
static bool autoModeEnabled = false;

// 初始化机械爪
void initClaw() {
  clawServo.attach(SERVO_PIN);
  clawServo.write(clawPosition);
  currentState = CLAW_STOPPED;
}

// 张开爪子
void openClaw() {
  clawPosition = OPEN_POSITION;
  clawServo.write(clawPosition);
  currentState = CLAW_OPEN;
}

// 闭合爪子
void closeClaw() {
  clawPosition = CLOSE_POSITION;
  clawServo.write(clawPosition);
  currentState = CLAW_CLOSED;
}

// 停止运动
void stopClaw() {
  // 舵机在目标位置会自动停止，此处保存当前状态
  currentState = CLAW_STOPPED;
}

// 设置指定角度
void setPosition(int angle) {
  // 限制角度范围在0-180度之间
  angle = constrain(angle, 0, 360);
  clawPosition = angle;
  clawServo.write(clawPosition);
  currentState = CLAW_MOVING;
}

// 启用自动模式
void enableAutoMode() {
  autoModeEnabled = true;
  currentState = CLAW_MOVING;
}

// 禁用自动模式
void disableAutoMode() {
  autoModeEnabled = false;
  currentState = CLAW_STOPPED;
}

// 检查是否处于自动模式
bool isAutoModeEnabled() {
  return autoModeEnabled;
}

// 获取爪子当前状态
ClawState getClawState() {
  return currentState;
}

// 获取爪子当前位置
int getClawPosition() {
  return clawPosition;
}