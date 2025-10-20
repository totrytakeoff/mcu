#ifndef DEBUG_HPP
#define DEBUG_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief 调试串口选择枚举
 */
typedef enum {
    DEBUG_UART_1 = 1,    // 使用USART1
    DEBUG_UART_2 = 2     // 使用USART2
} DebugUart_t;

/**
 * @brief 调试模式控制
 * 设置为true启用调试输出，设置为false禁用调试输出
 */
extern volatile bool g_debug_enabled;

/**
 * @brief 当前使用的调试串口
 */
extern UART_HandleTypeDef* g_debug_uart;

/**
 * @brief 启用调试输出
 */
void Debug_Enable(void);

/**
 * @brief 禁用调试输出
 */
void Debug_Disable(void);

/**
 * @brief 检查调试是否启用
 * @return true-启用，false-禁用
 */
bool Debug_IsEnabled(void);

/**
 * @brief 设置调试串口
 * @param uart 串口选择 (DEBUG_UART_1 或 DEBUG_UART_2)
 * 
 * 使用示例：
 *   Debug_SetUart(DEBUG_UART_1);  // 使用USART1进行调试
 *   Debug_SetUart(DEBUG_UART_2);  // 使用USART2进行调试
 */
void Debug_SetUart(DebugUart_t uart);

/**
 * @brief 设置调试串口（使用句柄）
 * @param huart 串口句柄指针
 * 
 * 使用示例：
 *   Debug_SetUartHandle(&huart1);  // 使用USART1
 *   Debug_SetUartHandle(&huart2);  // 使用USART2
 */
void Debug_SetUartHandle(UART_HandleTypeDef* huart);

/**
 * @brief 获取当前调试串口
 * @return 当前使用的串口句柄指针
 */
UART_HandleTypeDef* Debug_GetUart(void);

/**
 * @brief 调试打印函数（仅在调试模式启用时输出）
 * @param format 格式化字符串
 * @param ... 可变参数
 * 
 * 使用示例：
 *   Debug_Printf("传感器值: %d\r\n", value);
 *   Debug_Printf("温度: %d°C, 湿度: %d%%\r\n", temp, humi);
 */
void Debug_Printf(const char* format, ...);

/**
 * @brief 无条件打印函数（不受调试模式控制，总是输出）
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Debug_Print_Always(const char* format, ...);

/**
 * @brief printf重定向函数
 * 重定向标准printf到设置的调试串口
 * 受调试模式控制
 */
#ifdef __GNUC__
int _write(int file, char *ptr, int len);
#endif

#ifdef __cplusplus
}
#endif




#endif // DEBUG_HPP