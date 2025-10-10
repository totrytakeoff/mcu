/**
 * @file    drive_train.cpp
 * @brief   四电机小车差速转向控制系统实现
 * @author  AI Assistant
 * @date    2024
 * 
 * 实现的差速转向算法包含：
 * - Arcade混合算法（直行 + 转向）
 * - 梯形速度轮廓（平滑加减速）
 * - 速度归一化防止溢出
 * - 死区滤波提高稳定性
 * - 反向刹车（施加反向加速度）
 */

#include "../include/drive_train.hpp"
#include "stm32f1xx_hal.h"
#include <algorithm>
#include <cmath>

// 配置常量
namespace {
    constexpr int MIN_SPEED = -100;
    constexpr int MAX_SPEED = 100;
    constexpr int DEADBAND_THRESHOLD = 5;      // 忽略小于此值的输入
    constexpr float TURN_SENSITIVITY = 0.8f;   // 转向灵敏度 (0.0-1.0)
    constexpr float SPOT_TURN_REDUCTION = 0.80f; // 原地转向速度降低系数（光滑地面也转不动时用更低值）
    constexpr int MIN_SPOT_TURN_SPEED = 25;    // 原地转向最小有效速度
}

/**
 * @brief 构造函数 - 使用4个电机初始化动力系统
 */
DriveTrain::DriveTrain(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack)
    : leftFrontMotor_(leftFront),
      leftBackMotor_(leftBack),
      rightFrontMotor_(rightFront),
      rightBackMotor_(rightBack),
      initialized_(true)
{
}

/**
 * @brief 初始化动力系统电机
 */
void DriveTrain::init(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack)
{
    leftFrontMotor_ = leftFront;
    leftBackMotor_ = leftBack;
    rightFrontMotor_ = rightFront;
    rightBackMotor_ = rightBack;
    initialized_ = true;
}

/**
 * @brief 应用死区滤波消除微小抖动/噪声
 * @param value 输入值
 * @param threshold 死区阈值
 * @return 滤波后的值（死区内返回0）
 */
int DriveTrain::applyDeadband(int value, int threshold)
{
    if (std::abs(value) < threshold) {
        return 0;
    }
    return value;
}

/**
 * @brief 限制值到有效速度范围
 * @param value 输入值
 * @return 限幅后的值（MIN_SPEED 到 MAX_SPEED）
 */
int DriveTrain::clampSpeed(int value)
{
    return std::max(MIN_SPEED, std::min(MAX_SPEED, value));
}

/**
 * @brief 当速度超出限制时进行归一化
 * @param leftSpeed 左侧速度（会被修改）
 * @param rightSpeed 右侧速度（会被修改）
 * 
 * 确保在保持速度比例的同时，将两个速度都控制在有效范围 [-100, 100] 内
 */
void DriveTrain::normalizeSpeed(int& leftSpeed, int& rightSpeed)
{
    // 找到最大绝对值
    int maxAbsSpeed = std::max(std::abs(leftSpeed), std::abs(rightSpeed));
    
    // 如果在限制范围内，无需归一化
    if (maxAbsSpeed <= MAX_SPEED) {
        return;
    }
    
    // 按比例缩放两个速度
    float scaleFactor = static_cast<float>(MAX_SPEED) / static_cast<float>(maxAbsSpeed);
    leftSpeed = static_cast<int>(leftSpeed * scaleFactor);
    rightSpeed = static_cast<int>(rightSpeed * scaleFactor);
}

/**
 * @brief 主驱动函数 - 差速转向控制
 * @param straightSpeed 前进/后退速度 (-100 到 100)
 *                      正值：前进，负值：后退
 * @param turnSpeed 转向速度 (-100 到 100)
 *                  正值：左转，负值：右转
 * 
 * 算法流程：
 * 1. 应用死区滤波去除噪声
 * 2. 使用Arcade算法混合直行和转向速度
 * 3. 归一化防止速度溢出
 * 4. 将速度应用到左右电机组
 */
