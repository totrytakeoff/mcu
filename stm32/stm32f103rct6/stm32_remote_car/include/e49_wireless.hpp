/**
 * @file    e49_wireless.hpp
 * @brief   E49-400T20S 无线模块 C++ 封装类
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能：
 * - GPIO 配置（M0/M1/AUX）
 * - 工作模式切换（透传/配置/省电/唤醒）
 * - 数据收发（基于 USART）
 * - 状态检查（AUX 引脚）
 * - 接收回调机制
 * 
 * 硬件连接（基于原理图）：
 * - PA6  -> E49 M0  (模式选择位0)
 * - PA7  -> E49 M1  (模式选择位1)
 * - PA12 -> E49 AUX (辅助状态指示)
 */

#ifndef E49_WIRELESS_HPP
#define E49_WIRELESS_HPP

#include "stm32f1xx_hal.h"
#include <cstdint>
#include <functional>

class E49_Wireless {
public:
    /**
     * @brief E49 工作模式枚举
     * 
     * M1 M0 | 模式
     * ------|--------
     * 0  0  | 透传模式（正常通信）
     * 0  1  | 唤醒模式
     * 1  0  | 省电模式
     * 1  1  | 配置模式（AT命令）
     */
    enum class Mode {
        Transparent = 0,  // 透传模式 (M0=0, M1=0)
        Wakeup      = 1,  // 唤醒模式 (M0=1, M1=0)
        PowerSave   = 2,  // 省电模式 (M0=0, M1=1)
        Config      = 3   // 配置模式 (M0=1, M1=1)
    };
    
    /**
     * @brief 构造函数
     */
    E49_Wireless();
    
    /**
     * @brief 析构函数
     */
    ~E49_Wireless();
    
    // ========== 基础功能 ==========
    
    /**
     * @brief 初始化 E49 模块
     * 配置 GPIO，设置为透传模式，等待模块就绪
     */
    void init();
    
    /**
     * @brief 设置工作模式
     * @param mode 目标模式
     */
    void setMode(Mode mode);
    
    /**
     * @brief 检查模块是否就绪
     * @return true - 就绪（AUX 为高电平），false - 未就绪
     */
    bool isReady() const;
    
    /**
     * @brief 等待模块就绪（阻塞）
     * @param timeout_ms 超时时间（毫秒），默认 1000ms
     * @return true - 就绪，false - 超时
     */
    bool waitReady(uint32_t timeout_ms = 1000);
    
    // ========== 数据发送 ==========
    
    /**
     * @brief 发送单字节数据
     * @param data 要发送的字节
     */
    void send(uint8_t data);
    
    /**
     * @brief 发送多字节数据
     * @param data 数据缓冲区指针
     * @param length 数据长度
     */
    void send(const uint8_t* data, uint16_t length);
    
    /**
     * @brief 发送字符串
     * @param str 以 null 结尾的字符串
     */
    void sendString(const char* str);
    
    // ========== 数据接收 ==========
    
    /**
     * @brief 设置数据接收回调函数
     * @param callback 回调函数，参数为接收到的字节
     * 
     * 示例：
     * wireless.setDataReceivedCallback([](uint8_t data) {
     *     // 处理接收到的数据
     * });
     */
    void setDataReceivedCallback(std::function<void(uint8_t)> callback);
    
    /**
     * @brief 数据接收处理（由中断调用）
     * @param data 接收到的字节
     * @note 此函数应在 UART 接收中断回调中调用
     */
    void onDataReceived(uint8_t data);
    
    // ========== 状态查询 ==========
    
    /**
     * @brief 获取当前工作模式
     * @return 当前模式
     */
    Mode getMode() const;
    
private:
    // ========== GPIO 引脚定义 ==========
    static constexpr uint16_t M0_PIN  = GPIO_PIN_6;
    static constexpr uint16_t M1_PIN  = GPIO_PIN_7;
    static constexpr uint16_t AUX_PIN = GPIO_PIN_12;
    static GPIO_TypeDef* const GPIO_PORT;
    
    // ========== 成员变量 ==========
    Mode currentMode_;                          // 当前工作模式
    std::function<void(uint8_t)> dataCallback_; // 数据接收回调函数
    
    // ========== 内部辅助函数 ==========
    
    /**
     * @brief 初始化 GPIO 引脚
     */
    void initGPIO();
    
    /**
     * @brief 设置模式引脚电平
     * @param mode 目标模式
     */
    void setModePins(Mode mode);
};

#endif // E49_WIRELESS_HPP
