/**
 * @file    usart.c
 * @brief   USART串口配置实现（HAL库）
 * @author  AI Assistant
 * @date    2024
 * 
 * 硬件连接：
 * USART1 (E49无线模块通信):
 * - PA9  (USART1_TX) -> E49 RXD
 * - PA10 (USART1_RX) -> E49 TXD
 * - 波特率：9600, 8N1
 * 
 * USART2 (调试串口输出):
 * - PA2  (USART2_TX) -> USB转TTL RXD
 * - PA3  (USART2_RX) -> USB转TTL TXD
 * - 波特率：115200, 8N1
 */

#include "usart.h"
#include "common.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/**
 * @brief USART1初始化
 */
void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;                    // 与遥控器匹配
    huart1.Init.WordLength = UART_WORDLENGTH_8B;    // 8位数据
    huart1.Init.StopBits = UART_STOPBITS_1;         // 1位停止位
    huart1.Init.Parity = UART_PARITY_NONE;          // 无校验
    huart1.Init.Mode = UART_MODE_TX_RX;             // 收发模式
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;    // 无流控
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief USART2初始化
 */
void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;                  // 调试用，速度快
    huart2.Init.WordLength = UART_WORDLENGTH_8B;    // 8位数据
    huart2.Init.StopBits = UART_STOPBITS_1;         // 1位停止位
    huart2.Init.Parity = UART_PARITY_NONE;          // 无校验
    huart2.Init.Mode = UART_MODE_TX_RX;             // 收发模式
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;    // 无流控
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief USART1 MSP初始化回调
 * @param uartHandle UART句柄
 */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(uartHandle->Instance == USART1)
    {
        /* 使能时钟 */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        /**
         * USART1 GPIO配置
         * PA9  ------> USART1_TX (发送到E49)
         * PA10 ------> USART1_RX (从E49接收)
         */
        
        /* TX引脚 - 复用推挽输出 */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        /* RX引脚 - 浮空输入 */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        /* USART1 中断配置 */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
    else if(uartHandle->Instance == USART2)
    {
        /* 使能时钟 */
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        /**
         * USART2 GPIO配置
         * PA2  ------> USART2_TX (调试输出)
         * PA3  ------> USART2_RX (调试输入，可选)
         */
        
        /* TX引脚 - 复用推挽输出 */
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        /* RX引脚 - 浮空输入 */
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        /* USART2 中断配置（如果需要接收） */
        HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

/**
 * @brief USART MSP反初始化回调
 * @param uartHandle UART句柄
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    if(uartHandle->Instance == USART1)
    {
        /* 禁用外设时钟 */
        __HAL_RCC_USART1_CLK_DISABLE();
        
        /* 反初始化GPIO */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
        
        /* 禁用中断 */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
    else if(uartHandle->Instance == USART2)
    {
        /* 禁用外设时钟 */
        __HAL_RCC_USART2_CLK_DISABLE();
        
        /* 反初始化GPIO */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
        
        /* 禁用中断 */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
}

/**
 * @brief 通过USART2发送字符串（调试用）
 * @param str 要发送的字符串
 */
void USART2_Print(const char* str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);
}

/**
 * @brief 通过USART2发送格式化字符串（类似printf）
 * @param format 格式化字符串
 * @param ... 可变参数
 * 
 * 使用示例：
 *   USART2_Printf("温度: %d°C, 湿度: %d%%\r\n", temp, humi);
 *   USART2_Printf("收到指令: %c\r\n", cmd);
 */
void USART2_Printf(const char* format, ...)
{
    char buffer[256];  // 缓冲区，根据需要调整大小
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    USART2_Print(buffer);
}
