/**
 * @file    remote_control.cpp
 * @brief   遥控器控制类实现
 * @author  AI Assistant
 * @date    2024
 */

#include "../include/common.h"
#include "../include/remote_control.hpp"


// 静态成员初始化
RemoteControl* RemoteControl::instance_ = nullptr;

/**
 * @brief 构造函数
 */
RemoteControl::RemoteControl(DriveTrain& driveTrain, E49_Wireless& wireless)
        : driveTrain_(driveTrain)
        , wireless_(wireless)
        , baseSpeed_(30)        // 默认基础速度 30%（最低速度）
        , maxSpeed_(100)        // 默认最高速度 100%
        , speedIncrement_(10)   // 默认速度增量 10%
        , turnSensitivity_(50)  // 默认转向灵敏度 50%
        , timeout_(800)         // 默认超时 
        , lastCommand_(0)
        , lastCommandTime_(0)
        , isMoving_(false)
        , currentTargetStraightSpeed_(0)
        , currentTargetTurnSpeed_(0) {
    instance_ = this;  // 保存实例指针
}

/**
 * @brief 析构函数
 */
RemoteControl::~RemoteControl() { instance_ = nullptr; }

/**
 * @brief 初始化
 */
void RemoteControl::init() {
    // 设置 E49 接收回调
    wireless_.setDataReceivedCallback([](uint8_t data) {
        if (instance_ != nullptr) {
            instance_->handleCommand(static_cast<char>(data));
        }
    });

    // 初始停止状态
    stop();
}

/**
 * @brief 更新函数（主循环中调用）
 */
void RemoteControl::update() { checkTimeout(); }

/**
 * @brief 设置超时时间
 */
void RemoteControl::setTimeout(uint32_t timeout_ms) { timeout_ = timeout_ms; }

/**
 * @brief 设置基础速度
 */
void RemoteControl::setBaseSpeed(int speed) {
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;
    baseSpeed_ = speed;
}

/**
 * @brief 设置最高速度限制
 */
void RemoteControl::setMaxSpeed(int speed) {
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;
    maxSpeed_ = speed;
}

/**
 * @brief 设置速度增量
 */
void RemoteControl::setSpeedIncrement(int increment) {
    if (increment < 0) increment = 0;
    if (increment > 100) increment = 100;
    speedIncrement_ = increment;
}

/**
 * @brief 设置转向灵敏度
 */
void RemoteControl::setTurnSensitivity(int sensitivity) {
    if (sensitivity < 0) sensitivity = 0;
    if (sensitivity > 100) sensitivity = 100;
    turnSensitivity_ = sensitivity;
}

/**
 * @brief 获取最后的指令
 */
char RemoteControl::getLastCommand() const { return lastCommand_; }

/**
 * @brief 获取是否正在运动
 */
bool RemoteControl::isMoving() const { return isMoving_; }

/**
 * @brief 处理接收到的指令（累积加速逻辑）
 * 
 * 逻辑说明：
 * 1. 首次按下：目标速度 = baseSpeed_（基础速度）
 * 2. 持续按下：目标速度 += speedIncrement_（累积加速）
 * 3. 最高限制：目标速度 <= maxSpeed_
 * 4. 方向切换：重置为基础速度
 */
