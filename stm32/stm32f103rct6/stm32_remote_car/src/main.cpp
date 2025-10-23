/**
 * @file    main.cpp
 * @brief   简易双传感器巡线系统
 * @author  AI Assistant
 * @date    2024
 *
 * @description
 * 使用传感器0和传感器7实现简化巡线控制
 * - 梯度分级控制（7级状态）
 * - 直接电机驱动（不依赖 DriveTrain）
 * - 硬件容错性强
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
#include "simple_line_follower.hpp"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "usart.h"


/* ========== 全局对象 ========== */

// 电机对象（4个独立电机：前左、前右、后左、后右）
Motor motor_fl, motor_fr, motor_rl, motor_rr;

// 校准按钮（PD2，上拉模式，按下为低电平，200ms防抖）
Button calibButton(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 200);

// 系统状态
enum class SystemState {
    RUNNING,      // 正常巡线
    CALIBRATING,  // 传感器校准中
    STOPPED       // 停止状态
};

SystemState system_state = SystemState::STOPPED;

/* ========== 函数声明 ========== */
extern "C" {
void SystemClock_Config(void);
void Error_Handler(void);
}

void performCalibration(LineSensor& sensor, EEPROM& eeprom, SimpleLineFollower& follower);

/* ========== 主程序 ========== */
extern "C" int main(void) {
    /* ========== 1. HAL初始化 ========== */
    HAL_Init();
    SystemClock_Config();

    /* ========== 2. 外设初始化 ========== */
    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();
    MX_I2C2_Init();

    /* ========== 3. 启动PWM ========== */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    /* ========== 4. 调试串口初始化 ========== */
    Debug_Enable();
    Debug_Printf("\r\n========================================\r\n");
    Debug_Printf("简易双传感器巡线系统\r\n");
    Debug_Printf("========================================\r\n");

    /* ========== 5. 电机初始化 ========== */
    motor_fl.init(&htim3, TIM_CHANNEL_1);  // 前左
    motor_fr.init(&htim3, TIM_CHANNEL_2);  // 前右
    motor_rl.init(&htim3, TIM_CHANNEL_3);  // 后左
    motor_rr.init(&htim3, TIM_CHANNEL_4);  // 后右
    Debug_Printf("[OK] 电机初始化完成\r\n");

    /* ========== 6. EEPROM初始化 ========== */
    EEPROM eeprom;
    if (!eeprom.init()) {
        Debug_Printf("[WARN] EEPROM初始化失败\r\n");
    } else {
        Debug_Printf("[OK] EEPROM初始化完成\r\n");
    }

    /* ========== 7. 传感器初始化 ========== */
    LineSensor line_sensor;
    HAL_Delay(100);

    // 尝试从EEPROM加载校准数据
    Debug_Printf("[INIT] 加载校准数据...\r\n");
    if (line_sensor.loadCalibration(eeprom)) {
        Debug_Printf("[OK] 校准数据加载成功\r\n");
        system_state = SystemState::RUNNING;
    } else {
        Debug_Printf("[INFO] 未找到校准数据\r\n");
        Debug_Printf("[INFO] 请按下PD2按钮进行校准\r\n");
        system_state = SystemState::STOPPED;
    }

    /* ========== 8. 简易巡线控制器初始化 ========== */
    SimpleLineFollower simple_follower(line_sensor, motor_fl, motor_fr, motor_rl, motor_rr);

    // 配置参数（温和稳定模式）
    simple_follower.setLineMode(SimpleLineFollower::LineMode::WHITE_LINE_ON_BLACK);
    simple_follower.setBaseSpeed(20);                      // 基础速度: 0-100
    simple_follower.setSpeedGradient(1, 3, 7, 12);        // 微/轻/中/重（更温和，避免抖动）
    simple_follower.setThresholds(15.0f, 15.0f, 70.0f);   // 丢线/急转/在线阈值
    simple_follower.setSharpTurnDuration(700);             // 直角转弯总时长700ms（250ms停车+450ms转弯）
    simple_follower.enableDebug(true);                     // 启用调试输出

    simple_follower.init();
    Debug_Printf("[OK] 简易巡线控制器初始化完成\r\n");

    Debug_Printf("\r\n========== 系统就绪 ==========\r\n");
    Debug_Printf("传感器：仅使用传感器0和传感器7\r\n");
    Debug_Printf("控制：死区稳定控制（温和模式）\r\n");
    Debug_Printf("基础速度：%d\r\n", 20);
    Debug_Printf("速度梯度：微=%d 轻=%d 中=%d 重=%d\r\n", 1, 3, 7, 12);
    Debug_Printf("死区：±10%% 避免小波动触发转向\r\n");
    Debug_Printf("直角转弯：先停车250ms 再原地转450ms\r\n");
    Debug_Printf("丢线恢复：持续3秒智能搜索\r\n");
    Debug_Printf("提示：长按PD2按钮3秒重新校准\r\n\r\n");

    /* ========== 9. 主循环 ========== */
    calibButton.init();
    uint32_t last_update_time = HAL_GetTick();
    const uint32_t UPDATE_INTERVAL = 20;  // 20ms更新一次
    static bool long_press_handled = false;

    while (1) {
        // 更新按钮状态

        // 检测长按3秒进入校准
        if (calibButton.isLongPressed(3000) && !long_press_handled) {
            Debug_Printf("\r\n[系统] 长按检测，进入校准模式...\r\n");
            simple_follower.stop();
            performCalibration(line_sensor, eeprom, simple_follower);
            system_state = SystemState::RUNNING;
            long_press_handled = true;
        }

        // 释放按钮后重置长按标志
        if (calibButton.isReleased()) {
            long_press_handled = false;
        }

        // 根据系统状态执行
        switch (system_state) {
            case SystemState::RUNNING: {
                // 定时更新巡线控制（20ms一次）
                uint32_t current_time = HAL_GetTick();
                if (current_time - last_update_time >= UPDATE_INTERVAL) {
                    simple_follower.update();
                    last_update_time = current_time;
                }
                break;
            }

            case SystemState::STOPPED: {
                // 等待校准
                simple_follower.stop();

                // 检测按钮单击进入校准
                if (calibButton.isPressed()) {
                    Debug_Printf("\r\n[系统] 按钮按下，开始校准...\r\n");
                    performCalibration(line_sensor, eeprom, simple_follower);
                    system_state = SystemState::RUNNING;
                }
                HAL_Delay(100);
                break;
            }

            case SystemState::CALIBRATING:
                // 在 performCalibration 中处理
                break;
        }

        HAL_Delay(1);  // 防止CPU占用过高
    }
}

