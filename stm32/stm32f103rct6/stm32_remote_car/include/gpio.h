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


// LED配置（在gpio.c中定义）
extern const uint16_t LED_PIN;
extern GPIO_TypeDef* LED_PORT;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Initialize GPIO ports
 * @note Enables clocks for GPIOA, GPIOB, GPIOC, GPIOD
 *       Motor control pins are configured through TIM peripheral
 */

void MX_GPIO_Init(void);

/**
 * @brief 设置 LED 调试显示（4位二进制）
 * @param value 0-15 的值，用二进制方式显示在 LED 上
 * 
 * LED 映射（从低位到高位）：
 * - Bit 0 (值 & 0x01): PB5
 * - Bit 1 (值 & 0x02): PB12
 * - Bit 2 (值 & 0x04): PB13
 * - Bit 3 (值 & 0x08): PB14
 * 
 * 示例：
 * - setDebugLED(0)  -> 0000 (全灭)
 * - setDebugLED(1)  -> 0001 (PB5 亮)
 * - setDebugLED(5)  -> 0101 (PB5 + PB13 亮)
 * - setDebugLED(15) -> 1111 (全亮)
 */
void setDebugLED(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */
