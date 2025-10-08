/**
 * @file    gpio.h
 * @brief   GPIO configuration header
 * @author  Migrated from stm32_cmake project
 * @date    2024
 * 
 * GPIO initialization for STM32F103RC
 * Motor control GPIO is handled by TIM peripheral
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize GPIO ports
 * @note Enables clocks for GPIOA, GPIOB, GPIOC, GPIOD
 *       Motor control pins are configured through TIM peripheral
 */
void MX_GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */
