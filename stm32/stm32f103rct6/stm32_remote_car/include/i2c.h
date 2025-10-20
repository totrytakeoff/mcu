/**
 * @file    i2c.h
 * @brief   I2C总线配置（用于EEPROM通信）
 * @author  AI Assistant
 * @date    2024
 * 
 * 硬件连接：
 * - PB10 (I2C2_SCL) -> 24C02 SCL
 * - PB11 (I2C2_SDA) -> 24C02 SDA
 * 
 * 注意事项：
 * - STM32F103的I2C需要外接上拉电阻（通常4.7kΩ）
 * - 24C02器件地址前缀: 010 (binary) = 0x50 (7-bit address)
 */

#ifndef __I2C_H
#define __I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* I2C句柄 */
extern I2C_HandleTypeDef hi2c2;

/* 初始化函数 */
void MX_I2C2_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H */
