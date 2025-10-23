/**
 * @file    drive_train.hpp
 * @brief   四电机小车差速转向控制系统
 * @author  AI Assistant
 * @date    2024
 */

#ifndef DRIVE_TRAIN_HPP
#define DRIVE_TRAIN_HPP

#include "../include/motor.hpp"
#include "../include/motion_profile.hpp"

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
     * @brief 立即驱动（无梯形速度轮廓，实时生效）
     * @param straightSpeed 前进/后退速度 (-100 到 100)
     * @param turnSpeed 转向速度 (-100 到 100)
     *
     * 说明：绕过 MotionProfile，直接将组合后的左右轮速度应用到电机。
     *       仅用于需要毫秒级响应的闭环控制（如巡线）。
     */
    void driveImmediate(int straightSpeed, int turnSpeed);

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

    /**
     * @brief 设置转向灵敏度（影响转向指令占比）
     * @param sensitivity 0.0 - 1.5，默认0.8
     */
    void setTurnSensitivity(float sensitivity);

    /**
     * @brief 设置最小前进速度底线（防止大转向时单侧停转）
     * @param floor 百分比 0-100，建议 5-15
     */
    void setMinForwardFloor(int floor);

private:
    Motor leftFrontMotor_;
    Motor leftBackMotor_;
    Motor rightFrontMotor_;
    Motor rightBackMotor_;

    bool initialized_ = false;

    // 速度剖面（直行与转向）
    MotionProfile motionStraight_;
    MotionProfile motionTurn_;

    // 可调参数
    float turn_sensitivity_ = 0.8f;   // 转向灵敏度
    int min_forward_floor_ = 0;       // 最小前进速度底线（0表示关闭）

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
     * @brief 将当前速度应用到电机
     */
    void applySpeedToMotors();
};

#endif // DRIVE_TRAIN_HPP


