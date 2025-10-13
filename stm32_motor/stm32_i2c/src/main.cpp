/**
 * @file    main.cpp
 * @brief   遥控小车主程序
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能说明：
 * 通过 TLE100 遥控器控制 4 轮差速小车
 * - F: 前进
 * - B: 后退
 * - L: 左转
 * - R: 右转
 * - U: 前进+右转
 * - D: 后退+右转
 * - W: 前进+左转
 * - X: 后退+左转
 * - 超时自动停止（500ms）
 */

#include "stm32f1xx_hal.h"
#include "../include/common.h"
#include "../include/gpio.h"
#include "../include/tim.h"
#include "../include/usart.h"

/**
 * @brief  The application entry point
 * @retval int
 */
extern "C" int main(void)
{
    /* ========== 1. HAL库初始化 ========== */
    HAL_Init();
    
    /* ========== 2. 系统时钟配置 ========== */
    SystemClock_Config();
    
    /* ========== 3. GPIO初始化 ========== */
    MX_GPIO_Init();
    
    /* ========== 4. 定时器初始化（PWM） ========== */
    MX_TIM3_Init();
    
    /* ========== 5. USART1初始化（E49通信） ========== */
    MX_USART1_UART_Init();
    
    /* ========== 6. 启动所有 PWM 通道 ========== */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    
 
    HAL_Delay(500);
    
    
    /* ==================== 主循环 ==================== */
    while (1)
    {
       
    }
}

/**
 * @brief  System Clock Configuration
 * @note   Configures system clock to 72MHz using external 8MHz crystal
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
    /**
     * Initializes the RCC Oscillators:
     * - External HSE: 8MHz
     * - PLL: 8MHz * 9 = 72MHz
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    /**
     * Initializes the CPU, AHB and APB buses clocks:
     * - SYSCLK: 72MHz
     * - AHB: 72MHz
     * - APB1: 36MHz (max for STM32F103)
     * - APB2: 72MHz
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
    
    /* Peripheral Clock Configuration */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief  Error Handler
 * @note   Called when a HAL function returns an error
 * @retval None
 */
void Error_Handler(void)
{
    /* Disable interrupts */
    __disable_irq();
    
    /* Infinite loop on error */
    while (1) {
        // Could add LED blinking or other error indication here
    }
}

/**
 * @brief  HAL MSP Initialization
 * @retval None
 */
void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /* System interrupt init */
    
    /** NOJTAG: JTAG-DP Disabled and SW-DP Enabled */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
