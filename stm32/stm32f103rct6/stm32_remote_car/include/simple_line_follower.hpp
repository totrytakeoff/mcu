/**
 * @file    simple_line_follower.hpp
 * @brief   简易双传感器巡线系统 - 基于梯度分级控制
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 使用传感器0（最左）和传感器7（最右）实现简化巡线控制
 * 不依赖 DriveTrain，直接控制 4 个电机速度
 * 基于归一化百分比的梯度分级，平滑控制转向
 * 
 * @usage
 *   SimpleLineFollower follower(sensor, motor1, motor2, motor3, motor4);
 *   follower.setBaseSpeed(20);
 *   follower.setLineMode(SimpleLineFollower::LineMode::WHITE_LINE_ON_BLACK);
 *   
 *   while (1) {
 *       follower.update();
 *       HAL_Delay(20);
 *   }
 */

#ifndef __SIMPLE_LINE_FOLLOWER_HPP
#define __SIMPLE_LINE_FOLLOWER_HPP

#include <stdint.h>
#include "line_sensor.hpp"
#include "motor.hpp"

/**
 * @class SimpleLineFollower
 * @brief 简易巡线控制器（双边缘传感器）
 */
class SimpleLineFollower {
public:
    /**
     * @brief 巡线模式
     */
    enum class LineMode {
        WHITE_LINE_ON_BLACK,  ///< 黑底白线（传感器检测到白线时值较高）
        BLACK_LINE_ON_WHITE   ///< 白底黑线（传感器检测到黑线时值较高）
    };
    
    /**
     * @brief 运行状态（11级精细分级）
     */
    enum class Status {
        STRAIGHT,           ///< 居中直行
        TURN_LEFT_TINY,     ///< 极微左偏（1级）
        TURN_LEFT_SOFT,     ///< 轻微左偏（2级）
        TURN_LEFT_MID,      ///< 中度左偏（3级）
        TURN_LEFT_HARD,     ///< 大幅左偏（4级）
        TURN_LEFT_SHARP,    ///< 急左转（直角）
        TURN_RIGHT_TINY,    ///< 极微右偏（1级）
        TURN_RIGHT_SOFT,    ///< 轻微右偏（2级）
        TURN_RIGHT_MID,     ///< 中度右偏（3级）
        TURN_RIGHT_HARD,    ///< 大幅右偏（4级）
        TURN_RIGHT_SHARP,   ///< 急右转（直角）
        LOST_LINE,          ///< 丢线/路口
        STOPPED             ///< 停止状态
    };
    
    /**
     * @brief 构造函数
     * @param sensor 传感器对象引用
     * @param fl 前左电机
     * @param fr 前右电机
     * @param rl 后左电机
     * @param rr 后右电机
     */
    SimpleLineFollower(LineSensor& sensor, 
                       Motor& fl, Motor& fr, 
                       Motor& rl, Motor& rr);
    
    /**
     * @brief 初始化
     */
    void init();
    
    /**
     * @brief 更新巡线控制（在主循环中调用）
     */
    void update();
    
    /**
     * @brief 设置巡线模式
     * @param mode 黑底白线 或 白底黑线
     */
    void setLineMode(LineMode mode);
    
    /**
     * @brief 设置基础速度
     * @param speed 基础速度 (0-100)
     */
    void setBaseSpeed(int speed);
    
    /**
     * @brief 获取当前状态
     * @return Status 当前运行状态
     */
    Status getStatus() const { return status_; }
    
    /**
     * @brief 获取左传感器归一化值
     * @return float 归一化百分比 (0.0-100.0)
     */
    float getLeftNormalized() const { return left_normalized_; }
    
    /**
     * @brief 获取右传感器归一化值
     * @return float 归一化百分比 (0.0-100.0)
     */
    float getRightNormalized() const { return right_normalized_; }
    
    /**
     * @brief 停止所有电机
     */
    void stop();
    
