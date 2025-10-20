/**
 * @file    i2c_scan.cpp
 * @brief   I2C总线设备扫描程序
 * @author  AI Assistant
 * @date    2024
 * 
 * 用途：扫描I2C总线上的所有设备，帮助诊断硬件连接问题
 */

#include "stm32f1xx_hal.h"
#include "debug.hpp"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

extern "C" {
void SystemClock_Config(void);
}

/* I2C句柄 */
I2C_HandleTypeDef hi2c2;

/**
 * @brief I2C2初始化
 */
void MX_I2C2_Init(void)
{
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        Debug_Printf("[ERROR] I2C初始化失败\r\n");
    }
}

/**
 * @brief I2C2 MSP初始化
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(hi2c->Instance == I2C2) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C2_CLK_ENABLE();
        
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

/**
 * @brief 扫描I2C总线
 */
void I2C_Scan(void)
{
    Debug_Printf("\r\n========================================\r\n");
    Debug_Printf("   I2C总线设备扫描\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("正在扫描地址 0x00-0x7F...\r\n\r\n");
    
    uint8_t found_count = 0;
    
    for (uint8_t addr = 0; addr < 128; addr++) {
        // 尝试探测设备（7位地址）
        HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 3, 100);
        
        if (result == HAL_OK) {
            Debug_Printf("[FOUND] 设备地址: 0x%02X\r\n", addr);
            found_count++;
            
            // 识别常见设备
            if (addr == 0x50) {
                Debug_Printf("        --> 可能是 24C02/24C04 EEPROM\r\n");
            } else if (addr >= 0x50 && addr <= 0x57) {
                Debug_Printf("        --> 可能是 24Cxx EEPROM\r\n");
            } else if (addr == 0x68) {
                Debug_Printf("        --> 可能是 MPU6050/DS3231\r\n");
            } else if (addr == 0x76 || addr == 0x77) {
                Debug_Printf("        --> 可能是 BMP280/BME280\r\n");
            }
        }
        
        // 每16个地址打印一个进度点
        if ((addr + 1) % 16 == 0) {
            Debug_Printf(".");
        }
    }
    
    Debug_Printf("\r\n\r\n========================================\r\n");
    if (found_count > 0) {
        Debug_Printf("✅ 扫描完成：找到 %d 个I2C设备\r\n", found_count);
    } else {
        Debug_Printf("❌ 扫描完成：未找到任何I2C设备\r\n");
        Debug_Printf("\r\n可能的原因：\r\n");
        Debug_Printf("  1. 没有连接I2C设备\r\n");
        Debug_Printf("  2. PB10/PB11连接错误\r\n");
        Debug_Printf("  3. 缺少上拉电阻（需要4.7kΩ）\r\n");
        Debug_Printf("  4. I2C设备供电不正常\r\n");
    }
    Debug_Printf("========================================\r\n");
}

/**
 * @brief 主程序
 */
extern "C" int main(void)
{
    /* 1. HAL库初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 2. GPIO和串口初始化 */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    Debug_Enable();
    
    Debug_Printf("\r\n\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("   STM32 I2C设备扫描工具\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("I2C接口: I2C2\r\n");
    Debug_Printf("SCL引脚: PB10\r\n");
    Debug_Printf("SDA引脚: PB11\r\n");
    Debug_Printf("速率: 100kHz\r\n");
    
    HAL_Delay(1000);
    
    /* 3. 初始化I2C */
    Debug_Printf("\r\n[INIT] 正在初始化I2C2...\r\n");
    MX_I2C2_Init();
    Debug_Printf("[OK] I2C2初始化完成\r\n");
    
    HAL_Delay(500);
    
    /* 4. 扫描I2C总线 */
    I2C_Scan();
    
    /* 5. 完成 */
    Debug_Printf("\r\n提示：如果没有找到设备，请检查硬件连接\r\n");
    Debug_Printf("提示：24C02的地址通常是 0x50\r\n");
    
    while (1) {
        HAL_Delay(1000);
    }
}

/**
 * @brief 系统时钟配置
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1);
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | 
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        while(1);
    }
}
