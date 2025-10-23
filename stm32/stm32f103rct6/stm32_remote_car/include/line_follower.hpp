/**
 * @file    line_follower.hpp
 * @brief   抛物线拟合法巡线控制器
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 使用抛物线拟合算法实现高精度线位置检测，不依赖阈值判断
 * 支持动态位置计算、PID控制、丢线处理等功能
 */

#ifndef LINE_FOLLOWER_HPP
#define LINE_FOLLOWER_HPP

#include "line_sensor.hpp"
#include "drive_train.hpp"
#include <cmath>

/**
 * @class LineFollower
 * @brief 抛物线拟合法巡线控制器
 */
class LineFollower {
public:
    /**
     * @brief 线的类型
     */
    enum class LineMode {
        WHITE_LINE_ON_BLACK,  // 黑底白线（默认）
        BLACK_LINE_ON_WHITE   // 白底黑线
    };
    
    /**
     * @brief 动态缩放曲线类型
     */
    enum class ScaleCurve {
        LINEAR,      // 线性：均匀增长（推荐）
        QUADRATIC,   // 二次方：小偏差温和，大偏差激进
        SQRT,        // 平方根：小偏差激进，大偏差温和
        CUBIC        // 三次方：更激进的响应
    };
    
    /**
     * @brief 巡线状态
     */
    enum class Status {
        ON_LINE,       // 在线上
        LOST_LINE,     // 丢线
        CROSS_ROAD,    // 十字路口
        STOPPED        // 停止
    };

    /**
     * @brief 构造函数
     * @param sensor 传感器引用
     * @param drive 差速驱动引用
     */
    LineFollower(LineSensor& sensor, DriveTrain& drive);
    
    // ========== 配置接口 ==========
    
    /**
     * @brief 设置PID参数
     * @param kp 比例系数
     * @param ki 积分系数
     * @param kd 微分系数
     */
    void setPID(float kp, float ki, float kd);
    
    /**
     * @brief 设置基础速度
     * @param base_speed 基础速度 (0-100)
     */
    void setSpeed(int16_t base_speed);
    
    /**
     * @brief 设置线模式
     * @param mode 线模式（黑底白线或白底黑线）
     */
    void setLineMode(LineMode mode);
    
    /**
     * @brief 启用调试输出
     * @param enable true=启用，false=禁用
     */
    void enableDebug(bool enable);
    
    /**
     * @brief 设置PID输出缩放系数
     * @param scale 缩放系数 (0.01 - 1.0)，默认0.1
     *              越小越温和，越大越激进
     */
    void setPIDOutputScale(float scale);
    
    /**
     * @brief 启用动态缩放（根据偏差自动调整转向强度）
     * @param enable true=启用动态缩放，false=使用固定缩放
     */
    void enableDynamicScale(bool enable);
    
    /**
     * @brief 设置动态缩放参数
     * @param small_scale 小偏差时的缩放系数（位置接近0时）
     * @param large_scale 大偏差时的缩放系数（位置接近1000时）
     */
    void setDynamicScaleRange(float small_scale, float large_scale);
    
    /**
     * @brief 设置动态缩放曲线类型
     * @param curve 曲线类型
     */
    void setScaleCurve(ScaleCurve curve);
    
    /**
     * @brief 设置丢线搜索参数
     * @param search_speed 搜索基础速度 (5-20，默认10)
     * @param max_turn_speed 最大搜索转向速度 (0-20，默认10)
     */
    void setLostLineSearchParams(int16_t search_speed, int16_t max_turn_speed);
    
    // ========== 控制接口 ==========
    
    /**
     * @brief 启动巡线
     */
    void start();
    
    /**
     * @brief 停止巡线
     */
    void stop();
    
    /**
     * @brief 更新巡线控制（需在主循环中定期调用，建议20ms一次）
     */
    void update();
    
    // ========== 状态查询 ==========
    
    /**
     * @brief 获取当前线位置
     * @return 位置 (-1000 到 +1000)
     */
    float getPosition() const { return position_; }
    
    /**
     * @brief 获取当前误差
     * @return 误差值
     */
    float getError() const { return last_error_; }
    
    /**
     * @brief 获取当前状态
     * @return 巡线状态
     */
    Status getStatus() const { return status_; }
    
    /**
     * @brief 是否在线上
     * @return true=在线上，false=丢线
     */
    bool isOnLine() const { return status_ == Status::ON_LINE; }
    
    /**
     * @brief 获取PID输出值
     * @return PID输出
     */
    float getPIDOutput() const { return pid_output_; }

private:
    // ========== 核心算法 ==========
    
    /**
     * @brief 抛物线拟合法计算线位置（核心算法）
     * @param sensor_data 传感器数据数组
     * @return 线的精确位置 (-1000 到 +1000)
     */
    float calculateLinePositionParabolic(uint16_t sensor_data[8]);
    
    /**
     * @brief 检测巡线状态
     * @param sensor_data 传感器数据
     * @return 当前状态
     */
    Status detectStatus(uint16_t sensor_data[8]);
    
    /**
     * @brief PID控制计算
     * @param error 误差
     * @param dt 时间间隔（ms）
     * @return PID输出
     */
    float computePID(float error, uint32_t dt);
    
    /**
     * @brief 丢线处理
     */
    void handleLostLine();
    
    /**
     * @brief 重置PID状态
     */
    void resetPID();

    // ========== 成员变量 ==========
    
    // 引用
    LineSensor& sensor_;
    DriveTrain& drive_;
    
    // PID参数
    float kp_ = 0.06f;
    float ki_ = 0.0f;
    float kd_ = 1.0f;
    
    // 运行参数
    int16_t base_speed_ = 35;
    LineMode line_mode_ = LineMode::WHITE_LINE_ON_BLACK;
    bool debug_enabled_ = false;
    bool running_ = false;
    float pid_output_scale_ = 0.1f;  // PID输出缩放系数（默认0.1，非常温和）
    
    // 动态缩放参数
    bool dynamic_scale_enabled_ = true;        // 默认启用动态缩放
    float small_error_scale_ = 0.03f;          // 小偏差时的缩放（温和）
    float large_error_scale_ = 0.15f;          // 大偏差时的缩放（激进）
    ScaleCurve scale_curve_ = ScaleCurve::LINEAR;  // 默认线性曲线
    
    // 丢线搜索参数
    int16_t lost_line_search_speed_ = 10;      // 丢线搜索速度（默认极低速）
    int16_t lost_line_max_turn_speed_ = 10;    // 丢线最大转向速度（默认很温和）
    
    // 状态变量
    Status status_ = Status::STOPPED;
    float position_ = 0.0f;
    float last_error_ = 0.0f;
    float integral_ = 0.0f;
    float derivative_filtered_ = 0.0f;
    float last_valid_position_ = 0.0f;
    float pid_output_ = 0.0f;
    uint32_t last_update_time_ = 0;
    
    // 调试用：归一化后的传感器值（0.0~1.0）
    float normalized_values_[8] = {0.0f};
    
    // 传感器位置常量
    static constexpr float SENSOR_POSITIONS[8] = {
        -1000.0f, -714.0f, -428.0f, -142.0f,
         142.0f,   428.0f,  714.0f,  1000.0f
    };
    
    static constexpr float SENSOR_SPACING = 286.0f;  // 平均间距
    static constexpr float ADC_MAX = 4095.0f;        // ADC最大值
};

#endif  // LINE_FOLLOWER_HPP