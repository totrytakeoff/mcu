/*============================================================================
 *                        中断处理函数 - CMake从零开始项目
 *============================================================================*/

#include "main.h"

/* 外部变量声明 */
extern uint32_t system_tick;

/**
 * @brief  不可屏蔽中断处理函数
 */
void NMI_Handler(void)
{
    /* 用户代码 */
}

/**
 * @brief  硬件错误中断处理函数
 */
void HardFault_Handler(void)
{
    while (1)
    {
        /* 硬件错误处理 */
    }
}

/**
 * @brief  内存管理错误中断处理函数
 */
void MemManage_Handler(void)
{
    while (1)
    {
        /* 内存管理错误处理 */
    }
}

/**
 * @brief  总线错误中断处理函数
 */
void BusFault_Handler(void)
{
    while (1)
    {
        /* 总线错误处理 */
    }
}

/**
 * @brief  使用错误中断处理函数
 */
void UsageFault_Handler(void)
{
    while (1)
    {
        /* 使用错误处理 */
    }
}

/**
 * @brief  系统调用中断处理函数
 */
void SVC_Handler(void)
{
    /* 系统调用处理 */
}

/**
 * @brief  调试监视器中断处理函数
 */
void DebugMon_Handler(void)
{
    /* 调试监视器处理 */
}

/**
 * @brief  PendSV中断处理函数
 */
void PendSV_Handler(void)
{
    /* PendSV处理 */
}

/**
 * @brief  系统滴答定时器中断处理函数
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}