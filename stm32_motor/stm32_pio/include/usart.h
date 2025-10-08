/**
 * @file    usart.h
 * @brief   USART1串口配置（用于E49无线模块通信）
 * @author  AI Assistant
 * @date    2024
 */

#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* 全局UART句柄 */
extern UART_HandleTypeDef huart1;

/**
 * @brief USART1初始化
 * 配置：9600波特率，8位数据，1位停止位，无校验
 */
void MX_USART1_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
