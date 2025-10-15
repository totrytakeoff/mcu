/**
 * @file    uart.h
 * @brief   UART 串口通信接口
 * @author  AI Assistant
 * @date    2024
 */

#ifndef __UART_H__
#define __UART_H__

/**
 * @brief 初始化 UART (9600 波特率)
 * @note  适用于 11.0592MHz 晶振
 */
void UART_Init(void);

/**
 * @brief 发送单个字符
 * @param ch 要发送的字符
 */
void UART_SendChar(unsigned char ch);

/**
 * @brief 发送字符串
 * @param str 要发送的字符串 (以 '\0' 结尾)
 */
void UART_SendString(unsigned char *str);

#endif // __UART_H__
