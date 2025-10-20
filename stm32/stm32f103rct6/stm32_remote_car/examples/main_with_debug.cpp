/**
 * @file    main_with_debug.cpp
 * @brief   集成调试系统的主程序示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 本示例展示如何在实际项目中集成和使用调试系统
 */

#include "stm32f1xx_hal.h"
#include "adc.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include "debug.hpp"      // 调试系统
#include "motor.hpp"
#include "line_sensor.hpp"

/* ========== 配置选项 ========== */

// 编译时调试开关
#define ENABLE_DEBUG_OUTPUT     1       // 1=启用调试, 0=禁用调试
#define DEBUG_STARTUP_INFO      1       // 1=输出启动信息
#define DEBUG_LOOP_INFO         0       // 1=输出循环调试信息
#define DEBUG_SENSOR_INFO       1       // 1=输出传感器调试信息

/* ========== 全局对象 ========== */

Motor motor1, motor2, motor3, motor4;

/* ========== 函数声明 ========== */

extern "C" {
    void SystemClock_Config(void);
    void Error_Handler(void);
}

void Print_Startup_Banner(void);
void Debug_System_Status(void);

/* ========== 主程序 ========== */

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

    /* ========== 5. USART初始化 ========== */
    MX_USART1_UART_Init();  // E49无线模块
    MX_USART2_UART_Init();  // 调试串口
    
    /* ========== 6. 调试系统初始化 ========== */
    #if ENABLE_DEBUG_OUTPUT
        Debug_Enable();
    #else
        Debug_Disable();
    #endif
    
    /* ========== 7. 输出启动信息 ========== */
    #if DEBUG_STARTUP_INFO
        Print_Startup_Banner();
    #endif

    /* ========== 8. ADC初始化 ========== */
    Debug_Printf("[INIT] 初始化ADC...\r\n");
    MX_ADC1_Init();
    Debug_Printf("[INIT] ADC初始化完成\r\n");

    /* ========== 9. 启动PWM ========== */
    Debug_Printf("[INIT] 启动PWM...\r\n");
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    Debug_Printf("[INIT] PWM启动完成\r\n");

    /* ========== 10. 电机初始化 ========== */
    Debug_Printf("[INIT] 初始化电机...\r\n");
    motor1.init(&htim3, TIM_CHANNEL_1, GPIOC, GPIO_PIN_6, GPIO_PIN_7);
    motor2.init(&htim3, TIM_CHANNEL_2, GPIOC, GPIO_PIN_8, GPIO_PIN_9);
    motor3.init(&htim3, TIM_CHANNEL_3, GPIOC, GPIO_PIN_10, GPIO_PIN_11);
    motor4.init(&htim3, TIM_CHANNEL_4, GPIOC, GPIO_PIN_12, GPIO_PIN_13);
    Debug_Printf("[INIT] 电机初始化完成\r\n");

    /* ========== 11. 系统状态检查 ========== */
    Debug_System_Status();

    /* ========== 12. 完成初始化 ========== */
    Debug_Printf("\r\n");
    Debug_Printf("[READY] 系统初始化完成，进入主循环\r\n");
    Debug_Printf("[READY] 调试模式: %s\r\n", Debug_IsEnabled() ? "启用" : "禁用");
    Debug_Printf("\r\n");
    
    // 初始化完成后可以选择关闭调试以提高性能
    #if !DEBUG_LOOP_INFO
        Debug_Disable();
    #endif

    /* ========== 主循环 ========== */
    uint32_t loop_count = 0;
    uint32_t last_debug_time = 0;
    
    while (1)
    {
        loop_count++;
        
        /* 每隔1秒输出一次调试信息（避免刷屏） */
        #if DEBUG_LOOP_INFO
        if (HAL_GetTick() - last_debug_time >= 1000)
        {
            Debug_Enable();
            Debug_Printf("[LOOP] 循环计数: %lu, 运行时间: %lu ms\r\n", 
                         loop_count, HAL_GetTick());
            last_debug_time = HAL_GetTick();
        }
        #endif

        /* 读取传感器数据并调试输出 */
        #if DEBUG_SENSOR_INFO
        static uint32_t last_sensor_debug = 0;
        if (HAL_GetTick() - last_sensor_debug >= 2000)  // 每2秒输出一次
        {
            Debug_Enable();
            
            // 假设读取传感器值
            uint16_t sensor_values[8];
            // line_sensor_read(sensor_values);  // 实际读取函数
            
            Debug_Printf("[SENSOR] 传感器值: ");
            for (int i = 0; i < 8; i++)
            {
                printf("%d ", sensor_values[i]);
            }
            printf("\r\n");
            
            last_sensor_debug = HAL_GetTick();
            
            // 调试完后可以关闭
            Debug_Disable();
        }
        #endif

        /* 你的主要业务逻辑 */
        // ...

        HAL_Delay(10);
    }
}

