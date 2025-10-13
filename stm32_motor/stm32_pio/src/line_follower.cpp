/**
 * @file    line_follower.cpp
 * @brief   巡线控制类实现
 * @author  AI Assistant
 * @date    2024
 */

#include "line_follower.hpp"
#include "stm32f1xx_hal.h"
#include <cmath>

/**
 * @brief 构造函数
 */
LineFollower::LineFollower(LineSensor& sensor, DriveTrain& driveTrain)
    : sensor_(sensor)
    , driveTrain_(driveTrain)
    , baseSpeed_(40)
    , kp_(0.1f)
    , ki_(0.0f)
    , kd_(1.0f)
    , running_(false)
    , error_(0)
    , lastError_(0)
    , integral_(0.0f)
    , output_(0.0f)
    , lostLineHandling_(true)
    , lastPosition_(0)
    , lostLineTime_(0)
    , crossroadCallback_(nullptr)
{
}

/**
 * @brief 析构函数
 */
LineFollower::~LineFollower()
{
    stop();
}

/**
 * @brief 初始化巡线控制器
 */
void LineFollower::init()
{
    // 确保传感器已初始化
    sensor_.update();
    
    // 重置状态
    error_ = 0;
    lastError_ = 0;
    integral_ = 0.0f;
    output_ = 0.0f;
    lastPosition_ = 0;
}

/**
 * @brief 更新巡线控制
 */
void LineFollower::update()
{
    if (!running_) {
        return;
    }
    
    // 1. 读取传感器
    sensor_.update();
    
    // 2. 检查十字路口
    if (sensor_.isCrossroad()) {
        handleCrossroad();
        return;
    }
    
    // 3. 获取线条位置
    int16_t position = sensor_.getPosition();
    
    // 4. 检查是否丢线
    if (position == INT16_MIN) {
        handleLostLine();
        return;
    }
    
    // 5. 记录有效位置
    lastPosition_ = position;
    lostLineTime_ = 0;
    
    // 6. 计算偏差（期望位置为 0）
    error_ = position;
    
    // 7. 计算 PID 输出
    output_ = calculatePID(error_);
    
    // 8. 限制输出范围（-100 ~ +100）
    if (output_ > 100.0f) output_ = 100.0f;
    if (output_ < -100.0f) output_ = -100.0f;
    
    // 9. 计算左右轮速度
    // output > 0 表示线在右侧，需要右转（右轮减速）
    // output < 0 表示线在左侧，需要左转（左轮减速）
    int leftSpeed = baseSpeed_ - (int)output_;
    int rightSpeed = baseSpeed_ + (int)output_;
    
    // 10. 控制电机
    driveTrain_.drive(leftSpeed, rightSpeed);
    
    // 11. 更新上次偏差
    lastError_ = error_;
}

/**
 * @brief 设置基础速度
 */
void LineFollower::setSpeed(int speed)
{
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;
    baseSpeed_ = speed;
}

/**
 * @brief 设置 PID 参数
 */
void LineFollower::setPID(float kp, float ki, float kd)
{
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}

/**
 * @brief 启动巡线
 */
void LineFollower::start()
{
    resetIntegral();
    running_ = true;
}

/**
 * @brief 停止巡线
 */
void LineFollower::stop()
{
    running_ = false;
    driveTrain_.stop();
    resetIntegral();
}

/**
 * @brief 暂停巡线
 */
void LineFollower::pause()
{
    running_ = false;
    driveTrain_.stop();
}

/**
 * @brief 恢复巡线
 */
void LineFollower::resume()
{
    running_ = true;
}

/**
 * @brief 设置丢线处理模式
 */
void LineFollower::setLostLineHandling(bool enable)
{
    lostLineHandling_ = enable;
}

/**
 * @brief 设置十字路口处理回调
 */
void LineFollower::setCrossroadCallback(bool (*callback)(void))
{
    crossroadCallback_ = callback;
}

/**
 * @brief 重置 PID 积分项
 */
void LineFollower::resetIntegral()
{
    integral_ = 0.0f;
}

/**
 * @brief 计算 PID 输出
 */
float LineFollower::calculatePID(int16_t error)
{
    // P: 比例项
    float p = kp_ * error;
    
    // I: 积分项（防止积分饱和）
    integral_ += error;
    if (integral_ > 10000.0f) integral_ = 10000.0f;
    if (integral_ < -10000.0f) integral_ = -10000.0f;
    float i = ki_ * integral_;
    
    // D: 微分项
    float derivative = error - lastError_;
    float d = kd_ * derivative;
    
    // PID 输出
    return p + i + d;
}

/**
 * @brief 处理丢线情况
 */
void LineFollower::handleLostLine()
{
    if (!lostLineHandling_) {
        // 丢线停车模式
        stop();
        return;
    }
    
    // 丢线搜索模式
    if (lostLineTime_ == 0) {
        lostLineTime_ = HAL_GetTick();
    }
    
    uint32_t lostDuration = HAL_GetTick() - lostLineTime_;
    
    if (lostDuration < 500) {
        // 短时间丢线：按照上次方向继续搜索
        if (lastPosition_ < 0) {
            // 上次在左侧，向左转搜索
            driveTrain_.drive(-baseSpeed_ / 2, baseSpeed_ / 2);
        } else {
            // 上次在右侧，向右转搜索
            driveTrain_.drive(baseSpeed_ / 2, -baseSpeed_ / 2);
        }
    } else {
        // 长时间丢线：停车
        stop();
    }
}

/**
 * @brief 处理十字路口
 */
void LineFollower::handleCrossroad()
{
    if (crossroadCallback_ != nullptr) {
        // 调用用户回调
        bool continueFollowing = crossroadCallback_();
        if (!continueFollowing) {
            stop();
        }
    } else {
        // 默认行为：直行穿过
        driveTrain_.drive(baseSpeed_, baseSpeed_);
    }
}
