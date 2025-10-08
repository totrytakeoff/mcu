/**
 * @file    usart.c
 * @brief   USART1串口配置实现（HAL库）
 * @author  AI Assistant
 * @date    2024
 * 
 * 硬件连接（基于原理图）：
 * - PA9  (USART1_TX) -> E49 RXD
 * - PA10 (USART1_RX) -> E49 TXD
 * 
 * 通信参数：
 * - 波特率：9600
 * - 数据位：8位
 * - 停止位：1位
 * - 校验位：无
 */

#include "usart.h"
#include "common.h"

UART_HandleTypeDef huart1;

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
}

/**
 * @brief USART1 MSP反初始化回调
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
}
