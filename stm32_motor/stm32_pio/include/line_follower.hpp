/**
 * @file    line_follower.hpp
 * @brief   巡线控制类 - PID 算法
 * @author  AI Assistant
 * @date    2024
 * 
 * @usage   LineFollower follower(sensor, driveTrain);
 *          follower.init();
 *          follower.setSpeed(50);
 *          follower.update();  // 在主循环中调用
 */

#ifndef __LINE_FOLLOWER_HPP
#define __LINE_FOLLOWER_HPP

#include "line_sensor.hpp"
#include "drive_train.hpp"
#include <stdint.h>

/**
 * @class LineFollower
 * @brief 巡线控制器（PID 算法）
 */
class LineFollower
{
public:
    /**
     * @brief 构造函数
     * @param sensor: 灰度传感器对象引用
     * @param driveTrain: 差速转向对象引用
     */
    LineFollower(LineSensor& sensor, DriveTrain& driveTrain);
    
    /**
     * @brief 析构函数
     */
    ~LineFollower();
    
    /**
     * @brief 初始化巡线控制器
     */
    void init();
    
    /**
     * @brief 更新巡线控制（在主循环中调用）
     * @note 建议调用频率：50-100Hz
     */
    void update();
    
    /**
     * @brief 设置基础速度
     * @param speed: 速度值（0-100）
     */
    void setSpeed(int speed);
    
    /**
     * @brief 获取当前基础速度
     */
    int getSpeed() const { return baseSpeed_; }
    
    /**
     * @brief 设置 PID 参数
     * @param kp: 比例系数（推荐 0.05-0.2）
     * @param ki: 积分系数（推荐 0.0-0.01）
     * @param kd: 微分系数（推荐 0.5-2.0）
     */
    void setPID(float kp, float ki, float kd);
    
    /**
     * @brief 启动巡线
     */
    void start();
    
    /**
     * @brief 停止巡线
     */
    void stop();
    
    /**
     * @brief 暂停巡线（保持当前状态）
     */
    void pause();
    
    /**
     * @brief 恢复巡线
     */
    void resume();
    
    /**
     * @brief 检查是否正在运行
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief 获取当前位置偏差
     */
    int16_t getError() const { return error_; }
    
    /**
     * @brief 获取 PID 输出值
     */
    float getOutput() const { return output_; }
    
    /**
     * @brief 设置丢线处理模式
     * @param enable: true=启用丢线搜索, false=丢线时停车
     */
    void setLostLineHandling(bool enable);
    
    /**
     * @brief 设置十字路口处理回调
     * @param callback: 回调函数（返回 true 表示继续巡线）
     */
    void setCrossroadCallback(bool (*callback)(void));
    
    /**
     * @brief 重置 PID 积分项
     */
    void resetIntegral();
    
private:
    LineSensor& sensor_;          // 传感器引用
    DriveTrain& driveTrain_;      // 差速转向引用
    
    // 控制参数
    int baseSpeed_;               // 基础速度（0-100）
    float kp_;                    // PID 比例系数
    float ki_;                    // PID 积分系数
    float kd_;                    // PID 微分系数
    
    // 状态变量
    bool running_;                // 是否运行中
    int16_t error_;               // 当前偏差
    int16_t lastError_;           // 上次偏差
    float integral_;              // 积分累积
    float output_;                // PID 输出
    
    // 丢线处理
    bool lostLineHandling_;       // 是否启用丢线搜索
    int16_t lastPosition_;        // 上次检测到的位置
    uint32_t lostLineTime_;       // 丢线时间戳
    
    // 十字路口处理
    bool (*crossroadCallback_)(void);  // 十字路口回调函数
    
    // 内部方法
    float calculatePID(int16_t error);
    void handleLostLine();
    void handleCrossroad();
};

#endif /* __LINE_FOLLOWER_HPP */