    /**
     * @brief 启用/禁用调试输出
     * @param enable true=启用, false=禁用
     */
    void enableDebug(bool enable) { debug_enabled_ = enable; }
    
    /**
     * @brief 设置速度梯度参数（4级精细调节）
     * @param tiny 极微偏离时的速度增量 (默认 2)
     * @param soft 轻微偏离时的速度增量 (默认 5)
     * @param mid 中度偏离时的速度增量 (默认 10)
     * @param hard 大幅偏离时的速度增量 (默认 18)
     */
    void setSpeedGradient(int tiny, int soft, int mid, int hard);
    
    /**
     * @brief 设置阈值参数（高级）
     * @param lost_threshold 丢线判定阈值（两侧都低于此值）默认 15%
     * @param sharp_turn_threshold 急转弯判定阈值（单侧低于此值）默认 15%
     * @param on_line_threshold 在线判定阈值（单侧高于此值）默认 70%
     */
    void setThresholds(float lost_threshold, float sharp_turn_threshold, float on_line_threshold);
    
    /**
     * @brief 设置直角转弯参数
     * @param duration 直角转弯持续时间（ms）默认 500ms
     * @note 在检测到直角后，会锁定转向动作一段时间，避免中途误判
     */
    void setSharpTurnDuration(uint32_t duration);
    
private:
    /**
     * @brief 归一化传感器数据为百分比
     * @param sensor_idx 传感器索引 (0 或 7)
     * @param raw_value 原始传感器值
     * @return float 归一化百分比 (0.0-100.0)
     */
    float normalizeValue(uint8_t sensor_idx, uint16_t raw_value);
    
    /**
     * @brief 分析传感器数据并决定状态
     */
    void analyzeState();
    
    /**
     * @brief 根据状态设置电机速度
     */
    void applyMotorControl();
    
    /**
     * @brief 处理丢线情况
     */
    void handleLostLine();
    
    /**
     * @brief 调试输出
     */
    void debugPrint();
    
    // 引用对象
    LineSensor& sensor_;
    Motor& motor_fl_;  ///< 前左电机
    Motor& motor_fr_;  ///< 前右电机
    Motor& motor_rl_;  ///< 后左电机
    Motor& motor_rr_;  ///< 后右电机
    
    // 配置参数
    LineMode line_mode_;
    int base_speed_;
    
    // 梯度速度增量（4级）
    int speed_tiny_;   ///< 极微偏离增量
    int speed_soft_;   ///< 轻微偏离增量
    int speed_mid_;    ///< 中度偏离增量
    int speed_hard_;   ///< 大幅偏离增量
    
    // 阈值参数（百分比）
    float threshold_lost_;        ///< 丢线阈值
    float threshold_sharp_turn_;  ///< 急转弯阈值
    float threshold_on_line_;     ///< 在线阈值
    
    // 运行时数据
    uint16_t white_cal_[8];   ///< 白色校准值
    uint16_t black_cal_[8];   ///< 黑色校准值
    float left_normalized_;   ///< 左传感器归一化值(%)
    float right_normalized_;  ///< 右传感器归一化值(%)
    Status status_;           ///< 当前状态
    Status last_status_;      ///< 上次状态（用于丢线恢复）
    
    // 丢线恢复
    uint32_t lost_line_start_time_;  ///< 丢线开始时间戳
    uint32_t lost_line_duration_;    ///< 丢线持续时间（ms，默认3000）
    
    // 直角转弯状态
    uint32_t sharp_turn_start_time_;  ///< 直角转弯开始时间戳
    uint32_t sharp_turn_duration_;    ///< 直角转弯持续时间（ms，默认500）
    bool sharp_turn_active_;          ///< 直角转弯是否激活
    Status sharp_turn_direction_;     ///< 记录转弯方向
    
    // 调试
    bool debug_enabled_;
    uint32_t update_count_;   ///< 更新计数器
};

#endif // __SIMPLE_LINE_FOLLOWER_HPP
