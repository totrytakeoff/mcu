/**
 * @file    line_follower_pid.hpp
 * @brief   基于PID控制器的巡线类
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 使用加权算法计算线位置误差，通过PID控制器计算速度差，实现精确的巡线控制
 * 
 * 特性：
 * - 使用通用PID控制器
 * - 加权算法计算线位置
 * - 支持黑底白线和白底黑线
 * - 自动阈值判断
 * - 调试输出支持
 * 
 * 使用示例：
 * @code
 * LineSensor sensor;
 * Motor motor_lf, motor_lb, motor_rf, motor_rb;
 * LineFollowerPID follower(sensor, motor_lf, motor_lb, motor_rf, motor_rb);
 * 
 * follower.setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);
 * follower.setPID(0.06f, 0.0f, 1.0f);
 * follower.setBaseSpeed(30);
 * follower.start();
 * 
 * while(1) {
 *     follower.update();
 *     HAL_Delay(20);
 * }
 * @endcode
 */

#ifndef LINE_FOLLOWER_PID_HPP
#define LINE_FOLLOWER_PID_HPP

#include "line_sensor.hpp"
#include "motor.hpp"
#include "pid_controller.hpp"
#include <stdint.h>

class LineFollowerPID {
public:
    /**
     * @brief 线模式（使用LineSensor中的定义）
     */
    using LineMode = LineSensor::LineMode;

    /**
     * @brief 巡线状态
     */
    enum class State {
        STOPPED = 0,     // 停止
        RUNNING = 1,     // 正常运行
        LINE_LOST = 2    // 丢线
    };

public:
    /**
     * @brief 构造函数
     * @param sensor 线传感器对象
     * @param motor_lf 左前电机
     * @param motor_lb 左后电机
     * @param motor_rf 右前电机
     * @param motor_rb 右后电机
     */
    LineFollowerPID(LineSensor& sensor,
                    Motor& motor_lf, Motor& motor_lb,
                    Motor& motor_rf, Motor& motor_rb);

    /**
     * @brief 初始化巡线系统
     */
    void init();

    /**
     * @brief 更新巡线控制（需在主循环中定期调用，建议20ms一次）
     */
    void update();

    /**
     * @brief 启动巡线
     */
    void start();

    /**
     * @brief 停止巡线
     */
    void stop();

    /**
     * @brief 设置PID参数
     * @param kp 比例系数
     * @param ki 积分系数
     * @param kd 微分系数
     */
    void setPID(float kp, float ki, float kd);

    /**
     * @brief 设置基础速度
     * @param speed 基础速度 (0-100)
     */
    void setBaseSpeed(int speed);

    /**
     * @brief 设置线模式
     * @param mode 黑底白线或白底黑线
     */
    void setLineMode(LineMode mode);

    /**
     * @brief 设置传感器阈值
     * @param threshold 判断黑白的阈值 (0-4095)
     */
    void setThreshold(uint16_t threshold);

    /**
     * @brief 设置丢线阈值（多少传感器检测到线）
     * @param min_sensors 最少传感器数量 (默认1)
     */
    void setLineLostThreshold(int min_sensors);

    /**
     * @brief 启用/禁用调试输出
     * @param enable true=启用，false=禁用
     */
    void enableDebug(bool enable);

    /**
     * @brief 反转位置符号（当传感器物理方向与权重定义相反时启用）
     */
    void setInvertPosition(bool invert) { invert_position_ = invert; }

    /**
     * @brief 获取当前状态
     * @return 巡线状态
     */
    State getState() const { return state_; }

    /**
     * @brief 获取当前线位置
     * @return 线位置（-1000 ~ +1000，0为中心）
     */
    float getPosition() const { return last_position_; }

    /**
     * @brief 获取当前线位置误差
     * @return 误差值 (-1000 到 1000)
     */
    float getError() const { return error_; }

    /**
     * @brief 获取PID输出
     * @return PID输出值
     */
    float getPIDOutput() const { return pid_output_; }

    /**
     * @brief 获取当前左侧速度
     * @return 左侧速度
     */
    int getLeftSpeed() const { return left_speed_; }

    /**
     * @brief 获取当前右侧速度
     * @return 右侧速度
     */
    int getRightSpeed() const { return right_speed_; }

    /**
     * @brief 获取最近一次更新时的原始传感器数据（用于显示）
     */
    void getLastSensorData(uint16_t out[8]) const;

    /**
     * @brief 获取最近一次更新时的二值化结果（用于显示）
     */
    void getLastBinaryData(bool out[8]) const;

    /**
     * @brief 重置PID控制器
     */
    void resetPID();

    // ========== 可调参数接口 ==========

