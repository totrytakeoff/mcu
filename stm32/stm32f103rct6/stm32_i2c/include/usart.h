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
extern UART_HandleTypeDef huart2;

/**
 * @brief USART1初始化
 * 配置：9600波特率，8位数据，1位停止位，无校验
 * 用途：E49无线模块通信
 */
void MX_USART1_UART_Init(void);

/**
 * @brief USART2初始化
 * 配置：115200波特率，8位数据，1位停止位，无校验
 * 用途：串口调试输出
 */
void MX_USART2_UART_Init(void);

/**
 * @brief 通过USART2发送字符串（调试用）
 * @param str 要发送的字符串
 */
void USART2_Print(const char* str);

/**
 * @brief 通过USART2发送格式化字符串（类似printf）
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void USART2_Printf(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
