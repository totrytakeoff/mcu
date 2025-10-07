/*============================================================================
 *                        HAL MSP Configuration - PlatformIO版本
 *============================================================================*/

#include "main.h"

/**
 * @brief  HAL库初始化MSP
 * @param  None
 * @retval None
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
 * @brief  TIM2 MSP初始化
 * @param  htim: TIM handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim)
{
    if(htim->Instance==TIM2)
    {
        /* 使能TIM2时钟 */
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

/**
 * @brief  TIM2 MSP反初始化
 * @param  htim: TIM handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim)
{
    if(htim->Instance==TIM2)
    {
        /* 禁用TIM2时钟 */
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
}

/**
 * @brief  TIM2 PWM MSP后初始化
 * @param  htim: TIM handle pointer
 * @retval None
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(htim->Instance==TIM2)
    {
        /* 使能GPIOA时钟 */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        /**TIM2 GPIO Configuration
        PA0     ------> TIM2_CH1 (舵机控制)
        PA1     ------> TIM2_CH2 (LED控制)
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}