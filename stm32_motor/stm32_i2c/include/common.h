/**
 * @file    common.h
 * @brief   公共头文件 - 包含所有模块共用的声明
 * @author  AI Assistant
 * @date    2024
 * 
 * 本文件包含所有模块都需要的公共函数声明和头文件包含
 */

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* 公共函数声明 */

/**
 * @brief  错误处理函数
 * @note   当系统出现错误时调用此函数
 * @retval None
 */
void Error_Handler(void);

/**
 * @brief  系统时钟配置函数
 * @note   配置系统时钟为72MHz
 * @retval None
 */
void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */
