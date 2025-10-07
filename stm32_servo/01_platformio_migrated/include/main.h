/*============================================================================
 *                        Main Header File - PlatformIO版本
 *============================================================================*/

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含文件 */
#include "stm32f1xx_hal.h"

/* 导出的类型 */
/* 导出的常量 */
/* 导出的宏 */
/* 导出的函数 */
void Error_Handler(void);
void Delay_Ms(uint32_t ms);
void Delay_Us(uint32_t us);

/* HAL MSP函数 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim);

/* 私有定义 */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */