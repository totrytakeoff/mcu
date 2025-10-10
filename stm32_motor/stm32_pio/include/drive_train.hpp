/**
 * @file    drive_train.hpp
 * @brief   四电机小车差速转向控制系统
 * @author  AI Assistant
 * @date    2024
 */

#ifndef DRIVE_TRAIN_HPP
#define DRIVE_TRAIN_HPP

#include "../include/motor.hpp"

class DriveTrain {
public:
    DriveTrain() = delete;

    /**
     * @brief 构造函数 - 使用4个电机初始化动力系统
     * @param leftFront 左前电机
     * @param leftBack 左后电机
     * @param rightFront 右前电机
     * @param rightBack 右后电机
     */
    explicit DriveTrain(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack);

    /**
     * @brief 初始化动力系统电机（构造函数的替代方法）
     */
    void init(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack);

    /**
     * @brief 差速转向驱动函数（已弃用，保留兼容性）
     * @param straightSpeed 前进/后退速度 (-100 到 100)
     *                      正值前进，负值后退
     * @param turnSpeed 转向速度 (-100 到 100, 默认0)
     *                  正值左转，负值右转
     * 
     * 使用示例:
     * - drive(50, 0)    -> 半速前进
     * - drive(50, 30)   -> 前进同时左转
     * - drive(0, 50)    -> 原地左旋
     * - drive(-50, -30) -> 后退同时右转
     */
    void drive(int straightSpeed, int turnSpeed = 0);

    /**
     * @brief 设置目标速度（梯形速度轮廓）
     * @param straightSpeed 目标直行速度 (-100 到 100)
     * @param turnSpeed 目标转向速度 (-100 到 100)
     * 
     * 实际速度会按照梯形速度轮廓平滑过渡到目标速度
     */
    void setTargetSpeed(int straightSpeed, int turnSpeed = 0);

    /**
     * @brief 更新速度轮廓（需在主循环中定期调用）
     * 
     * 根据梯形速度轮廓算法更新当前速度，实现平滑加减速
     * 建议调用频率：每 10-20ms 调用一次
     */
    void update();

    /**
     * @brief 设置加速度参数
     * @param acceleration 加速度（每次更新增加的速度值，默认5）
     * @param deceleration 减速度（每次更新减少的速度值，默认8）
     * @param reverseDeceleration 反向减速度（反向切换时的减速度，默认12）
     */
    void setAcceleration(int acceleration, int deceleration, int reverseDeceleration);

    /**
     * @brief 设置速度更新间隔
     * @param intervalMs 更新间隔（毫秒，默认20ms）
     */
    void setUpdateInterval(uint32_t intervalMs);

    /**
     * @brief 紧急停止 - 立即停止所有电机
     */
    void stop();

    /**
     * @brief 获取当前直行速度指令
     * @return 当前直行速度 (-100 到 100)
     */
    int getStraightSpeed() const;

    /**
     * @brief 获取当前转向速度指令
     * @return 当前转向速度 (-100 到 100)
     */
    int getTurnSpeed() const;

private:
    Motor leftFrontMotor_;
    Motor leftBackMotor_;
    Motor rightFrontMotor_;
    Motor rightBackMotor_;

    bool initialized_ = false;

    // 目标速度（-100 到 100）
    int targetStraightSpeed_ = 0;  // 目标直行速度
    int targetTurnSpeed_ = 0;      // 目标转向速度

    // 当前实际速度（-100 到 100）
    int currentStraightSpeed_ = 0; // 当前直行速度
    int currentTurnSpeed_ = 0;     // 当前转向速度

    // 梯形速度轮廓参数
    int acceleration_ = 5;          // 加速度（每次更新增加的速度）
    int deceleration_ = 8;          // 减速度（每次更新减少的速度）
    int reverseDeceleration_ = 12; // 反向减速度（反向切换时）

    // 时间控制
    uint32_t lastUpdateTime_ = 0;  // 上次更新时间（毫秒）
    uint32_t updateInterval_ = 20; // 更新间隔（毫秒，默认20ms = 50Hz）

    // 旧版兼容（已弃用）
    int straightSpeed_ = 0;
    int turnSpeed_ = 0;

    /**
     * @brief 应用死区滤波消除噪声
     * @param value 输入值
     * @param threshold 死区阈值
     * @return 滤波后的值
     */
    int applyDeadband(int value, int threshold);

    /**
     * @brief 限制速度到有效范围 [-100, 100]
     * @param value 输入速度
     * @return 限幅后的速度
     */
    int clampSpeed(int value);

    /**
     * @brief 当超出限制时按比例归一化速度
     * @param leftSpeed 左侧速度（会被修改）
     * @param rightSpeed 右侧速度（会被修改）
     */
    void normalizeSpeed(int& leftSpeed, int& rightSpeed);

    /**
     * @brief 更新单个速度分量（梯形速度轮廓核心算法）
     * @param current 当前速度
     * @param target 目标速度
     * @return 更新后的速度
     */
    int updateSpeedComponent(int current, int target);

    /**
     * @brief 将当前速度应用到电机
     */
    void applySpeedToMotors();
};

#endif // DRIVE_TRAIN_HPP


