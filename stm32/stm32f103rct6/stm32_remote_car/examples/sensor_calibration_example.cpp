/**
 * @file    sensor_calibration_example.cpp
 * @brief   传感器校准示例程序
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能：
 * 1. 自动校准传感器的黑白阈值
 * 2. 保存校准数据到EEPROM
 * 3. 从EEPROM加载校准数据
 * 
 * 使用方法：
 * 1. 上传此程序到STM32
 * 2. 打开串口监视器（115200波特率）
 * 3. 按照提示进行校准
 */

#include "adc.h"
#include "debug.hpp"
#include "eeprom.hpp"
#include "gpio.h"
#include "line_sensor.hpp"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "usart.h"

extern "C" {
void SystemClock_Config(void);
}

/* ========== 全局对象 ========== */

LineSensor sensor;
EEPROM eeprom;

/* ========== 辅助函数 ========== */

/**
 * @brief 打印菜单
 */
void printMenu() {
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("       传感器校准菜单\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("1. 自动校准（推荐）\r\n");
    Debug_Printf("2. 手动校准 - 白色\r\n");
    Debug_Printf("3. 手动校准 - 黑色\r\n");
    Debug_Printf("4. 保存校准数据到EEPROM\r\n");
    Debug_Printf("5. 从EEPROM加载校准数据\r\n");
    Debug_Printf("6. 查看当前传感器读数\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("请输入选项(1-6)：\r\n\r\n");
}

/**
 * @brief 显示当前传感器读数
 */
void showSensorData() {
    Debug_Printf("\r\n正在采集传感器数据...\r\n");
    
    for (int i = 0; i < 10; i++) {
        uint16_t data[8];
        sensor.getRawData(data);
        
        Debug_Printf("传感器 %d: ", i + 1);
        for (int j = 0; j < 8; j++) {
            Debug_Printf("%4d ", data[j]);
        }
        Debug_Printf("\r\n");
        
        HAL_Delay(200);
    }
    
    Debug_Printf("\r\n");
}

/* ========== 主程序 ========== */

extern "C" int main(void) {
    /* 1. HAL库初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 2. 外设初始化 */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_ADC1_Init();
    
    Debug_Enable();
    
    Debug_Printf("\r\n\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("   巡线传感器校准系统\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("版本: 1.0\r\n");
    Debug_Printf("传感器: 8路灰度传感器\r\n");
    Debug_Printf("存储: 24C02 EEPROM\r\n");
    Debug_Printf("========================================\r\n");
    
    HAL_Delay(1000);
    
    /* 3. 初始化EEPROM */
    Debug_Printf("\r\n[INIT] 正在初始化EEPROM...\r\n");
    if (!eeprom.init()) {
        Debug_Printf("[ERROR] EEPROM初始化失败！\r\n");
        Debug_Printf("[INFO] 可以继续使用，但无法保存校准数据\r\n");
    } else {
        Debug_Printf("[OK] EEPROM初始化成功\r\n");
        
        // 尝试加载校准数据
        Debug_Printf("\r\n[INIT] 尝试加载已保存的校准数据...\r\n");
        if (sensor.loadCalibration(eeprom)) {
            Debug_Printf("[OK] 校准数据加载成功\r\n");
        } else {
            Debug_Printf("[INFO] 未找到有效校准数据，使用默认值\r\n");
        }
    }
    
    /* 4. 自动校准演示（可选） */
    bool auto_calibrate_on_start = false;  // 改为true自动执行校准
    
    if (auto_calibrate_on_start) {
        Debug_Printf("\r\n========== 自动校准模式 ==========\r\n");
        Debug_Printf("准备开始自动校准...\r\n");
        Debug_Printf("3秒后开始...\r\n");
        HAL_Delay(3000);
        
        // 执行自动校准
        sensor.autoCalibrate();
        
        // 保存到EEPROM
        if (sensor.saveCalibration(eeprom)) {
            Debug_Printf("\r\n✅ 校准完成并已保存到EEPROM！\r\n");
        } else {
            Debug_Printf("\r\n⚠️ 校准完成但保存失败\r\n");
        }
    }
    
    /* 5. 主循环 - 菜单模式 */
    printMenu();
    
    while (1) {
        // 这里可以添加串口命令处理
        // 由于没有实现串口接收，这里只是演示框架
        
        // 演示：每隔5秒显示传感器数据
        static uint32_t last_time = 0;
        if (HAL_GetTick() - last_time > 5000) {
            showSensorData();
            printMenu();
            last_time = HAL_GetTick();
        }
        
        HAL_Delay(100);
    }
}

/**
 * @brief 系统时钟配置
 */
void SystemClock_Config(void) {
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

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while (1);
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        while (1);
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        while (1);
    }
}
