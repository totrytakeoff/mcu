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
     * @brief 差速转向驱动函数
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

    // -100 到 100，前正后负
    int straightSpeed_ = 0;
    // -100 到 100，正左负右
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
};

#endif // DRIVE_TRAIN_HPP


