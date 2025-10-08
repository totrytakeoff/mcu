/**
 * @file    main.cpp
 * @brief   STM32 Motor Control Project - Main Program
 * @author  Migrated from stm32_cmake project
 * @date    2024
 * 
 * Motor control application using 4 DC motors controlled via PWM
 * - Motor 1 & 3: Same direction (forward/backward)
 * - Motor 2 & 4: Opposite direction (for robot chassis)
 */

#include "stm32f1xx_hal.h"
#include "../include/common.h"
#include "../include/gpio.h"
#include "../include/tim.h"
#include "../include/motor.hpp"
#include "../include/drive_train.hpp"

/**
 * @brief  The application entry point
 * @retval int
 */
extern "C" int main(void)
{
    /* HAL Initialization */
    HAL_Init();
    
    /* System Clock Configuration */
    SystemClock_Config();
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM3_Init();
    
    /* Start PWM for all motor channels */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    
    /* Initialize Motor Objects */
    Motor motor1;
    Motor motor2;
    Motor motor3;
    Motor motor4;
    
    motor1.init(&htim3, TIM_CHANNEL_1);
    motor3.init(&htim3, TIM_CHANNEL_3);
    motor2.init(&htim3, TIM_CHANNEL_2);
    motor4.init(&htim3, TIM_CHANNEL_4);
    
    DriveTrain driveTrain(motor1, motor3, motor2, motor4);
    /* Note: Motor 1 & 3 rotate in same direction
     *       Motor 2 & 4 rotate in same direction (opposite to 1 & 3)
     */
    

    HAL_Delay(1000);

    /* Infinite loop */
    while (1)
    {
        // driveTrain.drive(50, 20);  // Move forward with slight left turn
        // HAL_Delay(2000);
        
        driveTrain.drive(0, 50);   // Rotate in place to the left
        HAL_Delay(2000);
        driveTrain.stop();          // Stop all motors
        HAL_Delay(1000);
        
        driveTrain.drive(0, 80);   // Rotate in place to the left
        HAL_Delay(2000);
        driveTrain.stop();
        HAL_Delay(1000);

        driveTrain.drive(0,100);
        HAL_Delay(2000);
        driveTrain.stop();
        HAL_Delay(1000);


        HAL_Delay(2000);
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
