/**
 * @file    remote_control.hpp
 * @brief   遥控器控制类 - 解析遥控指令并控制小车运动
 * @author  AI Assistant
 * @date    2024
 */

#ifndef REMOTE_CONTROL_HPP
#define REMOTE_CONTROL_HPP

#include "stm32f1xx_hal.h"
#include "drive_train.hpp"
#include "e49_wireless.hpp"

/**
 * @class RemoteControl
 * @brief 遥控器控制类
 * 
 * 功能：
 * - 接收并解析遥控器指令
 * - 将指令转换为小车运动控制
 * - 实现安全超时停止机制
 */
class RemoteControl {
public:
    /**
     * @brief 构造函数
     * @param driveTrain 差速转向系统引用
     * @param wireless E49无线模块引用
     */
    RemoteControl(DriveTrain& driveTrain, E49_Wireless& wireless);
    
    /**
     * @brief 析构函数
     */
    ~RemoteControl();
    
    /**
     * @brief 初始化遥控器控制
     */
    void init();
    
    /**
     * @brief 更新函数（在主循环中调用）
     * 检查超时并自动停止
     */
    void update();
    
    /**
     * @brief 设置超时时间
     * @param timeout_ms 超时时间（毫秒）
     */
    void setTimeout(uint32_t timeout_ms);
    
    /**
     * @brief 设置基础速度（最低速度）
     * @param speed 基础速度值 (0-100)
     */
    void setBaseSpeed(int speed);
    
    /**
     * @brief 设置最高速度限制
     * @param speed 最高速度值 (0-100)
     */
    void setMaxSpeed(int speed);
    
    /**
     * @brief 设置速度增量（每次指令增加的速度）
     * @param increment 速度增量 (0-100)
     */
    void setSpeedIncrement(int increment);
    
    /**
     * @brief 设置转向灵敏度
     * @param sensitivity 灵敏度 (0-100)
     */
    void setTurnSensitivity(int sensitivity);
    
    /**
     * @brief 获取最后接收到的指令
     * @return 最后的指令字符
     */
    char getLastCommand() const;
    
    /**
     * @brief 获取是否正在运动
     * @return true=运动中, false=停止
     */
    bool isMoving() const;

private:
    DriveTrain& driveTrain_;        // 差速转向系统引用
    E49_Wireless& wireless_;        // 无线模块引用
    
    int baseSpeed_;                 // 基础速度（最低速度，0-100）
    int maxSpeed_;                  // 最高速度限制 (0-100)
    int speedIncrement_;            // 速度增量（每次指令增加的速度，0-100）
    int turnSensitivity_;           // 转向灵敏度 (0-100)
    uint32_t timeout_;              // 超时时间 (ms)
    
    char lastCommand_;              // 最后接收到的指令
    uint32_t lastCommandTime_;      // 最后接收指令的时间戳
    bool isMoving_;                 // 是否正在运动
    
    // 当前目标速度（累积）
    int currentTargetStraightSpeed_;  // 当前目标直行速度
    int currentTargetTurnSpeed_;      // 当前目标转向速度
    
    /**
     * @brief 处理接收到的指令
     * @param command 指令字符
     */
    void handleCommand(char command);
    
    /**
     * @brief 检查并处理超时
     */
    void checkTimeout();
    
    /**
     * @brief 停止所有运动
     */
    void stop();
    
    /**
     * @brief 静态回调函数（用于E49数据接收）
     * @param data 接收到的数据
     */
    static void dataReceivedCallback(uint8_t data);
    
    // 静态实例指针（用于回调）
    static RemoteControl* instance_;
};

#endif // REMOTE_CONTROL_HPP
