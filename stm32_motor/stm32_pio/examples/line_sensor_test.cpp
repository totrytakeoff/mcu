/**
 * @file    line_sensor_test.cpp
 * @brief   灰度传感器测试程序（不控制电机）
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 用于测试和调试灰度传感器，通过串口输出传感器读数
 * 不控制电机，仅读取传感器值
 * 
 * @usage
 * 1. 编译并上传程序
 * 2. 打开串口监视器（115200波特率）
 * 3. 观察传感器读数
 * 4. 调整阈值参数
 */

#include "stm32f1xx_hal.h"
#include "line_sensor.hpp"
#include "gpio.h"
#include <stdio.h>

/* ========== 全局对象 ========== */
LineSensor lineSensor;

/* ========== 函数声明 ========== */
extern "C" {
    void SystemClock_Config(void);
}

void printSensorData(void);
void printBar(uint16_t value, uint16_t max);

/* ========== 主程序 ========== */
extern "C" int main(void)
{
    /* HAL库初始化 */
    HAL_Init();
    
    /* 系统时钟配置 */
    SystemClock_Config();
    
    /* GPIO初始化 */
    MX_GPIO_Init();
    
    /* 初始化灰度传感器 */
    lineSensor.init();
    
    printf("\n\n");
    printf("========================================\n");
    printf("   灰度传感器测试程序 v1.0\n");
    printf("========================================\n");
    printf("传感器数量: 8\n");
    printf("ADC分辨率: 12位 (0-4095)\n");
    printf("默认阈值: %d\n", lineSensor.getThreshold());
    printf("========================================\n\n");
    
    /* 等待稳定 */
    HAL_Delay(1000);
    
    /* 主循环 */
    uint32_t lastPrint = 0;
    
    while (1)
    {
        uint32_t now = HAL_GetTick();
        
        // 每 200ms 打印一次
        if (now - lastPrint >= 200) {
            lastPrint = now;
            
            // 更新传感器
            lineSensor.update();
            
            // 打印数据
            printSensorData();
        }
    }
}

/**
 * @brief 打印传感器数据
 */
void printSensorData(void)
{
    const uint16_t* values = lineSensor.getRawValues();
    uint8_t pattern = lineSensor.getBlackPattern();
    int16_t position = lineSensor.getPosition();
    uint16_t threshold = lineSensor.getThreshold();
    
    // 清屏（可选）
    // printf("\033[2J\033[H");
    
    printf("\n");
    printf("===== 传感器读数 =====\n");
    
    // 打印索引
    printf("索引: ");
    for (int i = 0; i < 8; i++) {
        printf("  [%d]  ", i);
    }
    printf("\n");
    
    // 打印位置标签
    printf("位置: ");
    const char* labels[] = {"最左", " 左2", " 左3", "中左", "中右", " 右3", " 右2", "最右"};
    for (int i = 0; i < 8; i++) {
        printf(" %s ", labels[i]);
    }
    printf("\n");
    
    // 打印 ADC 值
    printf("ADC:  ");
    for (int i = 0; i < 8; i++) {
        printf(" %4d ", values[i]);
    }
    printf("\n");
    
    // 打印黑白状态
    printf("状态: ");
    for (int i = 0; i < 8; i++) {
        if (lineSensor.isBlack(i)) {
            printf(" [黑] ");
        } else {
            printf(" [白] ");
        }
    }
    printf("\n");
    
    // 打印可视化条形图
    printf("\n可视化 (阈值=%d):\n", threshold);
    for (int i = 0; i < 8; i++) {
        printf("[%d] ", i);
        printBar(values[i], 4095);
        printf("\n");
    }
    
    // 打印位置信息
    printf("\n");
    printf("位置值: ");
    if (position == INT16_MIN) {
        printf("未检测到线条\n");
    } else {
        printf("%d ", position);
        if (position < -500) {
            printf("(偏左)\n");
        } else if (position > 500) {
            printf("(偏右)\n");
        } else {
            printf("(居中)\n");
        }
    }
    
    // 打印特殊状态
    printf("特殊状态: ");
    if (lineSensor.isCrossroad()) {
        printf("[十字路口] ");
    }
    if (lineSensor.isLost()) {
        printf("[丢线] ");
    }
    if (lineSensor.isOnLine()) {
        printf("[在线上] ");
    }
    printf("\n");
    
    printf("======================\n");
}

/**
 * @brief 打印条形图
 * @param value: 当前值
 * @param max: 最大值
 */
void printBar(uint16_t value, uint16_t max)
{
    int barLength = 30;
    int filled = (value * barLength) / max;
    
    printf("[");
    for (int i = 0; i < barLength; i++) {
        if (i < filled) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    printf("] %4d", value);
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