void DriveTrain::drive(int straightSpeed, int turnSpeed)
{
    if (!initialized_) {
        return;
    }
    
    // 保存当前命令
    straightSpeed_ = straightSpeed;
    turnSpeed_ = turnSpeed;
    
    // 应用死区滤波
    int filteredStraight = applyDeadband(straightSpeed, DEADBAND_THRESHOLD);
    int filteredTurn = applyDeadband(turnSpeed, DEADBAND_THRESHOLD);
    
    // 应用转向灵敏度调整
    int adjustedTurn = static_cast<int>(filteredTurn * TURN_SENSITIVITY);
    
    // 检测是否为原地转向（直行速度接近0）
    bool isSpotTurn = (std::abs(filteredStraight) < 10);
    
    // 差速转向混合算法
    // 左侧: 直行 + 转向 (turnSpeed > 0 时左转)
    // 右侧: 直行 - 转向
    int leftSpeed = filteredStraight + adjustedTurn;
    int rightSpeed = filteredStraight - adjustedTurn;
    
    // 原地转向时降低速度，避免堵转和空转
    if (isSpotTurn && adjustedTurn != 0) {
        leftSpeed = static_cast<int>(leftSpeed * SPOT_TURN_REDUCTION);
        rightSpeed = static_cast<int>(rightSpeed * SPOT_TURN_REDUCTION);
        
        // 确保速度不会太低导致电机无法启动
        if (leftSpeed != 0 && std::abs(leftSpeed) < MIN_SPOT_TURN_SPEED) {
            leftSpeed = (leftSpeed > 0) ? MIN_SPOT_TURN_SPEED : -MIN_SPOT_TURN_SPEED;
        }
        if (rightSpeed != 0 && std::abs(rightSpeed) < MIN_SPOT_TURN_SPEED) {
            rightSpeed = (rightSpeed > 0) ? MIN_SPOT_TURN_SPEED : -MIN_SPOT_TURN_SPEED;
        }
    }
    
    // 归一化速度以保持比例并在范围内
    normalizeSpeed(leftSpeed, rightSpeed);
    
    // 限幅到绝对限制（安全检查）
    leftSpeed = clampSpeed(leftSpeed);
    rightSpeed = clampSpeed(rightSpeed);
    
    // 反转方向（修正前后反向问题）
    leftSpeed = -leftSpeed;
    rightSpeed = -rightSpeed;
    
    // 应用到电机
    // 左侧电机（正速度 = 前进）
    leftFrontMotor_.setSpeed(leftSpeed);
    leftBackMotor_.setSpeed(leftSpeed);
    
    // 右侧电机（需要反向，因为电机安装方向相反）
    // 根据你的 main.cpp，电机2和4前进时需要负值
    rightFrontMotor_.setSpeed(-rightSpeed);
    rightBackMotor_.setSpeed(-rightSpeed);
}

/**
 * @brief 立即停止所有电机
 */
void DriveTrain::stop()
{
    if (!initialized_) {
        return;
    }
    
    leftFrontMotor_.stop();
    leftBackMotor_.stop();
    rightFrontMotor_.stop();
    rightBackMotor_.stop();
    
    // 清零所有速度状态
    straightSpeed_ = 0;
    turnSpeed_ = 0;
    targetStraightSpeed_ = 0;
    targetTurnSpeed_ = 0;
    currentStraightSpeed_ = 0;
    currentTurnSpeed_ = 0;
}

/**
 * @brief 获取当前直行速度指令
 * @return 当前直行速度 (-100 到 100)
 */
int DriveTrain::getStraightSpeed() const
{
    return straightSpeed_;
}

/**
 * @brief 获取当前转向速度指令
 * @return 当前转向速度 (-100 到 100)
 */
int DriveTrain::getTurnSpeed() const
{
    return turnSpeed_;
}

/**
 * @brief 设置目标速度（梯形速度轮廓）
 * @param straightSpeed 目标直行速度 (-100 到 100)
 * @param turnSpeed 目标转向速度 (-100 到 100)
 */
void DriveTrain::setTargetSpeed(int straightSpeed, int turnSpeed)
{
    if (!initialized_) {
        return;
    }
    
    // 限幅目标速度
    targetStraightSpeed_ = clampSpeed(straightSpeed);
    targetTurnSpeed_ = clampSpeed(turnSpeed);
}

/**
 * @brief 更新速度轮廓（需在主循环中定期调用）
 * 
 * 根据梯形速度轮廓算法更新当前速度，实现平滑加减速
 */
void DriveTrain::update()
{
    if (!initialized_) {
        return;
    }
    
    // 检查是否到达更新时间
    uint32_t now = HAL_GetTick();
    if (now - lastUpdateTime_ < updateInterval_) {
        return; // 还没到更新时间
    }
    lastUpdateTime_ = now;
    
    // 更新直行速度分量
    currentStraightSpeed_ = updateSpeedComponent(currentStraightSpeed_, targetStraightSpeed_);
    
    // 更新转向速度分量
    currentTurnSpeed_ = updateSpeedComponent(currentTurnSpeed_, targetTurnSpeed_);
    
    // 将当前速度应用到电机
    applySpeedToMotors();
}

/**
 * @brief 更新单个速度分量（梯形速度轮廓核心算法）
 * @param current 当前速度
 * @param target 目标速度
 * @return 更新后的速度
 * 
 * 算法逻辑：
 * 1. 如果当前速度 = 目标速度，保持不变（匀速阶段）
 * 2. 如果方向相反（反向切换），施加反向加速度快速刹停
 * 3. 如果同向但未达到目标，施加加速度
 * 4. 如果超过目标，施加减速度
 */
