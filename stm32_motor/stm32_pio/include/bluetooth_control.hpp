/**
 * @file    bluetooth_control.hpp
 * @brief   蓝牙控制类 - 解析ESP32-S3蓝牙数据并控制小车
 * @author  AI Assistant
 * @date    2025-10-11
 */

#ifndef BLUETOOTH_CONTROL_HPP
#define BLUETOOTH_CONTROL_HPP

#include "stm32f1xx_hal.h"
#include "remote_control.hpp"

/**
 * @class BluetoothControl
 * @brief 蓝牙控制类
 * 
 * 功能：
 * - 接收并解析ESP32-S3蓝牙模块数据
 * - 支持按键模式（单字符命令）
 * - 支持摇杆模式（A[角度]P[力度]格式）
 * - 将解析后的数据传递给RemoteControl处理
 */
class BluetoothControl {
public:
    /**
     * @brief 构造函数
     * @param remoteControl 遥控器控制类引用
     */
    BluetoothControl(RemoteControl& remoteControl);
    
    /**
     * @brief 析构函数
     */
    ~BluetoothControl();
    
    /**
     * @brief 初始化蓝牙控制
     */
    void init();
    
    /**
     * @brief 处理接收到的蓝牙数据
     * @param data 接收到的字节
     */
    void handleData(uint8_t data);
    
    // 从USART中断上下文入队一个字节（非阻塞、ISR安全）
    void enqueueFromISR(uint8_t data);
    
    // 在主循环中调用，消费队列并解析
    void update();
    
    /**
     * @brief 设置摇杆模式使能
     * @param enable true=启用摇杆模式, false=仅按键模式
     */
    void setJoystickMode(bool enable);
    
    /**
     * @brief 获取当前是否为摇杆模式
     * @return true=摇杆模式, false=按键模式
     */
    bool isJoystickMode() const;

private:
    RemoteControl& remoteControl_;  // 遥控器控制引用
    
    // 摇杆模式相关
    bool joystickEnabled_;          // 摇杆模式使能
    uint8_t joystickBuffer_[16];    // 摇杆数据缓冲区
    uint8_t joystickIndex_;         // 当前缓冲区索引
    bool textMode_;                 // 文本模式（忽略非协议行，直到换行）
    uint8_t lineBuffer_[64];        // 行缓冲（非摇杆）
    uint8_t lineIndex_;             // 当前行长度
    
    // --- UART2 字节级队列（ISR 生产，主循环消费） ---
    static const uint16_t kRxQueueSize = 256;
    volatile uint16_t rxqHead_ = 0; // 写入（ISR）
    volatile uint16_t rxqTail_ = 0; // 读取（主循环）
    uint8_t rxQueue_[kRxQueueSize];
    
    /**
     * @brief 处理按键模式数据（单字符）
     * @param data 按键字符
     */
    void handleKeyCommand(uint8_t data);
    
    /**
     * @brief 处理摇杆模式数据（A[角度]P[力度]\n）
     */
    void handleJoystickCommand();
    
    /**
     * @brief 将角度和力度转换为小车控制指令
     * @param angle 角度 (0-359度)
     * @param power 力度 (0-99)
     */
    void convertJoystickToMotion(int angle, int power);
    
    /**
     * @brief 静态实例指针（用于UART回调）
     */
    static BluetoothControl* instance_;
};

#endif // BLUETOOTH_CONTROL_HPP
