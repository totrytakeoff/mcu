/**
 * @file    debug.cpp
 * @brief   串口调试输出封装（基于printf重定向）
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能说明：
 * 1. 支持手动开关调试模式
 * 2. printf重定向到USART2串口
 * 3. 提供Debug_Printf()函数，仅在调试模式启用时输出
 * 4. 提供Debug_Print_Always()函数，不受调试模式控制
 * 
 * 使用示例：
 *   // 启用调试
 *   Debug_Enable();
 *   
 *   // 使用printf（受调试模式控制）
 *   printf("系统启动\r\n");
 *   
 *   // 使用Debug_Printf（受调试模式控制）
 *   Debug_Printf("传感器值: %d\r\n", sensor_value);
 *   
 *   // 使用Debug_Print_Always（总是输出）
 *   Debug_Print_Always("错误: %d\r\n", error_code);
 *   
 *   // 禁用调试
 *   Debug_Disable();
 */

#include "debug.hpp"
#include "usart.h"
#include <string.h>

/* 调试模式标志（默认启用） */
volatile bool g_debug_enabled = true;

/* 当前使用的调试串口（默认USART1） */
UART_HandleTypeDef* g_debug_uart = &huart1;

/**
 * @brief 启用调试输出
 */
void Debug_Enable(void)
{
    g_debug_enabled = true;
}

/**
 * @brief 禁用调试输出
 */
void Debug_Disable(void)
{
    g_debug_enabled = false;
}

/**
 * @brief 检查调试是否启用
 * @return true-启用，false-禁用
 */
bool Debug_IsEnabled(void)
{
    return g_debug_enabled;
}

/**
 * @brief 设置调试串口
 * @param uart 串口选择 (DEBUG_UART_1 或 DEBUG_UART_2)
 */
void Debug_SetUart(DebugUart_t uart)
{
    if (uart == DEBUG_UART_1) {
        g_debug_uart = &huart1;
    } else if (uart == DEBUG_UART_2) {
        g_debug_uart = &huart2;
    }
}

/**
 * @brief 设置调试串口（使用句柄）
 * @param huart 串口句柄指针
 */
void Debug_SetUartHandle(UART_HandleTypeDef* huart)
{
    if (huart != NULL) {
        g_debug_uart = huart;
    }
}

/**
 * @brief 获取当前调试串口
 * @return 当前使用的串口句柄指针
 */
UART_HandleTypeDef* Debug_GetUart(void)
{
    return g_debug_uart;
}

/**
 * @brief 调试打印函数（仅在调试模式启用时输出）
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Debug_Printf(const char* format, ...)
{
    if (!g_debug_enabled) {
        return;  // 调试模式未启用，直接返回
    }
    
    char buffer[256];  // 缓冲区大小可根据需要调整
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // 通过设置的调试串口发送
    HAL_UART_Transmit(g_debug_uart, (uint8_t*)buffer, strlen(buffer), 1000);
}

/**
 * @brief 无条件打印函数（不受调试模式控制，总是输出）
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Debug_Print_Always(const char* format, ...)
{
    char buffer[256];  // 缓冲区大小可根据需要调整
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // 通过设置的调试串口发送
    HAL_UART_Transmit(g_debug_uart, (uint8_t*)buffer, strlen(buffer), 1000);
}

/**
 * @brief printf重定向函数（GCC编译器）
 * @param file 文件描述符
 * @param ptr 数据指针
 * @param len 数据长度
 * @return 写入的字节数
 * 
 * 说明：
 * - 重定向标准printf到设置的调试串口
 * - 受调试模式控制，当g_debug_enabled=false时不输出
 */
#ifdef __GNUC__
int _write(int file, char *ptr, int len)
{
    // 检查调试模式
    if (!g_debug_enabled) {
        return len;  // 假装写入成功，但实际不输出
    }
    
    // 通过设置的调试串口发送数据
    HAL_UART_Transmit(g_debug_uart, (uint8_t*)ptr, len, 1000);
    
    return len;
}
#endif

/**
 * @brief fputc重定向函数（ARMCC/Keil编译器）
 * @param ch 字符
 * @param f 文件指针
 * @return 写入的字符
 */
#ifdef __CC_ARM
int fputc(int ch, FILE *f)
{
    // 检查调试模式
    if (!g_debug_enabled) {
        return ch;  // 假装写入成功，但实际不输出
    }
    
    // 通过设置的调试串口发送单个字符
    HAL_UART_Transmit(g_debug_uart, (uint8_t*)&ch, 1, 1000);
    
    return ch;
}
#endif

/**
 * @brief fgetc重定向函数（ARMCC/Keil编译器）
 * @param f 文件指针
 * @return 读取的字符
 */
#ifdef __CC_ARM
int fgetc(FILE *f)
{
    uint8_t ch = 0;
    
    // 通过设置的调试串口接收单个字符
    HAL_UART_Receive(g_debug_uart, &ch, 1, 0xFFFF);
    
    return ch;
}
#endif