int DriveTrain::updateSpeedComponent(int current, int target)
{
    // 已到达目标速度
    if (current == target) {
        return current;
    }
    
    // 判断是否为反向切换（正→负 或 负→正）
    bool isReversing = (current > 0 && target < 0) || (current < 0 && target > 0);
    
    if (isReversing) {
        // 反向切换：施加反向加速度（快速刹停）
        if (current > 0) {
            // 当前正向，需要减速到0
            current = std::max(0, current - reverseDeceleration_);
        } else {
            // 当前负向，需要加速到0
            current = std::min(0, current + reverseDeceleration_);
        }
    }
    else if (std::abs(current) < std::abs(target)) {
        // 加速阶段：当前速度小于目标速度
        if (target > current) {
            current = std::min(target, current + acceleration_);
        } else {
            current = std::max(target, current - acceleration_);
        }
    }
    else {
        // 减速阶段：当前速度大于目标速度
        if (target > current) {
            current = std::min(target, current + deceleration_);
        } else {
            current = std::max(target, current - deceleration_);
        }
    }
    
    return current;
}

/**
 * @brief 将当前速度应用到电机
 * 
 * 使用差速转向算法将直行速度和转向速度混合后应用到左右电机
 */
void DriveTrain::applySpeedToMotors()
{
    // 应用死区滤波
    int filteredStraight = applyDeadband(currentStraightSpeed_, DEADBAND_THRESHOLD);
    int filteredTurn = applyDeadband(currentTurnSpeed_, DEADBAND_THRESHOLD);
    
    // 应用转向灵敏度调整
    int adjustedTurn = static_cast<int>(filteredTurn * TURN_SENSITIVITY);
    
    // 检测是否为原地转向（直行速度接近0）
    bool isSpotTurn = (std::abs(filteredStraight) < 10);
    
    // 差速转向混合算法
    // 左侧: 直行 + 转向 (turnSpeed > 0 时左转)
    // 右侧: 直行 - 转向
    int leftSpeed = filteredStraight + adjustedTurn;
    int rightSpeed = filteredStraight - adjustedTurn;
    
    // 原地转向时降低速度，避免堵转和空转
    if (isSpotTurn && adjustedTurn != 0) {
        leftSpeed = static_cast<int>(leftSpeed * SPOT_TURN_REDUCTION);
        rightSpeed = static_cast<int>(rightSpeed * SPOT_TURN_REDUCTION);
        
        // 确保速度不会太低导致电机无法启动
        if (leftSpeed != 0 && std::abs(leftSpeed) < MIN_SPOT_TURN_SPEED) {
            leftSpeed = (leftSpeed > 0) ? MIN_SPOT_TURN_SPEED : -MIN_SPOT_TURN_SPEED;
        }
        if (rightSpeed != 0 && std::abs(rightSpeed) < MIN_SPOT_TURN_SPEED) {
            rightSpeed = (rightSpeed > 0) ? MIN_SPOT_TURN_SPEED : -MIN_SPOT_TURN_SPEED;
        }
    }
    
    // 归一化速度以保持比例并在范围内
    normalizeSpeed(leftSpeed, rightSpeed);
    
    // 限幅到绝对限制（安全检查）
    leftSpeed = clampSpeed(leftSpeed);
    rightSpeed = clampSpeed(rightSpeed);
    
    // 反转方向（修正前后反向问题）
    leftSpeed = -leftSpeed;
    rightSpeed = -rightSpeed;
    
    // 应用到电机
    // 左侧电机（正速度 = 前进）
    leftFrontMotor_.setSpeed(leftSpeed);
    leftBackMotor_.setSpeed(leftSpeed);
    
    // 右侧电机（需要反向，因为电机安装方向相反）
    rightFrontMotor_.setSpeed(-rightSpeed);
    rightBackMotor_.setSpeed(-rightSpeed);
}

/**
 * @brief 设置加速度参数
 * @param acceleration 加速度（每次更新增加的速度值）
 * @param deceleration 减速度（每次更新减少的速度值）
 * @param reverseDeceleration 反向减速度（反向切换时的减速度）
 */
void DriveTrain::setAcceleration(int acceleration, int deceleration, int reverseDeceleration)
{
    acceleration_ = std::max(1, acceleration);
    deceleration_ = std::max(1, deceleration);
    reverseDeceleration_ = std::max(1, reverseDeceleration);
}

/**
 * @brief 设置速度更新间隔
 * @param intervalMs 更新间隔（毫秒）
 */
void DriveTrain::setUpdateInterval(uint32_t intervalMs)
{
    updateInterval_ = std::max(static_cast<uint32_t>(10), intervalMs); // 最小10ms
}
