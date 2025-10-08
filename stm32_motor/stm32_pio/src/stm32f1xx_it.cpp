/**
 * @file    stm32f1xx_it.cpp
 * @brief   中断处理函数实现文件
 * @author  AI Assistant
 * @date    2024
 * 
 * 本文件包含所有中断服务程序的实现
 */

#include "../include/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  不可屏蔽中断处理函数
 * @retval None
 */
void NMI_Handler(void)
{
    /* 不可屏蔽中断处理代码 */
}

/**
 * @brief  硬件错误中断处理函数
 * @retval None
 */
void HardFault_Handler(void)
{
    /* 硬件错误处理：进入无限循环 */
    while (1) {
        /* 空循环 */
    }
}

/**
 * @brief  内存管理错误中断处理函数
 * @retval None
 */
void MemManage_Handler(void)
{
    /* 内存管理错误处理：进入无限循环 */
    while (1) {
    }
}

/**
 * @brief  总线错误中断处理函数
 * @retval None
 */
void BusFault_Handler(void)
{
    /* 总线错误处理：进入无限循环 */
    while (1) {
    }
}

/**
 * @brief  用法错误中断处理函数
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* 用法错误处理：进入无限循环 */
    while (1) {
    }
}

/**
 * @brief  SVC中断处理函数
 * @retval None
 */
void SVC_Handler(void)
{
    /* SVC中断处理代码 */
}

/**
 * @brief  调试监视器中断处理函数
 * @retval None
 */
void DebugMon_Handler(void)
{
    /* 调试监视器中断处理代码 */
}

/**
 * @brief  PendSV中断处理函数
 * @retval None
 */
void PendSV_Handler(void)
{
    /* PendSV中断处理代码 */
}

/**
 * @brief  SysTick中断处理函数
 * @retval None
 */
void SysTick_Handler(void)
{
    /* SysTick中断处理：递增系统时钟计数器 */
    HAL_IncTick();
}

#ifdef __cplusplus
}
#endif
