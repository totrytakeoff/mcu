/**
 * @file    e49_wireless.cpp
 * @brief   E49-400T20S 无线模块实现
 * @author  AI Assistant
 * @date    2024
 */

#include "../include/e49_wireless.hpp"
#include "../include/usart.h"
#include <cstring>

// 静态成员初始化
GPIO_TypeDef* const E49_Wireless::GPIO_PORT = GPIOA;

/**
 * @brief 构造函数
 */
E49_Wireless::E49_Wireless()
    : currentMode_(Mode::Transparent)
    , dataCallback_(nullptr)
{
}

/**
 * @brief 析构函数
 */
E49_Wireless::~E49_Wireless()
{
}

/**
 * @brief 初始化 E49 模块
 */
void E49_Wireless::init()
{
    // 1. 初始化 GPIO
    initGPIO();
    
    // 2. 设置为透传模式
    setMode(Mode::Transparent);
    
    // 3. 等待模块稳定
    HAL_Delay(10);
    
    // 4. 等待模块就绪
    waitReady(1000);
}

/**
 * @brief 初始化 GPIO 引脚
 */
void E49_Wireless::initGPIO()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能 GPIOA 时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // 配置 M0 和 M1 为输出模式（控制工作模式）
    GPIO_InitStruct.Pin = M0_PIN | M1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIO_PORT, &GPIO_InitStruct);
    
    // 配置 AUX 为输入模式（读取模块状态）
    GPIO_InitStruct.Pin = AUX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief 设置工作模式
 */
void E49_Wireless::setMode(Mode mode)
{
    setModePins(mode);
    currentMode_ = mode;
    
    // 模式切换需要时间
    HAL_Delay(10);
}

/**
 * @brief 设置模式引脚电平
 */
void E49_Wireless::setModePins(Mode mode)
{
    switch(mode)
    {
        case Mode::Transparent:  // M0=0, M1=0
            HAL_GPIO_WritePin(GPIO_PORT, M0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIO_PORT, M1_PIN, GPIO_PIN_RESET);
            break;
            
        case Mode::Wakeup:       // M0=1, M1=0
            HAL_GPIO_WritePin(GPIO_PORT, M0_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIO_PORT, M1_PIN, GPIO_PIN_RESET);
            break;
            
        case Mode::PowerSave:    // M0=0, M1=1
            HAL_GPIO_WritePin(GPIO_PORT, M0_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIO_PORT, M1_PIN, GPIO_PIN_SET);
            break;
            
        case Mode::Config:       // M0=1, M1=1
            HAL_GPIO_WritePin(GPIO_PORT, M0_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIO_PORT, M1_PIN, GPIO_PIN_SET);
            break;
    }
}

/**
 * @brief 检查模块是否就绪
 */
bool E49_Wireless::isReady() const
{
    // AUX 引脚为高电平表示模块就绪
    return (HAL_GPIO_ReadPin(GPIO_PORT, AUX_PIN) == GPIO_PIN_SET);
}

/**
 * @brief 等待模块就绪（阻塞）
 */
bool E49_Wireless::waitReady(uint32_t timeout_ms)
{
    uint32_t startTime = HAL_GetTick();
    
    while(!isReady())
    {
        // 检查超时
        if(HAL_GetTick() - startTime > timeout_ms)
        {
            return false;  // 超时
        }
        
        HAL_Delay(10);  // 小延时，避免过于频繁检查
    }
    
    return true;  // 就绪
}

/**
 * @brief 发送单字节数据
 */
void E49_Wireless::send(uint8_t data)
{
    // 通过 UART 发送数据
    HAL_UART_Transmit(&huart1, &data, 1, 100);
}

/**
 * @brief 发送多字节数据
 */
void E49_Wireless::send(const uint8_t* data, uint16_t length)
{
    // 通过 UART 发送数据
    HAL_UART_Transmit(&huart1, const_cast<uint8_t*>(data), length, 1000);
}

/**
 * @brief 发送字符串
 */
void E49_Wireless::sendString(const char* str)
{
    uint16_t length = strlen(str);
    send(reinterpret_cast<const uint8_t*>(str), length);
}

/**
 * @brief 设置数据接收回调函数
 */
void E49_Wireless::setDataReceivedCallback(std::function<void(uint8_t)> callback)
{
    dataCallback_ = callback;
}

/**
 * @brief 数据接收处理（由中断调用）
 */
void E49_Wireless::onDataReceived(uint8_t data)
{
    // 如果设置了回调函数，则调用
    if(dataCallback_)
    {
        dataCallback_(data);
    }
}

/**
 * @brief 获取当前工作模式
 */
E49_Wireless::Mode E49_Wireless::getMode() const
{
    return currentMode_;
}