void RemoteControl::handleCommand(char command) {
    // 更新时间戳
    lastCommandTime_ = HAL_GetTick();
    
    // 检测指令是否改变（方向切换）
    bool commandChanged = (lastCommand_ != command);
    lastCommand_ = command;

    // 根据指令执行动作（累积加速逻辑）
    switch (command) {
        case 'F':  // 前进
            if (commandChanged || currentTargetStraightSpeed_ <= 0) {
                // 首次按下或方向切换：设置为基础速度
                currentTargetStraightSpeed_ = baseSpeed_;
            } else {
                // 持续按下：累积加速
                currentTargetStraightSpeed_ += speedIncrement_;
                if (currentTargetStraightSpeed_ > maxSpeed_) {
                    currentTargetStraightSpeed_ = maxSpeed_;
                }
            }
            currentTargetTurnSpeed_ = 0;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'B':  // 后退
            if (commandChanged || currentTargetStraightSpeed_ >= 0) {
                // 首次按下或方向切换：设置为基础速度（负）
                currentTargetStraightSpeed_ = -baseSpeed_;
            } else {
                // 持续按下：累积加速（负方向）
                currentTargetStraightSpeed_ -= speedIncrement_;
                if (currentTargetStraightSpeed_ < -maxSpeed_) {
                    currentTargetStraightSpeed_ = -maxSpeed_;
                }
            }
            currentTargetTurnSpeed_ = 0;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'L':  // 左转
            if (commandChanged || currentTargetTurnSpeed_ >= 0) {
                // 首次按下或方向切换：设置为基础速度（负）
                currentTargetTurnSpeed_ = -baseSpeed_;
            } else {
                // 持续按下：累积加速（负方向）
                currentTargetTurnSpeed_ -= speedIncrement_;
                if (currentTargetTurnSpeed_ < -maxSpeed_) {
                    currentTargetTurnSpeed_ = -maxSpeed_;
                }
            }
            currentTargetStraightSpeed_ = 0;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'R':  // 右转
            if (commandChanged || currentTargetTurnSpeed_ <= 0) {
                // 首次按下或方向切换：设置为基础速度
                currentTargetTurnSpeed_ = baseSpeed_;
            } else {
                // 持续按下：累积加速
                currentTargetTurnSpeed_ += speedIncrement_;
                if (currentTargetTurnSpeed_ > maxSpeed_) {
                    currentTargetTurnSpeed_ = maxSpeed_;
                }
            }
            currentTargetStraightSpeed_ = 0;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'U':  // 上（直接最大速度）
            currentTargetStraightSpeed_ = maxSpeed_;
            currentTargetTurnSpeed_ = 0;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'D':  // 下（保留，暂无功能）
            break;

        case 'W':  // 左上（前进+左转）
            if (commandChanged || currentTargetStraightSpeed_ <= 0) {
                currentTargetStraightSpeed_ = baseSpeed_;
            } else {
                currentTargetStraightSpeed_ += speedIncrement_;
                if (currentTargetStraightSpeed_ > maxSpeed_) {
                    currentTargetStraightSpeed_ = maxSpeed_;
                }
            }
            currentTargetTurnSpeed_ = -(currentTargetStraightSpeed_ * turnSensitivity_) / 100;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'X':  // 左下（后退+左转）
            if (commandChanged || currentTargetStraightSpeed_ >= 0) {
                currentTargetStraightSpeed_ = -baseSpeed_;
            } else {
                currentTargetStraightSpeed_ -= speedIncrement_;
                if (currentTargetStraightSpeed_ < -maxSpeed_) {
                    currentTargetStraightSpeed_ = -maxSpeed_;
                }
            }
            currentTargetTurnSpeed_ = (currentTargetStraightSpeed_ * turnSensitivity_) / 100;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'Y':  // 右上（前进+右转）
            if (commandChanged || currentTargetStraightSpeed_ <= 0) {
                currentTargetStraightSpeed_ = baseSpeed_;
            } else {
                currentTargetStraightSpeed_ += speedIncrement_;
                if (currentTargetStraightSpeed_ > maxSpeed_) {
                    currentTargetStraightSpeed_ = maxSpeed_;
                }
            }
            currentTargetTurnSpeed_ = (currentTargetStraightSpeed_ * turnSensitivity_) / 100;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        case 'Z':  // 右下（后退+右转）
            if (commandChanged || currentTargetStraightSpeed_ >= 0) {
                currentTargetStraightSpeed_ = -baseSpeed_;
            } else {
                currentTargetStraightSpeed_ -= speedIncrement_;
                if (currentTargetStraightSpeed_ < -maxSpeed_) {
                    currentTargetStraightSpeed_ = -maxSpeed_;
                }
            }
            currentTargetTurnSpeed_ = -(currentTargetStraightSpeed_ * turnSensitivity_) / 100;
            driveTrain_.setTargetSpeed(currentTargetStraightSpeed_, currentTargetTurnSpeed_);
            isMoving_ = true;
            break;

        default:
            // 未知指令，忽略
            break;
    }
}

/**
 * @brief 检查超时
 */
void RemoteControl::checkTimeout() {
    if (isMoving_) {
        uint32_t currentTime = HAL_GetTick();

        // 检查是否超时
        if (currentTime - lastCommandTime_ >= timeout_) {
            stop();
        }
    }
}

/**
 * @brief 停止（设置目标速度为0，让小车平滑减速）
 */
void RemoteControl::stop() {
    // 清零累积速度
    currentTargetStraightSpeed_ = 0;
    currentTargetTurnSpeed_ = 0;
    
    // 设置目标速度为0，让梯形速度轮廓平滑减速
    driveTrain_.setTargetSpeed(0, 0);
    isMoving_ = false;
}
