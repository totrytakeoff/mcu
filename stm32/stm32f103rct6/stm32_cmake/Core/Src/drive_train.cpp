#include "../Inc/drive_train.hpp"
#include "drive_train.hpp"
#include <algorithm> // for std::clamp
#include <cmath>     // for abs

DriveTrain::DriveTrain(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack)
        : leftFrontMotor_(leftFront)
        , leftBackMotor_(leftBack)
        , rightFrontMotor_(rightFront)
        , rightBackMotor_(rightBack)
        , initialized_(true)
        , straightSpeed_(0)
        , turnSpeed_(0) {}

void DriveTrain::init(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack) {
    leftBackMotor_ = leftBack;
    leftFrontMotor_ = leftFront;
    rightBackMotor_ = rightBack;
    rightFrontMotor_ = rightFront;
    initialized_ = true;
    straightSpeed_ = 0;
    turnSpeed_ = 0;
}

/**
 * @brief 差速转向控制主函数
 * @param straightSpeed 直线速度 (-100 到 100)
 * @param turnSpeed 转向速度 (-100 到 100)
 * 
 * 差速转向算法：
 * - 左轮速度 = straightSpeed - turnSpeed
 * - 右轮速度 = straightSpeed + turnSpeed
 * 
 * 当转向速度为正时，车辆右转（左轮速度降低，右轮速度增加）
 * 当转向速度为负时，车辆左转（左轮速度增加，右轮速度降低）
 */
void DriveTrain::drive(int straightSpeed, int turnSpeed) {
    if (!initialized_) {
        return;
    }

    // 限制速度范围在 -100 到 100 之间
    straightSpeed = (straightSpeed < -100) ? -100 : (straightSpeed > 100) ? 100 : straightSpeed;
    turnSpeed = (turnSpeed < -100) ? -100 : (turnSpeed > 100) ? 100 : turnSpeed;

    // 保存当前设置
    straightSpeed_ = straightSpeed;
    turnSpeed_ = turnSpeed;

    // 计算差速后的轮速
    int leftSpeed = straightSpeed - turnSpeed;
    int rightSpeed = straightSpeed + turnSpeed;

    // 限制轮速范围在 -100 到 100 之间
    leftSpeed = (leftSpeed < -100) ? -100 : (leftSpeed > 100) ? 100 : leftSpeed;
    rightSpeed = (rightSpeed < -100) ? -100 : (rightSpeed > 100) ? 100 : rightSpeed;

    // 应用速度到电机
    leftFrontMotor_.setSpeed(leftSpeed);
    leftBackMotor_.setSpeed(leftSpeed);
    rightFrontMotor_.setSpeed(rightSpeed);
    rightBackMotor_.setSpeed(rightSpeed);
}

/**
 * @brief 纯转向控制
 * @param turnSpeed 转向速度 (-100 到 100)
 * 
 * 实现原地转向功能：
 * - 左轮速度 = -turnSpeed
 * - 右轮速度 = turnSpeed
 */
void DriveTrain::turn(int turnSpeed) {
    if (!initialized_) {
        return;
    }

    // 限制速度范围在 -100 到 100 之间
    turnSpeed = (turnSpeed < -100) ? -100 : (turnSpeed > 100) ? 100 : turnSpeed;

    // 保存当前设置
    turnSpeed_ = turnSpeed;
    straightSpeed_ = 0;

    // 原地转向：左轮和右轮速度相反
    leftFrontMotor_.setSpeed(-turnSpeed);
    leftBackMotor_.setSpeed(-turnSpeed);
    rightFrontMotor_.setSpeed(turnSpeed);
    rightBackMotor_.setSpeed(turnSpeed);
}

/**
 * @brief 停止所有电机
 */
void DriveTrain::stop() {
    if (!initialized_) {
        return;
    }
    
    leftFrontMotor_.stop();
    leftBackMotor_.stop();
    rightFrontMotor_.stop();
    rightBackMotor_.stop();
    
    // 重置速度状态
    straightSpeed_ = 0;
    turnSpeed_ = 0;
}