/**
 * @brief 输出启动横幅信息
 */
void Print_Startup_Banner(void)
{
    // 使用强制输出，确保启动信息总是显示
    Debug_Print_Always("\r\n");
    Debug_Print_Always("========================================\r\n");
    Debug_Print_Always("  STM32F103 遥控小车系统 v1.0\r\n");
    Debug_Print_Always("========================================\r\n");
    Debug_Print_Always("固件版本  : v1.0.0\r\n");
    Debug_Print_Always("芯片型号  : STM32F103RCT6\r\n");
    Debug_Print_Always("系统时钟  : 72 MHz\r\n");
    Debug_Print_Always("编译时间  : %s %s\r\n", __DATE__, __TIME__);
    Debug_Print_Always("========================================\r\n");
    Debug_Print_Always("外设配置:\r\n");
    Debug_Print_Always("  - USART1: E49无线模块 @ 9600bps\r\n");
    Debug_Print_Always("  - USART2: 调试串口 @ 115200bps\r\n");
    Debug_Print_Always("  - TIM3  : PWM输出 (4路电机)\r\n");
    Debug_Print_Always("  - ADC1  : 8路灰度传感器\r\n");
    Debug_Print_Always("========================================\r\n");
    Debug_Print_Always("\r\n");
}

/**
 * @brief 输出系统状态信息
 */
void Debug_System_Status(void)
{
    Debug_Printf("\r\n");
    Debug_Printf("========== 系统状态 ==========\r\n");
    
    // 芯片信息
    Debug_Printf("芯片唯一ID: 0x%08lX\r\n", HAL_GetUIDw0());
    Debug_Printf("芯片版本  : 0x%04X\r\n", HAL_GetREVID());
    Debug_Printf("设备ID    : 0x%04X\r\n", HAL_GetDEVID());
    
    // 时钟信息
    Debug_Printf("SYSCLK    : %lu Hz\r\n", HAL_RCC_GetSysClockFreq());
    Debug_Printf("HCLK      : %lu Hz\r\n", HAL_RCC_GetHCLKFreq());
    Debug_Printf("PCLK1     : %lu Hz\r\n", HAL_RCC_GetPCLK1Freq());
    Debug_Printf("PCLK2     : %lu Hz\r\n", HAL_RCC_GetPCLK2Freq());
    
    // 运行信息
    Debug_Printf("运行时间  : %lu ms\r\n", HAL_GetTick());
    
    Debug_Printf("==============================\r\n");
    Debug_Printf("\r\n");
}

/**
 * @brief 系统时钟配置
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        // 使用强制输出报告错误
        Debug_Print_Always("[ERROR] 时钟配置失败！\r\n");
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Debug_Print_Always("[ERROR] 时钟配置失败！\r\n");
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Debug_Print_Always("[ERROR] 外设时钟配置失败！\r\n");
        Error_Handler();
    }
    
    Debug_Printf("[INIT] 系统时钟配置完成: 72MHz\r\n");
}

/**
 * @brief 错误处理函数
 */
void Error_Handler(void)
{
    // 错误信息使用强制输出
    Debug_Print_Always("\r\n");
    Debug_Print_Always("!!! 系统错误 !!!\r\n");
    Debug_Print_Always("错误位置: %s:%d\r\n", __FILE__, __LINE__);
    Debug_Print_Always("系统已停止运行\r\n");
    Debug_Print_Always("\r\n");
    
    __disable_irq();
    while (1)
    {
        // 无限循环，等待复位
    }
}

/**
 * @brief 断言失败处理
 */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    Debug_Print_Always("[ASSERT] 断言失败: %s:%lu\r\n", file, line);
    Error_Handler();
}
#endif
