/**
 * @file    eeprom_basic_example.cpp
 * @brief   EEPROM基本使用示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 本示例演示：
 * 1. EEPROM初始化
 * 2. 基本类型读写（int, float等）
 * 3. 设备检测
 */

#include "eeprom.hpp"
#include "debug.hpp"
#include "stm32f1xx_hal.h"

extern "C" {
void SystemClock_Config(void);
}

/* ========== 主程序 ========== */

extern "C" int main(void) {
    /* 1. HAL库初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 2. 初始化调试串口 */
    MX_USART2_UART_Init();
    Debug_Enable();
    
    Debug_Printf("\r\n========== EEPROM基本示例 ==========\r\n");
    
    /* 3. 创建并初始化EEPROM对象 */
    EEPROM eeprom;
    
    Debug_Printf("[INIT] 正在初始化EEPROM...\r\n");
    if (!eeprom.init()) {
        Debug_Printf("[ERROR] EEPROM初始化失败！\r\n");
        Debug_Printf("[ERROR] 请检查：\r\n");
        Debug_Printf("        1. I2C连接是否正确（PB10=SCL, PB11=SDA）\r\n");
        Debug_Printf("        2. 是否有4.7kΩ上拉电阻\r\n");
        Debug_Printf("        3. 24C02是否有供电\r\n");
        while (1);
    }
    Debug_Printf("[OK] EEPROM初始化成功\r\n\r\n");
    
    /* 4. 示例1：写入和读取整数 */
    Debug_Printf("===== 示例1: 整数读写 =====\r\n");
    
    int write_value = 12345;
    Debug_Printf("写入整数: %d (地址: 0x00)\r\n", write_value);
    
    if (eeprom.write(0x00, write_value)) {
        Debug_Printf("[OK] 写入成功\r\n");
    } else {
        Debug_Printf("[ERROR] 写入失败\r\n");
    }
    
    int read_value = 0;
    if (eeprom.read(0x00, read_value)) {
        Debug_Printf("[OK] 读取成功: %d\r\n", read_value);
        
        if (read_value == write_value) {
            Debug_Printf("[OK] 数据验证通过！\r\n");
        } else {
            Debug_Printf("[ERROR] 数据不匹配！\r\n");
        }
    } else {
        Debug_Printf("[ERROR] 读取失败\r\n");
    }
    
    HAL_Delay(1000);
    
    /* 5. 示例2：写入和读取浮点数 */
    Debug_Printf("\r\n===== 示例2: 浮点数读写 =====\r\n");
    
    float write_float = 3.14159f;
    Debug_Printf("写入浮点数: %.5f (地址: 0x10)\r\n", write_float);
    
    if (eeprom.write(0x10, write_float)) {
        Debug_Printf("[OK] 写入成功\r\n");
    } else {
        Debug_Printf("[ERROR] 写入失败\r\n");
    }
    
    float read_float = 0.0f;
    if (eeprom.read(0x10, read_float)) {
        Debug_Printf("[OK] 读取成功: %.5f\r\n", read_float);
        
        // 浮点数比较需要考虑精度
        if (read_float == write_float) {
            Debug_Printf("[OK] 数据验证通过！\r\n");
        } else {
            Debug_Printf("[ERROR] 数据不匹配！\r\n");
        }
    } else {
        Debug_Printf("[ERROR] 读取失败\r\n");
    }
    
    HAL_Delay(1000);
    
    /* 6. 示例3：写入和读取字节数组 */
    Debug_Printf("\r\n===== 示例3: 字节数组读写 =====\r\n");
    
    uint8_t write_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Debug_Printf("写入数组: ");
    for (int i = 0; i < 10; i++) {
        Debug_Printf("%d ", write_array[i]);
    }
    Debug_Printf("(地址: 0x20-0x29)\r\n");
    
    if (eeprom.writeBytes(0x20, write_array, 10)) {
        Debug_Printf("[OK] 写入成功\r\n");
    } else {
        Debug_Printf("[ERROR] 写入失败\r\n");
    }
    
    uint8_t read_array[10] = {0};
    if (eeprom.readBytes(0x20, read_array, 10)) {
        Debug_Printf("[OK] 读取成功: ");
        for (int i = 0; i < 10; i++) {
            Debug_Printf("%d ", read_array[i]);
        }
        Debug_Printf("\r\n");
        
        // 验证数据
        bool match = true;
        for (int i = 0; i < 10; i++) {
            if (read_array[i] != write_array[i]) {
                match = false;
                break;
            }
        }
        
        if (match) {
            Debug_Printf("[OK] 数据验证通过！\r\n");
        } else {
            Debug_Printf("[ERROR] 数据不匹配！\r\n");
        }
    } else {
        Debug_Printf("[ERROR] 读取失败\r\n");
    }
    
    /* 7. 完成 */
    Debug_Printf("\r\n========== 示例运行完成 ==========\r\n");
    Debug_Printf("提示：断电后重新上电，数据应该仍然保存在EEPROM中\r\n");
    
    while (1) {
        HAL_Delay(1000);
    }
}
