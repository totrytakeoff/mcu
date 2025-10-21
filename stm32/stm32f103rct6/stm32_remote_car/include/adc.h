/**
 * @file    adc.h
 * @brief   ADC 配置 - 8路灰度传感器采样
 * @author  AI Assistant
 * @date    2024
 * 
 * @hardware 灰度传感器连接 (从左到右排列):
 *   索引   引脚     ADC通道      信号名
 *   [0]    PB0   - ADC_CH8  - SIG1 (最左侧)
 *   [1]    PB1   - ADC_CH9  - SIG2 (左2)
 *   [2]    PC0   - ADC_CH10 - SIG3 (左3)
 *   [3]    PC1   - ADC_CH11 - SIG4 (中左)
 *   [4]    PC2   - ADC_CH12 - SIG5 (中右)
 *   [5]    PC3   - ADC_CH13 - SIG6 (右3)
 *   [6]    PC4   - ADC_CH14 - SIG7 (右2)
 *   [7]    PC5   - ADC_CH15 - SIG8 (最右侧)
 */

#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* ADC 句柄 */
extern ADC_HandleTypeDef hadc1;

/**
 * @brief 错误处理函数（由main.cpp提供）
 */
void Error_Handler(void);

/* 初始化函数 */
void MX_ADC1_Init(void);

/* 读取单个通道的 ADC 值 (0-4095) */
uint16_t ADC_ReadChannel(uint32_t channel);

/* 读取所有 8 路传感器值 (DMA 方式) */
void ADC_ReadAll(uint16_t *buffer);

/* 开始 DMA 连续转换 */
void ADC_StartDMA(uint16_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */
