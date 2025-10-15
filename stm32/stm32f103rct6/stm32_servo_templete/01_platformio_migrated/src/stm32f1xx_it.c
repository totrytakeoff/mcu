/*============================================================================
 *                        中断处理函数 - PlatformIO版本
 *============================================================================*/

#include "main.h"
#include "stm32f1xx_it.h"

/**
 * @brief  不可屏蔽中断处理函数
 */
void NMI_Handler(void)
{
}

/**
 * @brief  硬件错误中断处理函数
 */
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  内存管理错误中断处理函数
 */
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  总线错误中断处理函数
 */
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  使用错误中断处理函数
 */
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  系统调用中断处理函数
 */
void SVC_Handler(void)
{
}

/**
 * @brief  调试监视器中断处理函数
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  PendSV中断处理函数
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  系统滴答定时器中断处理函数
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}