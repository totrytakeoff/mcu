/**
 * @file    line_following_white_on_black_demo.cpp
 * @brief   黑底白线巡线示例程序
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 演示如何使用 LineSensor 和 LineFollower 类实现黑底白线巡线功能
 * 
 * @hardware
 * - 8路灰度传感器 → PB0, PB1, PC0-PC5 (ADC)
 * - 4个电机 → TIM3_CH1-4 (PWM)
 * - LED → PA8 (状态指示)
 * 
 * @note
 * 与白底黑线的唯一区别：设置为 WHITE_ON_BLACK 模式
 */

#include "stm32f1xx_hal.h"
#include "line_sensor.hpp"
#include "line_follower.hpp"
#include "drive_train.hpp"
#include "motor.hpp"
#include "gpio.h"
#include "tim.h"

/* ========== 全局对象 ========== */
LineSensor lineSensor;
Motor motor1, motor2, motor3, motor4;
DriveTrain driveTrain(motor1, motor3, motor2, motor4);
LineFollower lineFollower(lineSensor, driveTrain);

/* ========== 函数声明 ========== */
extern "C" {
    void SystemClock_Config(void);
}

void LED_Toggle(void);
bool CrossroadHandler(void);

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
    
    /* ========== 5. 启动所有 PWM 通道 ========== */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    
    /* ========== 6. 初始化 4 个电机 ========== */
    motor1.init(&htim3, TIM_CHANNEL_1);
    motor2.init(&htim3, TIM_CHANNEL_2);
    motor3.init(&htim3, TIM_CHANNEL_3);
    motor4.init(&htim3, TIM_CHANNEL_4);
    
    /* ========== 7. 初始化灰度传感器 ========== */
    lineSensor.init();
    
    /* ========== 8. 设置为黑底白线模式 ⭐重要⭐ ========== */
    lineSensor.setLineMode(LineSensor::LineMode::WHITE_ON_BLACK);
    
    /* ========== 9. 传感器校准 ========== */
    HAL_Delay(1000);  // 等待稳定
    
    // LED 闪烁提示：放在黑色地面上
    for (int i = 0; i < 3; i++) {
        LED_Toggle();
        HAL_Delay(300);
    }
    lineSensor.calibrateBlack();  // 校准黑色地面
    HAL_Delay(1000);
    
    // LED 闪烁提示：放在白色线条上
    for (int i = 0; i < 3; i++) {
        LED_Toggle();
        HAL_Delay(300);
    }
    lineSensor.calibrateWhite();  // 校准白色线条
    lineSensor.finishCalibration();
    
    HAL_Delay(1000);  // 准备开始
    
    /* ========== 10. 初始化巡线控制器 ========== */
    lineFollower.init();
    
    // 设置 PID 参数
    lineFollower.setPID(
        0.08f,   // Kp: 比例系数
        0.0f,    // Ki: 积分系数
        1.2f     // Kd: 微分系数
    );
    
    // 设置基础速度
    lineFollower.setSpeed(40);  // 40% 速度
    
    // 设置丢线处理
    lineFollower.setLostLineHandling(true);
    
    // 设置十字路口回调
    lineFollower.setCrossroadCallback(CrossroadHandler);
    
    /* ========== 11. 启动巡线 ========== */
    lineFollower.start();
    
    /* ========== 12. 主循环 ========== */
    uint32_t lastUpdate = HAL_GetTick();
    uint32_t lastDebug = HAL_GetTick();
    
    while (1)
    {
        uint32_t now = HAL_GetTick();
        
        // 50Hz 控制频率（每 20ms 更新一次）
        if (now - lastUpdate >= 20) {
            lastUpdate = now;
            
            // 更新巡线控制
            lineFollower.update();
            
            // LED 指示：运行中
            if (lineFollower.isRunning()) {
                if ((now / 100) % 2) {
                    LED_Toggle();
                }
            }
        }
        
        // 调试信息输出（每 500ms）
        if (now - lastDebug >= 500) {
            lastDebug = now;
            
            // 可以通过 USART2 输出调试信息
            // printf("Mode: WHITE_ON_BLACK, Error: %d\n", 
            //        lineFollower.getError());
        }
    }
}

/* ========== 辅助函数 ========== */

/**
 * @brief LED 翻转（状态指示）
 */
void LED_Toggle(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
}

/**
 * @brief 十字路口处理回调
 * @return true=继续巡线, false=停车
 */
bool CrossroadHandler(void)
{
    // 直行穿过十字路口
    driveTrain.drive(40, 40);
    HAL_Delay(200);
    
    // 继续巡线
    return true;
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
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}
