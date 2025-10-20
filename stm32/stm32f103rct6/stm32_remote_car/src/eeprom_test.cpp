/**
 * @file    eeprom_test.cpp
 * @brief   EEPROM功能测试程序
 * @author  AI Assistant
 * @date    2024
 * 
 * 本程序用于测试EEPROM的所有功能：
 * 1. 设备检测
 * 2. 单字节读写
 * 3. 多字节读写
 * 4. 基本类型读写
 * 5. 结构体读写
 * 6. CRC校验
 * 7. 页边界处理
 * 8. 地址越界保护
 */

#include "eeprom.hpp"
#include "debug.hpp"
#include "stm32f1xx_hal.h"
#include <string.h>


#include "gpio.h"

#include "tim.h"
#include "usart.h"


/* ========== 测试计数器 ========== */

struct TestStats {
    int total_tests;
    int passed_tests;
    int failed_tests;
};

TestStats stats = {0, 0, 0};

/* ========== 测试辅助函数 ========== */

void test_begin(const char* test_name) {
    Debug_Printf("\r\n[TEST] %s\r\n", test_name);
    stats.total_tests++;
}

void test_pass(const char* message) {
    Debug_Printf("[PASS] %s\r\n", message);
    stats.passed_tests++;
}

void test_fail(const char* message) {
    Debug_Printf("[FAIL] %s\r\n", message);
    stats.failed_tests++;
}

void print_test_summary() {
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("           测试结果汇总\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("总测试数: %d\r\n", stats.total_tests);
    Debug_Printf("通过: %d\r\n", stats.passed_tests);
    Debug_Printf("失败: %d\r\n", stats.failed_tests);
    
    if (stats.failed_tests == 0) {
        Debug_Printf("\r\n✅ 所有测试通过！\r\n");
    } else {
        Debug_Printf("\r\n❌ 有 %d 个测试失败\r\n", stats.failed_tests);
    }
    Debug_Printf("========================================\r\n");
}

/* ========== 测试函数 ========== */

/**
 * @brief 测试1：设备检测
 */
void test_device_detection(EEPROM& eeprom) {
    test_begin("设备检测");
    
    if (eeprom.isDeviceReady()) {
        test_pass("EEPROM设备在线");
    } else {
        test_fail("EEPROM设备离线");
    }
}

/**
 * @brief 测试2：单字节读写
 */
void test_single_byte(EEPROM& eeprom) {
    test_begin("单字节读写");
    
    uint8_t write_value = 0xAB;
    uint8_t read_value = 0;
    
    if (!eeprom.writeByte(0x00, write_value)) {
        test_fail("写入失败");
        return;
    }
    
    if (!eeprom.readByte(0x00, read_value)) {
        test_fail("读取失败");
        return;
    }
    
    if (read_value == write_value) {
        test_pass("单字节读写成功");
    } else {
        test_fail("数据不匹配");
    }
}

/**
 * @brief 测试3：多字节读写
 */
void test_multi_bytes(EEPROM& eeprom) {
    test_begin("多字节读写");
    
    uint8_t write_data[16];
    uint8_t read_data[16];
    
    // 初始化写入数据
    for (int i = 0; i < 16; i++) {
        write_data[i] = i * 10;
    }
    
    if (!eeprom.writeBytes(0x10, write_data, 16)) {
        test_fail("写入失败");
        return;
    }
    
    if (!eeprom.readBytes(0x10, read_data, 16)) {
        test_fail("读取失败");
        return;
    }
    
    bool match = true;
    for (int i = 0; i < 16; i++) {
        if (read_data[i] != write_data[i]) {
            match = false;
            break;
        }
    }
    
    if (match) {
        test_pass("多字节读写成功");
    } else {
        test_fail("数据不匹配");
    }
}

/**
 * @brief 测试4：整数读写
 */
void test_integer(EEPROM& eeprom) {
    test_begin("整数读写");
    
    int write_value = 12345;
    int read_value = 0;
    
    if (!eeprom.write(0x20, write_value)) {
        test_fail("写入失败");
        return;
    }
    
    if (!eeprom.read(0x20, read_value)) {
        test_fail("读取失败");
        return;
    }
    
    if (read_value == write_value) {
        test_pass("整数读写成功");
    } else {
        test_fail("数据不匹配");
    }
}

/**
 * @brief 测试5：浮点数读写
 */
void test_float(EEPROM& eeprom) {
    test_begin("浮点数读写");
    
    float write_value = 3.14159f;
    float read_value = 0.0f;
    
    if (!eeprom.write(0x30, write_value)) {
        test_fail("写入失败");
        return;
    }
    
    if (!eeprom.read(0x30, read_value)) {
        test_fail("读取失败");
        return;
    }
    
    if (read_value == write_value) {
        test_pass("浮点数读写成功");
    } else {
        test_fail("数据不匹配");
    }
}

/**
 * @brief 测试6：结构体读写（不带CRC）
 */
void test_struct(EEPROM& eeprom) {
    test_begin("结构体读写");
    
    struct TestStruct {
        int a;
        float b;
        uint8_t c;
    };
    
    TestStruct write_data = {100, 2.5f, 0xAB};
    TestStruct read_data = {0, 0.0f, 0};
    
    if (!eeprom.writeStruct(0x40, write_data)) {
        test_fail("写入失败");
        return;
    }
    
    if (!eeprom.readStruct(0x40, read_data)) {
        test_fail("读取失败");
        return;
    }
    
    if (read_data.a == write_data.a && 
        read_data.b == write_data.b && 
        read_data.c == write_data.c) {
        test_pass("结构体读写成功");
    } else {
        test_fail("数据不匹配");
    }
}

/**
 * @brief 测试7：结构体读写（带CRC）
 */
void test_struct_crc(EEPROM& eeprom) {
    test_begin("结构体读写（CRC校验）");
    
    struct TestStruct {
        float kp, ki, kd;
    };
    
    TestStruct write_data = {1.5f, 0.5f, 0.2f};
    TestStruct read_data = {0, 0, 0};
    
    if (!eeprom.writeStructCRC(0x50, write_data)) {
        test_fail("写入失败");
        return;
    }
    
    if (!eeprom.readStructCRC(0x50, read_data)) {
        test_fail("读取失败或CRC校验失败");
        return;
    }
    
    if (read_data.kp == write_data.kp && 
        read_data.ki == write_data.ki && 
        read_data.kd == write_data.kd) {
        test_pass("结构体+CRC读写成功");
    } else {
        test_fail("数据不匹配");
    }
}

/**
 * @brief 测试8：CRC校验检测数据损坏
 */
void test_crc_detection(EEPROM& eeprom) {
    test_begin("CRC校验（检测数据损坏）");
    
    struct TestData {
        uint32_t value;
    };
    
    TestData write_data = {0xDEADBEEF};
    
    // 写入数据+CRC
    if (!eeprom.writeStructCRC(0x60, write_data)) {
        test_fail("写入失败");
        return;
    }
    
    // 人为破坏数据（修改中间的某个字节）
    uint8_t corrupt_byte = 0xFF;
    eeprom.writeByte(0x62, corrupt_byte);
    
    // 尝试读取（应该CRC校验失败）
    TestData read_data;
    if (eeprom.readStructCRC(0x60, read_data)) {
        test_fail("CRC应该检测到数据损坏");
    } else {
        test_pass("CRC成功检测到数据损坏");
    }
    
    // 恢复数据
    eeprom.writeStructCRC(0x60, write_data);
}

/**
 * @brief 测试9：页边界跨越
 */
void test_page_boundary(EEPROM& eeprom) {
    test_begin("页边界跨越");
    
    // 24C02页大小为8字节，从0x06开始写16字节会跨越两个页边界
    uint8_t write_data[16];
    uint8_t read_data[16];
    
    for (int i = 0; i < 16; i++) {
        write_data[i] = i + 100;
    }
    
    if (!eeprom.writeBytes(0x06, write_data, 16)) {
        test_fail("写入失败");
        return;
    }
    
    if (!eeprom.readBytes(0x06, read_data, 16)) {
        test_fail("读取失败");
        return;
    }
    
    bool match = true;
    for (int i = 0; i < 16; i++) {
        if (read_data[i] != write_data[i]) {
            match = false;
            break;
        }
    }
    
    if (match) {
        test_pass("页边界跨越处理正确");
    } else {
        test_fail("页边界数据不匹配");
    }
}

/**
 * @brief 测试10：地址越界保护
 */
void test_address_overflow(EEPROM& eeprom) {
    test_begin("地址越界保护");
    
    uint8_t data[10] = {0};
    
    // 尝试写入超出范围的地址（256字节容量，0x00-0xFF）
    bool should_fail = eeprom.writeBytes(0xFC, data, 10);  // 0xFC+10 > 0xFF
    
    if (!should_fail) {
        test_pass("地址越界保护有效");
    } else {
        test_fail("未检测到地址越界");
    }
}

/**
 * @brief 测试11：填充功能
 */
void test_fill(EEPROM& eeprom) {
    test_begin("填充功能");
    
    // 填充0x70-0x7F为0xAA
    if (!eeprom.fill(0x70, 0xAA, 16)) {
        test_fail("填充失败");
        return;
    }
    
    // 读取并验证
    uint8_t read_data[16];
    if (!eeprom.readBytes(0x70, read_data, 16)) {
        test_fail("读取失败");
        return;
    }
    
    bool all_match = true;
    for (int i = 0; i < 16; i++) {
        if (read_data[i] != 0xAA) {
            all_match = false;
            break;
        }
    }
    
    if (all_match) {
        test_pass("填充功能正确");
    } else {
        test_fail("填充数据不匹配");
    }
}

/* ========== 主程序 ========== */

extern "C" int main(void) {
    /* 1. HAL库初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 2. 初始化调试串口 */
    MX_USART1_UART_Init();
    Debug_Enable();
    
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("       EEPROM功能测试程序\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("芯片型号: 24C02\r\n");
    Debug_Printf("存储容量: 256字节\r\n");
    Debug_Printf("通信接口: I2C (PB10=SCL, PB11=SDA)\r\n");
    Debug_Printf("========================================\r\n");
    
    HAL_Delay(1000);
    
    /* 3. 初始化EEPROM */
    Debug_Printf("\r\n[INIT] 正在初始化EEPROM...\r\n");
    
    EEPROM eeprom;
    if (!eeprom.init()) {
        Debug_Printf("[ERROR] EEPROM初始化失败！\r\n");
        Debug_Printf("[ERROR] 请检查硬件连接：\r\n");
        Debug_Printf("        1. PB10连接到SCL\r\n");
        Debug_Printf("        2. PB11连接到SDA\r\n");
        Debug_Printf("        3. 有4.7kΩ上拉电阻\r\n");
        Debug_Printf("        4. 24C02有供电\r\n");
        while (1) {
            HAL_Delay(1000);
        }
    }
    
    Debug_Printf("[OK] EEPROM初始化成功\r\n");
    HAL_Delay(1000);
    
    /* 4. 运行测试 */
    Debug_Printf("\r\n========== 开始运行测试 ==========\r\n");
    
    test_device_detection(eeprom);
    HAL_Delay(500);
    
    test_single_byte(eeprom);
    HAL_Delay(500);
    
    test_multi_bytes(eeprom);
    HAL_Delay(500);
    
    test_integer(eeprom);
    HAL_Delay(500);
    
    test_float(eeprom);
    HAL_Delay(500);
    
    test_struct(eeprom);
    HAL_Delay(500);
    
    test_struct_crc(eeprom);
    HAL_Delay(500);
    
    test_crc_detection(eeprom);
    HAL_Delay(500);
    
    test_page_boundary(eeprom);
    HAL_Delay(500);
    
    test_address_overflow(eeprom);
    HAL_Delay(500);
    
    test_fill(eeprom);
    HAL_Delay(500);
    
    /* 5. 打印测试结果 */
    print_test_summary();
    
    /* 6. 完成 */
    Debug_Printf("\r\n测试完成！\r\n");
    
    while (1) {
        HAL_Delay(1000);
    }
}


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