/**
 * @brief 执行传感器校准流程
 */
void performCalibration(LineSensor& sensor, EEPROM& eeprom, SimpleLineFollower& follower) {
    follower.stop();

    Debug_Printf("\r\n========================================\r\n");
    Debug_Printf("传感器校准\r\n");
    Debug_Printf("========================================\r\n");

    // LED亮起表示校准中
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);

    // 使用自动校准功能（三步式）
    sensor.autoCalibrate(calibButton);

    // 保存到EEPROM
    Debug_Printf("[校准] 保存数据到EEPROM...\r\n");
    if (sensor.saveCalibration(eeprom)) {
        Debug_Printf("[成功] ✅ 校准完成并已保存！\r\n");

        // LED闪烁3次表示成功
        for (int i = 0; i < 3; i++) {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
            HAL_Delay(200);
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            HAL_Delay(200);
        }
    } else {
        Debug_Printf("[失败] ❌ 保存失败！\r\n");
    }

    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);

    // 重新初始化巡线控制器（加载新校准数据）
    follower.init();

    Debug_Printf("[校准] 完成！开始巡线...\r\n");
    Debug_Printf("========================================\r\n\r\n");

    HAL_Delay(500);
}

/* ========== 系统配置函数 ========== */

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
    Debug_Printf("Assert failed: file %s on line %d\r\n", file, line);
}
#endif

#ifdef __cplusplus
}
#endif
