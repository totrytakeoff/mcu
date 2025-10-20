/**
 * @file    i2c.c
 * @brief   I2C总线配置实现（HAL库）
 * @author  AI Assistant
 * @date    2024
 */

#include "i2c.h"
#include "common.h"

/* I2C句柄 */
I2C_HandleTypeDef hi2c2;

/**
 * @brief I2C2初始化
 * @note  使用PB10(SCL)和PB11(SDA)
 *        时钟速度: 100kHz (标准模式)
 */
void MX_I2C2_Init(void)
{
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;                     // 100kHz 标准模式
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;             // 2:1 占空比
    hi2c2.Init.OwnAddress1 = 0;                         // 主机模式，地址无关
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;// 7位地址模式
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief I2C2 MSP初始化回调
 * @param hi2c I2C句柄
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(hi2c->Instance == I2C2)
    {
        /* 1. 使能GPIO和I2C时钟 */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C2_CLK_ENABLE();
        
        /* 2. 配置I2C引脚
         * PB10 (I2C2_SCL)
         * PB11 (I2C2_SDA)
         * 配置为开漏输出，复用功能
         */
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;          // 开漏输出
        GPIO_InitStruct.Pull = GPIO_NOPULL;              // 外部上拉，这里不使能内部上拉
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        /* 注意：确保硬件上有4.7kΩ上拉电阻连接到VCC */
    }
}

/**
 * @brief I2C2 MSP反初始化回调
 * @param hi2c I2C句柄
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    if(hi2c->Instance == I2C2)
    {
        /* 禁用I2C时钟 */
        __HAL_RCC_I2C2_CLK_DISABLE();
        
        /* 反初始化GPIO引脚 */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
    }
}
