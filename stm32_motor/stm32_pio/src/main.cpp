/**
 * @file    main.cpp
 * @brief   基础STM32项目主程序
 * @author  AI Assistant
 * @date    2024
 */

#include "stm32f1xx_hal.h"

// 函数声明
void SystemClock_Config(void);
void Error_Handler(void);

/**
 * @brief  主函数
 */
int main(void)
{
    // HAL库初始化
    HAL_Init();
    
    // 系统时钟配置
    SystemClock_Config();
    
    // 简单的主循环
    while (1)
    {
        // 基础的空循环
        __asm("nop");
    }
}

/**
 * @brief  系统时钟配置
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // 配置主振荡器
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    // 配置系统时钟
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief  错误处理函数
 */
void Error_Handler(void)
{
    // 禁用中断
    __disable_irq();
    
    // 错误处理：进入无限循环
    while (1) {
        // 空循环
    }
}

/**
 * @brief  HAL MSP初始化
 */
void HAL_MspInit(void)
{
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
}
