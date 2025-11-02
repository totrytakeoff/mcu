/**
 * @file    i2c_scanner.cpp
 * @brief   I2C设备扫描工具
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能：
 * - 扫描I2C总线上的所有设备
 * - 显示设备地址（7位和8位格式）
 * - 识别常见设备（OLED、EEPROM等）
 * 
 * 编译说明：
 * 在 platformio.ini 中添加：
 *    build_src_filter = 
 *        +<../examples/i2c_scanner.cpp>
 *        -<main.cpp>
 */

#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"
#include "debug.hpp"

/* ========== 外部函数声明 ========== */
extern "C" void SystemClock_Config(void);

/* ========== 设备名称识别 ========== */
const char* getDeviceName(uint8_t addr) {
    switch(addr) {
        case 0x3C: return "OLED (SSD1306/SSD1315)";
        case 0x3D: return "OLED (SSD1306/SSD1315, Alt)";
        case 0x50: return "EEPROM (24C02)";
        case 0x51: return "EEPROM (24C02, Alt1)";
        case 0x52: return "EEPROM (24C02, Alt2)";
        case 0x53: return "EEPROM (24C02, Alt3)";
        case 0x68: return "RTC (DS1307/DS3231)";
        case 0x76: return "BME280/BMP280";
        case 0x77: return "BME280/BMP280 (Alt)";
        default:   return "Unknown Device";
    }
}

/* ========== 主程序 ========== */

int main(void) {
    // HAL初始化
    HAL_Init();
    SystemClock_Config();
    
    // 外设初始化
    MX_GPIO_Init();
    MX_I2C2_Init();
    MX_USART1_UART_Init();
    
    HAL_Delay(500);
    
    Debug_Printf("\r\n");
    Debug_Printf("╔════════════════════════════════════════════════╗\r\n");
    Debug_Printf("║        I2C Device Scanner (I2C2)               ║\r\n");
    Debug_Printf("║        PB10 (SCL), PB11 (SDA)                  ║\r\n");
    Debug_Printf("╚════════════════════════════════════════════════╝\r\n");
    Debug_Printf("\r\n");
    
    while (1) {
        Debug_Printf("开始扫描I2C总线...\r\n");
        Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
        
        uint8_t found_count = 0;
        
        // 扫描所有可能的I2C地址（7位地址：0x01 ~ 0x7F）
        for (uint8_t addr = 0x01; addr <= 0x7F; addr++) {
            // 尝试与设备通信
            HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(
                &hi2c2, 
                addr << 1,  // 转换为8位地址
                1,          // 重试次数
                10          // 超时时间(ms)
            );
            
            if (result == HAL_OK) {
                // 找到设备
                Debug_Printf("✓ 找到设备：0x%02X (8位: 0x%02X) - %s\r\n", 
                           addr, 
                           addr << 1, 
                           getDeviceName(addr));
                found_count++;
            }
            
            // 简单延时，避免过快扫描
            HAL_Delay(2);
        }
        
        Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
        
        if (found_count == 0) {
            Debug_Printf("⚠ 未找到任何I2C设备！\r\n");
            Debug_Printf("\r\n");
            Debug_Printf("请检查：\r\n");
            Debug_Printf("  1. 设备是否正确连接到 PB10(SCL) 和 PB11(SDA)\r\n");
            Debug_Printf("  2. 设备是否已供电（VCC 和 GND）\r\n");
            Debug_Printf("  3. I2C上拉电阻是否存在（通常4.7kΩ）\r\n");
            Debug_Printf("  4. 接线是否松动或接触不良\r\n");
        } else {
            Debug_Printf("✓ 总共找到 %d 个I2C设备\r\n", found_count);
        }
        
        Debug_Printf("\r\n");
        Debug_Printf("5秒后重新扫描...\r\n");
        Debug_Printf("\r\n");
        
        HAL_Delay(5000);
    }
}

/**
 * @brief I2C错误回调
 */
extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == I2C2) {
        uint32_t error = HAL_I2C_GetError(hi2c);
        Debug_Printf("I2C Error: 0x%08lX\r\n", error);
        
        if (error & HAL_I2C_ERROR_BERR) {
            Debug_Printf("  - Bus Error\r\n");
        }
        if (error & HAL_I2C_ERROR_ARLO) {
            Debug_Printf("  - Arbitration Lost\r\n");
        }
        if (error & HAL_I2C_ERROR_AF) {
            Debug_Printf("  - Acknowledge Failure\r\n");
        }
        if (error & HAL_I2C_ERROR_OVR) {
            Debug_Printf("  - Overrun/Underrun\r\n");
        }
        if (error & HAL_I2C_ERROR_TIMEOUT) {
            Debug_Printf("  - Timeout\r\n");
        }
    }
}
