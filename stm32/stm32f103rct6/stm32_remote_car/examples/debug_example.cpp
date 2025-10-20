/**
 * @file    debug_example.cpp
 * @brief   调试系统使用示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 此文件演示如何使用封装的调试系统
 */

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "debug.hpp"

void debug_example(void)
{
    /* ========== 方式1: 使用Debug_Printf（推荐） ========== */
    
    // 启用调试模式
    Debug_Enable();
    
    // 会输出到串口
    Debug_Printf("调试模式已启用\r\n");
    Debug_Printf("传感器值: %d\r\n", 1234);
    Debug_Printf("温度: %d°C, 湿度: %d%%\r\n", 25, 60);
    
    // 禁用调试模式
    Debug_Disable();
    
    // 不会输出到串口（因为调试已禁用）
    Debug_Printf("这条消息不会被输出\r\n");
    
    // 重新启用调试
    Debug_Enable();
    
    // 会输出到串口
    Debug_Printf("调试模式重新启用\r\n");
    
    
    /* ========== 方式2: 使用标准printf（支持重定向） ========== */
    
    // 启用调试模式
    Debug_Enable();
    
    // 标准printf会被重定向到USART2，受调试模式控制
    printf("使用标准printf输出\r\n");
    printf("数值: %d, 字符: %c, 字符串: %s\r\n", 100, 'A', "Hello");
    
    // 禁用调试模式
    Debug_Disable();
    
    // 不会输出（因为调试已禁用）
    printf("这条printf消息不会被输出\r\n");
    
    
    /* ========== 方式3: 使用Debug_Print_Always（无条件输出） ========== */
    
    // 即使调试模式禁用，也会输出（用于重要信息或错误）
    Debug_Print_Always("重要提示: 这条消息总是会输出\r\n");
    Debug_Print_Always("错误代码: %d\r\n", 0xFF);
    
    
    /* ========== 方式4: 条件编译（编译期控制） ========== */
    
    #define DEBUG_ENABLE 1  // 可以在编译时控制
    
    #if DEBUG_ENABLE
        Debug_Printf("编译时调试信息\r\n");
    #endif
    
    
    /* ========== 方式5: 运行时动态控制 ========== */
    
    int sensor_value = 0;
    
    for (int i = 0; i < 10; i++)
    {
        sensor_value = i * 100;
        
        // 每隔5次输出一次（减少调试信息量）
        if (i % 5 == 0)
        {
            Debug_Enable();
            Debug_Printf("循环 %d: 传感器 = %d\r\n", i, sensor_value);
        }
        else
        {
            Debug_Disable();
        }
        
        HAL_Delay(100);
    }
    
    
    /* ========== 方式6: 检查调试状态 ========== */
    
    if (Debug_IsEnabled())
    {
        Debug_Printf("调试当前是启用的\r\n");
    }
    else
    {
        // 即使调试禁用，也可以通过Always函数输出
        Debug_Print_Always("调试当前是禁用的\r\n");
    }
    
    
    /* ========== 实际应用示例 ========== */
    
    // 场景1: 正常运行时关闭调试，节省性能
    Debug_Disable();
    
    // 场景2: 出现问题时打开调试
    int error_code = check_system_status();
    if (error_code != 0)
    {
        Debug_Enable();
        Debug_Printf("系统错误: %d\r\n", error_code);
        Debug_Printf("详细信息: 传感器异常\r\n");
    }
    
    // 场景3: 只在特定模块启用调试
    #define DEBUG_MOTOR_MODULE 1
    
    #if DEBUG_MOTOR_MODULE
        Debug_Enable();
        Debug_Printf("电机模块调试信息\r\n");
    #else
        Debug_Disable();
    #endif
}

/**
 * @brief 模拟的系统状态检查函数
 */
int check_system_status(void)
{
    // 模拟返回错误代码
    return 0;  // 0表示正常
}

/**
 * @brief 在main函数中的使用示例
 */
void main_example(void)
{
    /* 系统初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 初始化串口（必须在使用调试前初始化） */
    MX_USART2_UART_Init();
    
    /* 启动时输出系统信息 */
    Debug_Enable();
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("    STM32F103 遥控小车系统启动\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("固件版本: v1.0.0\r\n");
    Debug_Printf("编译时间: %s %s\r\n", __DATE__, __TIME__);
    Debug_Printf("调试串口: USART2 @ 115200bps\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("\r\n");
    
    /* 初始化其他外设 */
    // ...
    
    /* 正常运行时可以关闭调试，提高性能 */
    Debug_Disable();
    
    /* 主循环 */
    while (1)
    {
        // 你的代码...
        
        // 需要调试时临时打开
        // Debug_Enable();
        // Debug_Printf("循环执行中...\r\n");
        // Debug_Disable();
        
        HAL_Delay(1000);
    }
}

/**
 * @brief 系统时钟配置（占位符）
 */
void SystemClock_Config(void)
{
    // 实际的时钟配置代码
}