/**
 * @brief 获取当前直线速度
 * @return 直线速度 (-100 到 100)
 */
int DriveTrain::getStraightSpeed() const {
    return straightSpeed_;
}

/**
 * @brief 获取当前转向速度
 * @return 转向速度 (-100 到 100)
 */
int DriveTrain::getTurnSpeed() const {
    return turnSpeed_;
}

/**
 * @brief 获取左轮当前速度
 * @return 左轮速度 (-100 到 100)
 */
int DriveTrain::getLeftSpeed() const {
    return straightSpeed_ - turnSpeed_;
}

/**
 * @brief 获取右轮当前速度
 * @return 右轮速度 (-100 到 100)
 */
int DriveTrain::getRightSpeed() const {
    return straightSpeed_ + turnSpeed_;
}

/**
 * @brief 设置最大速度限制
 * @param maxSpeed 最大速度百分比 (0-100)
 */
void DriveTrain::setMaxSpeed(int maxSpeed) {
    if (maxSpeed < 0) maxSpeed = 0;
    if (maxSpeed > 100) maxSpeed = 100;
    
    // 重新计算当前速度限制
    int currentLeftSpeed = getLeftSpeed();
    int currentRightSpeed = getRightSpeed();
    
    float scale = static_cast<float>(maxSpeed) / 100.0f;
    
    int newStraightSpeed = static_cast<int>(straightSpeed_ * scale);
    int newTurnSpeed = static_cast<int>(turnSpeed_ * scale);
    
    drive(newStraightSpeed, newTurnSpeed);
}

/**
 * @brief 平滑转向控制
 * @param targetStraightSpeed 目标直线速度
 * @param targetTurnSpeed 目标转向速度
 * @param smoothingFactor 平滑因子 (0-1, 值越小越平滑)
 */
void DriveTrain::smoothDrive(int targetStraightSpeed, int targetTurnSpeed, float smoothingFactor) {
    if (!initialized_) {
        return;
    }

    // 限制速度范围
    targetStraightSpeed = (targetStraightSpeed < -100) ? -100 : (targetStraightSpeed > 100) ? 100 : targetStraightSpeed;
    targetTurnSpeed = (targetTurnSpeed < -100) ? -100 : (targetTurnSpeed > 100) ? 100 : targetTurnSpeed;

    // 平滑插值
    smoothingFactor = (smoothingFactor < 0.0f) ? 0.0f : (smoothingFactor > 1.0f) ? 1.0f : smoothingFactor;
    
    int currentStraight = straightSpeed_;
    int currentTurn = turnSpeed_;
    
    int newStraight = currentStraight + static_cast<int>((targetStraightSpeed - currentStraight) * smoothingFactor);
    int newTurn = currentTurn + static_cast<int>((targetTurnSpeed - currentTurn) * smoothingFactor);
    
    drive(newStraight, newTurn);
}

/**
 * @brief 弧线运动控制
 * @param speed 基础速度
 * @param turnRadius 转向半径 (正值为右转，负值为左转，0为原地转向)
 */
void DriveTrain::arcDrive(int speed, int turnRadius) {
    if (!initialized_) {
        return;
    }

    // 限制速度范围
    speed = (speed < -100) ? -100 : (speed > 100) ? 100 : speed;
    
    // 根据转向半径计算转向速度
    // 转向半径越小，转向速度越大
    // 使用非线性映射，使小半径转向更灵敏
    int turnSpeed = 0;
    if (turnRadius != 0) {
        // 将转向半径映射到转向速度
        // 假设转向半径范围 -1000 到 1000，映射到 -100 到 100
        float normalizedRadius = static_cast<float>(turnRadius) / 10.0f;
        turnSpeed = static_cast<int>(100.0f / (1.0f + abs(normalizedRadius) / 10.0f));
        
        // 根据转向半径正负确定转向方向
        if (turnRadius < 0) {
            turnSpeed = -turnSpeed;
        }
    }
    
    drive(speed, turnSpeed);
}
