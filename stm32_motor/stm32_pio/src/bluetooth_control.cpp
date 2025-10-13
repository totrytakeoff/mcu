/**
 * @file    bluetooth_control.cpp
 * @brief   蓝牙控制类实现
 * @author  AI Assistant
 * @date    2025-10-11
 */

#include "../include/common.h"
#include "../include/bluetooth_control.hpp"
#include <cstring>
#include <cmath>

// 辅助：判断是否为允许的一字节按键命令
static inline bool isAllowedKey(uint8_t c) {
    switch (c) {
        case 'F': case 'B': case 'L': case 'R':
        case 'W': case 'X': case 'Y': case 'Z':
        case 'U': case 'S': case 'D':
            return true;
        default:
            return false;
    }
}

// 静态成员初始化
BluetoothControl* BluetoothControl::instance_ = nullptr;

/**
 * @brief 构造函数
 */
BluetoothControl::BluetoothControl(RemoteControl& remoteControl)
        : remoteControl_(remoteControl)
        , joystickEnabled_(true)
        , joystickIndex_(0)
        , textMode_(false)
        , lineIndex_(0) {
    instance_ = this;
    memset(joystickBuffer_, 0, sizeof(joystickBuffer_));
    memset(lineBuffer_, 0, sizeof(lineBuffer_));
    rxqHead_ = rxqTail_ = 0;
}

/**
 * @brief 析构函数
 */
BluetoothControl::~BluetoothControl() {
    instance_ = nullptr;
}

/**
 * @brief 初始化
 */
void BluetoothControl::init() {
    joystickIndex_ = 0;
    memset(joystickBuffer_, 0, sizeof(joystickBuffer_));
    textMode_ = false;
    lineIndex_ = 0;
    memset(lineBuffer_, 0, sizeof(lineBuffer_));
    rxqHead_ = rxqTail_ = 0;
}

/**
 * @brief 设置摇杆模式使能
 */
void BluetoothControl::setJoystickMode(bool enable) {
    joystickEnabled_ = enable;
    joystickIndex_ = 0;
}

void BluetoothControl::enqueueFromISR(uint8_t data) {
    uint16_t nextHead = (uint16_t)((rxqHead_ + 1) % kRxQueueSize);
    if (nextHead == rxqTail_) {
        // 队列满，丢弃最旧一个，腾位置（避免卡死）
        rxqTail_ = (uint16_t)((rxqTail_ + 1) % kRxQueueSize);
    }
    rxQueue_[rxqHead_] = data;
    rxqHead_ = nextHead;
}

void BluetoothControl::update() {
    // 消费队列：逐字节喂给解析器
    while (rxqTail_ != rxqHead_) {
        __disable_irq();
        uint8_t b = rxQueue_[rxqTail_];
        rxqTail_ = (uint16_t)((rxqTail_ + 1) % kRxQueueSize);
        __enable_irq();
        handleData(b);
    }
}

/**
 * @brief 获取摇杆模式状态
 */
bool BluetoothControl::isJoystickMode() const {
    return joystickEnabled_;
}

/**
 * @brief 处理接收到的蓝牙数据（主入口）
 */
void BluetoothControl::handleData(uint8_t data) {
    // 统一大小写：将小写转换为大写，便于解析
    if (data >= 'a' && data <= 'z') {
        data = static_cast<uint8_t>(data - 'a' + 'A');
    }

    // 文本模式：忽略所有非协议数据直至换行
    if (textMode_) {
        if (data == '\n' || data == '\r') {
            // 行结束：检查是否恰好是允许的单字节命令行
            if (lineIndex_ == 1 && isAllowedKey(lineBuffer_[0]) && lineBuffer_[0] != 'A') {
                handleKeyCommand(lineBuffer_[0]);
            }
            // 清空并退出文本模式
            textMode_ = false;
            lineIndex_ = 0;
            memset(lineBuffer_, 0, sizeof(lineBuffer_));
        } else if (lineIndex_ < sizeof(lineBuffer_)) {
            lineBuffer_[lineIndex_++] = data;
        }
        return;
    }

    // 忽略孤立的换行/回车（非摇杆帧内）
    if (joystickIndex_ == 0 && (data == '\r' || data == '\n')) return;

    // 摇杆帧起始：进入摇杆帧接收模式，直到换行
    if (data == 'A' && joystickEnabled_) {
        joystickIndex_ = 0;
        joystickBuffer_[joystickIndex_++] = data;
        return;
    }
    
    // 如果正在接收摇杆数据
    if (joystickIndex_ > 0 && joystickIndex_ < sizeof(joystickBuffer_)) {
        joystickBuffer_[joystickIndex_++] = data;
        
        // 检查是否接收完整（以 \n 或 \r 结尾）
        if (data == '\n' || data == '\r') {
            handleJoystickCommand();
            joystickIndex_ = 0;
        }
        
        // 防止缓冲区溢出
        if (joystickIndex_ >= sizeof(joystickBuffer_)) {
            // 异常数据，丢弃并进入文本模式，直到下一行结束
            joystickIndex_ = 0;
            textMode_ = true;
        }
        return;
    }

    // 其余情况：统一按“行模式”收集，等到行结束时再决定是否为单字节命令
    if (!textMode_) {
        textMode_ = true;
        lineIndex_ = 0;
        memset(lineBuffer_, 0, sizeof(lineBuffer_));
    }
    if (data != '\n' && data != '\r' && lineIndex_ < sizeof(lineBuffer_)) {
        lineBuffer_[lineIndex_++] = data;
    }
}

