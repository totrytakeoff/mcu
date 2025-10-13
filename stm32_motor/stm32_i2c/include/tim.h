/**
 * @file    tim.h
 * @brief   Timer configuration header for motor PWM control
 * @author  Migrated from stm32_cmake project
 * @date    2024
 * 
 * TIM3 configuration for 4-channel PWM output (motor control)
 * - Prescaler: 71 (72MHz / 72 = 1MHz timer clock)
 * - Period: 20000 (1MHz / 20000 = 50Hz PWM frequency, 20ms period)
 * - Channels: PC6 (CH1), PC7 (CH2), PC8 (CH3), PC9 (CH4)
 */

#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "common.h"

/* Exported variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim3;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize TIM3 for 4-channel PWM output
 * @note Configures TIM3 with 50Hz PWM frequency for motor control
 */
void MX_TIM3_Init(void);

/**
 * @brief Post-initialization GPIO configuration for TIM3
 * @param htim Pointer to TIM handle
 */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */
