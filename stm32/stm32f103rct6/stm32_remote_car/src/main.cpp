/**
 * @file    main.cpp
 * @brief   简洁巡线系统（带按钮校准功能）
 * @author  AI Assistant
 * @date    2024
 */

#include <cstdint>
#include "adc.h"
#include "button.hpp"
#include "debug.hpp"
#include "eeprom.hpp"
#include "gpio.h"
#include "i2c.h"
#include "line_sensor.hpp"
#include "motor.hpp"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "tim.h"
#include "usart.h"

/* ========== 全局对象 ========== */



enum class RUN_MODE { NORMAL, CALIBRATING, OTHER, TEST };

RUN_MODE g_run_mode = RUN_MODE::NORMAL;


Motor motor1, motor2, motor3, motor4;

uint16_t g_sensor_data[8];

const int16_t sensor_offset[8] = {0, 120, 0, 0, 0, 0, 0, 0};

// 校准按钮（PD2，上拉模式，按下为低电平，200ms防抖）
Button calibButton(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 200);

/* ========== 函数声明 ========== */
extern "C" {
void SystemClock_Config(void);
void Error_Handler(void);
}



/* ========== 主程序 ========== */
extern "C" int main(void) {
    /* ========== 1. HAL库初始化 ========== */
    HAL_Init();

    /* ========== 2. 系统时钟配置 ========== */
    SystemClock_Config();

    /* ========== 3. GPIO初始化 ========== */
    MX_GPIO_Init();

    /* ========== 4. 定时器初始化（PWM） ========== */
    MX_TIM3_Init();

    /* ========== 5. USART初始化 ========== */
    MX_USART1_UART_Init();
    Debug_Enable();  // 启用调试输出

    /* ========== 6. ADC初始化（8路灰度传感器） ========== */
    MX_ADC1_Init();

    /* ========== 6b. I2C初始化（EEPROM） ========== */
    MX_I2C2_Init();

    /* ========== 7. 启动所有 PWM 通道 ========== */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    /* ========== 8. 初始化 4 个电机 ========== */
    motor1.init(&htim3, TIM_CHANNEL_1);
    motor2.init(&htim3, TIM_CHANNEL_2);
    motor3.init(&htim3, TIM_CHANNEL_3);
    motor4.init(&htim3, TIM_CHANNEL_4);

    /* ========== 9. 初始化校准按钮 ========== */
    calibButton.init();

    /* ========== 10. 初始化EEPROM和传感器 ========== */
    Debug_Printf("\r\n========== 巡线系统启动 ==========\r\n");

    EEPROM eeprom;
    Debug_Printf("[INIT] 正在初始化EEPROM...\r\n");
    if (!eeprom.init()) {
        Debug_Printf("[WARN] EEPROM初始化失败，无法保存校准数据\r\n");
    } else {
        Debug_Printf("[OK] EEPROM初始化成功\r\n");
    }

    LineSensor line_sensor;

    line_sensor.setSensorOffsets(sensor_offset);

    // 加载校准数据
    Debug_Printf("[INIT] 正在加载传感器校准数据...\r\n");
    if (!line_sensor.loadCalibration(eeprom)) {
        Debug_Printf("[INFO] 未找到有效校准数据\r\n");
        Debug_Printf("[INFO] 请按PD2按钮进行校准\r\n");
    }

    Debug_Printf("\r\n========== 系统就绪 ==========\r\n");
    Debug_Printf("提示：长按PD2按钮3秒可随时重新校准传感器\r\n\r\n");

    /* ========== 11. 主循环 ========== */
    static bool long_press_handled = false;  // 防止长按重复触发

    // 注意set是关 reset是亮
    // HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);

    while (1) {
        switch (g_run_mode) {
            case RUN_MODE::NORMAL: {
                // 检测长按3秒进入校准模式
                if (calibButton.isLongPressed(3000) && !long_press_handled) {
                    Debug_Printf("\r\n🔧 长按检测到，进入校准模式...\r\n");
                    g_run_mode = RUN_MODE::CALIBRATING;
                    long_press_handled = true;
                    break;  // 立即进入校准模式
                }

                // 释放按钮后重置长按标志
                if (calibButton.isReleased()) {
                    long_press_handled = false;
                }

                // ========== 正常巡线逻辑 ==========
                line_sensor.getData(g_sensor_data);

                // 可选：打印传感器数据（调试用）
                // Debug_Printf("SENSOR_DATA: %d %d %d %d %d %d %d %d \r\n", g_sensor_data[0],
                //              g_sensor_data[1], g_sensor_data[2], g_sensor_data[3], g_sensor_data[4],
                //              g_sensor_data[5], g_sensor_data[6], g_sensor_data[7]);

                // TODO: 在这里添加你的巡线控制逻辑
                // 例如：计算位置偏差、PID控制、电机驱动等

                break;
            }

            case RUN_MODE::CALIBRATING: {
                // 进入校准模式亮 D10
                HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
                calibButton.reset();
                // 执行手动分步校准（内部等待按钮，分三步完成）
                line_sensor.autoCalibrate(calibButton);

                // 保存到EEPROM
                Debug_Printf("\r\n[SAVE] 正在保存校准数据到EEPROM...\r\n");
                if (line_sensor.saveCalibration(eeprom)) {
                    Debug_Printf("[SUCCESS] ✅ 校准完成并已保存！\r\n");

                    // LED闪烁3次表示成功
                    for (int i = 0; i < 3; i++) {
                        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
                        HAL_Delay(200);
                        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
                        HAL_Delay(200);
                    }
                } else {
                    Debug_Printf("[ERROR] ❌ 校准数据保存失败！\r\n");
                }

                Debug_Printf("\r\n");
                Debug_Printf("╔════════════════════════════════════════╗\r\n");
                Debug_Printf("║       退出校准，恢复巡线模式           ║\r\n");
                Debug_Printf("╚════════════════════════════════════════╝\r\n");
                Debug_Printf("\r\n");

                
                HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
                HAL_Delay(1000);

                // 退出校准模式
                g_run_mode = RUN_MODE::NORMAL;
                break;
            }

            case RUN_MODE::TEST: {

                if (calibButton.isPressed()) {
                    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
                }

                break;
            }

            default:
                break;
        }

        HAL_Delay(10);  // 小延迟，避免CPU占用过高
    }
}

/* ========== 系统配置 ========== */
void SystemClock_Config(void) {
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
    RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
void Error_Handler(void) {
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
void HAL_MspInit(void) {
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init */

    /** NOJTAG: JTAG-DP Disabled and SW-DP Enabled */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}
