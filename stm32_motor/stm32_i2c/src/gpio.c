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
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 配置 LED 调试引脚：PB5, PB12, PB13, PB14（推挽输出，初始低电平） */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 初始化所有 LED 为熄灭状态 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

    /* Note: GPIO configuration for motor control is handled by TIM peripheral */
}

/**
 * @brief 设置 LED 调试显示（4位二进制）
 * @param value 0-15 的值，用二进制方式显示在 LED 上
 */
void setDebugLED(uint8_t value)
{
    // Bit 0 -> PB5
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (value & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Bit 1 -> PB12
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (value & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Bit 2 -> PB13
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (value & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Bit 3 -> PB14
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (value & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