    /**
     * @brief 设置控制参数（可调）
     * @param max_adjustment_ratio 最大调整幅度比例（默认0.5，即±50%）
     * @param min_speed_ratio 最小速度比例（默认0.3，即30%）
     * @param max_speed_ratio 最大速度比例（默认1.5，即150%）
     * @param pid_output_ratio PID输出限制比例（默认0.6，即60%）
     */
    void setControlParameters(float max_adjustment_ratio = 0.5f,
                              float min_speed_ratio = 0.3f,
                              float max_speed_ratio = 1.5f,
                              float pid_output_ratio = 0.6f);

    /**
     * @brief 设置非线性映射参数（可调）
     * @param small_threshold 小偏差阈值比例（默认0.2）
     * @param medium_threshold 中等偏差阈值比例（默认0.5）
     * @param large_threshold 大偏差阈值比例（默认0.8）
     * @param small_gain 小偏差增益（默认0.1）
     * @param medium_gain 中等偏差增益（默认0.3）
     * @param large_gain 大偏差增益（默认0.6）
     */
    void setNonLinearParameters(float small_threshold = 0.2f,
                                float medium_threshold = 0.5f,
                                float large_threshold = 0.8f,
                                float small_gain = 0.1f,
                                float medium_gain = 0.3f,
                                float large_gain = 0.6f);

    /**
     * @brief 获取当前控制参数
     */
    void getControlParameters(float& max_adjustment, float& min_speed,
                             float& max_speed, float& pid_output) const;

private:
    // 硬件引用
    LineSensor& sensor_;
    Motor& motor_lf_;
    Motor& motor_lb_;
    Motor& motor_rf_;
    Motor& motor_rb_;

    // PID控制器
    PIDController pid_;

    // 配置参数
    LineMode line_mode_;        // 线模式
    int base_speed_;            // 基础速度
    uint16_t threshold_;        // 黑白判断阈值（0表示使用传感器校准值）
    int line_lost_threshold_;   // 丢线阈值（最少传感器数）
    bool debug_enabled_;        // 调试输出使能

    // 可调参数
    float max_adjustment_ratio_;   // 最大调整幅度比例（默认0.5）
    float min_speed_ratio_;        // 最小速度比例（默认0.3）
    float max_speed_ratio_;        // 最大速度比例（默认1.5）
    float pid_output_ratio_;       // PID输出限制比例（默认0.6）

    // 非线性映射参数
    float small_threshold_;        // 小偏差阈值比例（默认0.2）
    float medium_threshold_;       // 中等偏差阈值比例（默认0.5）
    float large_threshold_;        // 大偏差阈值比例（默认0.8）
    float small_gain_;             // 小偏差增益（默认0.1）
    float medium_gain_;            // 中等偏差增益（默认0.3）
    float large_gain_;             // 大偏差增益（默认0.6）

    // 状态变量
    State state_;               // 当前状态
    float error_;               // 当前误差
    float last_position_;       // 上次线位置（用于丢线处理）
    float pid_output_;          // PID输出
    int left_speed_;            // 左侧速度
    int right_speed_;           // 右侧速度
    uint32_t last_update_time_; // 上次更新时间

    /**
     * @brief 应用速度到电机
     * @param left_speed 左侧速度
     * @param right_speed 右侧速度
     */
    void applySpeed(int left_speed, int right_speed);

    /**
     * @brief 限制速度范围
     * @param speed 速度值
     * @return 限制后的速度 (-100 到 100)
     */
    int clampSpeed(int speed);

    /**
     * @brief 打印调试信息
     * @param sensor_data 传感器数据
     * @param binary_data 二值化数据
     */
    void printDebugInfo(const uint16_t sensor_data[8], const bool binary_data[8]);

    /**
     * @brief 动态更新PID输出限制（基于基础速度）
     */
    void updatePIDOutputLimits();

    /**
     * @brief 双向速度保护函数
     */
    void constrainSpeeds();

    // 最近一次的传感器数据缓存（供显示等使用，避免重复采样）
    uint16_t last_sensor_data_[8] = {0};
    bool last_binary_data_[8] = {false};

    // 上一次的调整系数（用于限幅与平滑，避免左右快速来回抖动）
    float last_adjustment_factor_ = 0.0f;

    // 方向判定（基于线位置的滞回判断）：true=左侧为内侧（应当减速），false=右侧为内侧
    bool last_inner_left_ = true;
    // 方向滞回阈值（单位与position一致，-1000..1000），低阈值用于保持，高阈值用于切换
    float dir_hyst_low_ = 80.0f;
    float dir_hyst_high_ = 150.0f;

    // 是否反转位置符号
    bool invert_position_ = false;

    // 自动方向判定（仅在启动后早期进行一次）
    bool orientation_confirmed_ = false;
    uint8_t orientation_frames_ = 0;
    uint8_t orientation_mismatch_ = 0;
};

#endif // LINE_FOLLOWER_PID_HPP
