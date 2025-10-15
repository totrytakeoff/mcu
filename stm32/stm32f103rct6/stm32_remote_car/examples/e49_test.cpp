/**
 * @file    e49_test.cpp
 * @brief   E49_Wireless 类测试示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 测试功能：
 * 1. E49 模块初始化
 * 2. 发送测试数据
 * 3. 接收数据回显
 * 4. LED 指示接收状态
 */

#include "stm32f1xx_hal.h"
#include "../include/common.h"
#include "../include/gpio.h"
#include "../include/usart.h"
#include "../include/e49_wireless.hpp"

// 全局 E49 对象
E49_Wireless g_wireless;

// 接收缓冲区
uint8_t rxByte;

/**
 * @brief UART 接收完成回调（由 HAL 库调用）
 */
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 将接收数据传递给 E49 对象处理
        g_wireless.onDataReceived(rxByte);
        
        // 继续接收下一个字节
        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}

/**
 * @brief 主函数
 */
extern "C" int main(void)
{
    /* HAL 初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 外设初始化 */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    /* E49 模块初始化 */
    g_wireless.init();
    
    /* 注册接收回调 */
    g_wireless.setDataReceivedCallback([](uint8_t data) {
        // 回显接收到的数据（用于测试）
        g_wireless.send(data);
        
        // 可选：LED 闪烁指示收到数据
        // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    });
    
    /* 启动 UART 接收中断 */
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    
    /* 发送启动消息 */
    if (g_wireless.isReady())
    {
        g_wireless.sendString("E49 Wireless Ready!\r\n");
    }
    else
    {
        g_wireless.sendString("E49 Not Ready!\r\n");
    }
    
    /* 主循环 */
    uint32_t lastSendTime = 0;
    while (1)
    {
        // 每秒发送一次心跳消息
        if (HAL_GetTick() - lastSendTime > 1000)
        {
            g_wireless.sendString("Heartbeat\r\n");
            lastSendTime = HAL_GetTick();
        }
        
        HAL_Delay(10);
    }
}

/**
 * ============================================================================
 * 测试步骤
 * ============================================================================
 * 
 * 1. 硬件连接测试：
 *    - 烧录程序到 STM32
 *    - 打开串口工具，连接到 USART1
 *    - 应该看到 "E49 Wireless Ready!" 消息
 *    - 每秒应该收到 "Heartbeat" 消息
 * 
 * 2. 回环测试：
 *    - 在串口工具中发送任意字符
 *    - 应该收到相同字符的回显
 *    - 这说明 E49 收发正常
 * 
 * 3. 无线测试：
 *    - 用两块开发板，都烧录此程序
 *    - 板A 发送数据 → 板B 应该收到并回显
 *    - 验证无线通信正常
 * 
 * 4. 遥控器测试（如果有遥控器）：
 *    - 按遥控器按键
 *    - 应该在串口工具中看到对应字符（'F', 'B', 'L', 'R'等）
 * 
 * ============================================================================
 */