/**
 * @brief 处理按键模式命令
 */
void BluetoothControl::handleKeyCommand(uint8_t data) {
    // 兼容性处理：将 D 视为后退（某些APP可能用D代表Down）
    if (data == 'D') {
        remoteControl_.handleCommand('B');
        return;
    }

    // 直接转发给 RemoteControl 处理（F/B/L/R/U/W/X/Y/Z/S ...）
    remoteControl_.handleCommand(static_cast<char>(data));
}

/**
 * @brief 处理摇杆模式命令（A[角度]P[力度]\n）
 */
void BluetoothControl::handleJoystickCommand() {
    // 验证格式：A[3位数字]P[2位数字]\n 或以\r结尾
    // 最小长度：A090P50\n = 8字节
    if (joystickIndex_ < 8) {
        return;
    }
    
    // 验证格式头：AxxxPyy[\r|\n]
    if (joystickBuffer_[0] != 'A' || joystickBuffer_[4] != 'P') {
        return;
    }
    
    // 解析角度（A后面3位数字）
    int angle = 0;
    for (int i = 1; i <= 3; i++) {
        if (joystickBuffer_[i] < '0' || joystickBuffer_[i] > '9') {
            return;  // 非法字符
        }
        angle = angle * 10 + (joystickBuffer_[i] - '0');
    }
    
    // 解析力度（P后面2位数字）
    int power = 0;
    for (int i = 5; i <= 6; i++) {
        if (joystickBuffer_[i] < '0' || joystickBuffer_[i] > '9') {
            return;  // 非法字符
        }
        power = power * 10 + (joystickBuffer_[i] - '0');
    }
    
    // 范围检查
    if (angle > 359 || power > 99) {
        return;
    }
    
    // 转换为小车控制
    convertJoystickToMotion(angle, power);
}

/**
 * @brief 将摇杆角度和力度转换为小车运动控制
 * 
 * 角度定义：
 * - 000° = 正右（右转）
 * - 090° = 正上（前进）
 * - 180° = 正左（左转）
 * - 270° = 正下（后退）
 * 
 * 策略：
 * - 将360度分为8个区域，映射到 F/B/L/R/W/X/Y/Z
 * - 力度映射为速度百分比
 */
void BluetoothControl::convertJoystickToMotion(int angle, int power) {
    // 力度为0：停止
    if (power <= 0) {
        remoteControl_.handleJoystickSpeeds(0, 0);
        return;
    }

    // 角度归一化
    angle = angle % 360;

    // 将极坐标(角度,力度0-99)映射为车体坐标：前进/后退(-100..100)、转向(-100..100)
    // 约定：90°=前，270°=后，0°=右转，180°=左转
    // 使用正弦/余弦映射：
    // straight = sin(theta) * magnitude
    // turn = cos(theta) * magnitude
    const float rad = static_cast<float>(angle) * 3.1415926f / 180.0f;
    const float magnitude = static_cast<float>(power) * (100.0f / 99.0f); // 规范化到约0..100
    int straight = static_cast<int>(std::roundf(std::sin(rad) * magnitude));
    int turn = static_cast<int>(std::roundf(std::cos(rad) * magnitude));

    // 将模拟速度交给 RemoteControl（会做限幅与梯形平滑）
    remoteControl_.handleJoystickSpeeds(straight, turn);
}
