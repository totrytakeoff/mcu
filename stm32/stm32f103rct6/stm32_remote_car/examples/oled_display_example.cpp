/**
 * @file    oled_display_example.cpp
 * @brief   OLED显示屏使用示例
 * @author  AI Assistant
 * @date    2024
 *
 * 编译说明：
 * 1. 在 platformio.ini 中添加：
 *    lib_deps = olikraus/U8g2 @ ^2.35.9
 *
 * 2. 在 build_src_filter 中添加：
 *    +<../examples/oled_display_example.cpp>
 *    -<main.cpp>
 */

#include "gpio.h"
#include "i2c.h"
#include <U8g2lib.h>  // 触发PlatformIO LDF查找u8g2库
#include "../include/oled_display.hpp"
#include "stm32f1xx_hal.h"
#include "usart.h"


/* ========== 外部函数声明 ========== */
extern "C" {
void SystemClock_Config(void);
void Error_Handler(void);
}

/* ========== 全局对象 ========== */
OLEDDisplay oled;

/* ========== 主程序 ========== */

int main(void) {
    // HAL初始化
    HAL_Init();
    SystemClock_Config();

    // 外设初始化
    MX_GPIO_Init();
    MX_I2C2_Init();
    MX_USART1_UART_Init();

    HAL_Delay(100);

    // 初始化OLED
    if (oled.init()) {
        // 示例1：显示欢迎界面
        oled.showWelcome();
        HAL_Delay(2000);

        // 示例2：显示文本
        oled.clear();
        oled.printLine(0, "Hello STM32!");
        oled.printLine(1, "OLED Test");
        oled.show();
        HAL_Delay(2000);

        // 示例3：显示格式化文本
        oled.clear();
        oled.printfLine(0, "Voltage: %.2fV", 3.3f);
        oled.printfLine(1, "Speed: %d%%", 75);
        oled.printfLine(2, "Temp: %d C", 25);
        oled.show();
        HAL_Delay(2000);

        // 示例4：显示PID参数
        oled.showPIDParams(1.5f, 0.5f, 0.2f);
        HAL_Delay(2000);

        // 示例5：显示调试信息
        oled.showDebugInfo("Running", 50, -12.5f, 2500);
        HAL_Delay(2000);

        // 示例6：绘制图形
        oled.clear();
        oled.drawRect(10, 10, 50, 30);
        oled.drawCircle(90, 25, 15);
        oled.drawLine(10, 50, 118, 50);
        oled.drawProgressBar(10, 55, 108, 8, 60);
        oled.show();
        HAL_Delay(2000);
    }

    // 主循环：动态更新
    uint32_t counter = 0;
    int speed = 0;
    float position = 0.0f;

    while (1) {
        // 模拟数据变化
        speed = 30 + (counter % 50);
        position = -20.0f + (counter % 40);

        // 更新显示
        oled.clear();
        oled.printfLine(0, "Counter: %lu", counter);
        oled.printfLine(1, "Speed: %d", speed);
        oled.printfLine(2, "Pos: %.1f", position);

        // 显示进度条
        uint8_t progress = (counter % 100);
        oled.drawProgressBar(10, 45, 108, 10, progress);
        oled.printfLine(4, "Progress: %d%%", progress);

        oled.show();

        counter++;
        HAL_Delay(100);  // 10Hz 刷新率
    }
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
}
