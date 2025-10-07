/*============================================================================
 *                        HAL MSP配置 - CMake从零开始项目
 *============================================================================*/

#include "main.h"

/**
 * @brief  HAL库初始化MSP
 */
void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* 系统中断优先级配置 */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* 系统异常中断优先级配置 */
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
    HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
    HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
    HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
    HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
 * @brief  TIM基本MSP初始化
 * @param  htim TIM句柄指针
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim)
{
    if(htim->Instance == TIM2)
    {
        /* 使能TIM2时钟 */
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

/**
 * @brief  TIM基本MSP反初始化
 * @param  htim TIM句柄指针
 */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim)
{
    if(htim->Instance == TIM2)
    {
        /* 禁用TIM2时钟 */
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
}