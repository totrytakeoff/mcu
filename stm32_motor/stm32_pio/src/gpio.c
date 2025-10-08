/**
 * @file    gpio.c
 * @brief   GPIO configuration implementation
 * @author  Migrated from stm32_cmake project
 * @date    2024
 * 
 * Basic GPIO initialization - enables all GPIO port clocks
 * Motor control pins are configured by TIM peripheral (PWM)
 */

#include "gpio.h"

/**
 * @brief GPIO Initialization Function
 * @note Enables clocks for all GPIO ports
 *       Specific pin configurations are handled by other peripherals
 *       (e.g., TIM for PWM, USART for serial, etc.)
 */
void MX_GPIO_Init(void)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Note: GPIO configuration for motor control is handled by TIM peripheral */
}
