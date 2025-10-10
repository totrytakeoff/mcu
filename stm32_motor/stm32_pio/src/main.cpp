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
#include "../include/motor.hpp"
#include "../include/drive_train.hpp"
#include "../include/e49_wireless.hpp"
#include "../include/remote_control.hpp"

/* ========== 全局变量 ========== */
// E49 对象（全局，方便中断访问）
E49_Wireless* g_e49 = nullptr;

// UART 接收缓冲
uint8_t rxBuffer;

/**
 * @brief UART 接收完成回调函数（HAL库调用）
 * @param huart UART句柄
 */
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 将接收到的数据传递给 E49 对象
        if (g_e49 != nullptr)
        {
            g_e49->onDataReceived(rxBuffer);
        }
        
        // 重新启动接收（单字节循环接收）
        HAL_UART_Receive_IT(&huart1, &rxBuffer, 1);
    }
}

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
    
    /* ========== 7. 初始化 4 个电机 ========== */
    Motor motor1, motor2, motor3, motor4;
    
    motor1.init(&htim3, TIM_CHANNEL_1);
    motor2.init(&htim3, TIM_CHANNEL_2);
    motor3.init(&htim3, TIM_CHANNEL_3);
    motor4.init(&htim3, TIM_CHANNEL_4);
    
    /* ========== 8. 初始化差速转向系统 ========== */
    // Motor 1 & 3: 左侧
    // Motor 2 & 4: 右侧
    DriveTrain driveTrain(motor1, motor3, motor2, motor4);
    
    /* ========== 9. 初始化 E49 无线模块 ========== */
    E49_Wireless e49;
    e49.init();
    g_e49 = &e49;  // 保存到全局指针
    
    /* ========== 10. 初始化遥控器控制 ========== */
    RemoteControl remoteControl(driveTrain, e49);
    remoteControl.init();
    
    // 自定义参数（累积加速逻辑）
    remoteControl.setBaseSpeed(25);          // 基础速度 25%（最低速度，降低起步速度）
    remoteControl.setMaxSpeed(80);           // 最高速度 80%（降低最高限制，更安全）
    remoteControl.setSpeedIncrement(3);      // 速度增量 3%（降低加速灵敏度：10%→3%）
    remoteControl.setTurnSensitivity(35);    // 转向灵敏度 35%（降低转向激进程度）
    remoteControl.setTimeout(150);           // 超时 150ms（快速响应松开：1000ms→150ms）
    
    // 配置梯形速度轮廓参数（平滑加减速）
    driveTrain.setAcceleration(
        8,   // 加速度：提升启动响应（5→8）
        15,  // 减速度：提升刹车响应（8→15）
        20   // 反向减速度：快速反向切换（12→20）
    );
    
    /* ========== 11. 启动 UART 中断接收 ========== */
    HAL_UART_Receive_IT(&huart1, &rxBuffer, 1);
    
    /* ========== 12. 等待系统就绪 ========== */
    HAL_Delay(500);
    
    
    /* ==================== 主循环 ==================== */
    while (1)
    {
        // 更新梯形速度轮廓（平滑加减速）
        driveTrain.update();
        
        // 更新遥控器控制（检查超时）
        remoteControl.update();
        
        // 短暂延时
        HAL_Delay(10);
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
