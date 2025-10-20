/**
 * @file    debug_uart_selection_example.cpp
 * @brief   动态选择调试串口示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 本示例展示如何动态选择使用USART1或USART2进行调试输出
 */

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "debug.hpp"

extern "C" {
    void SystemClock_Config(void);
}

/* ========== 示例1: 使用枚举方式选择串口 ========== */
void example1_enum_selection(void)
{
    /* 初始化两个串口 */
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    
    /* 启用调试 */
    Debug_Enable();
    
    /* 使用USART2进行调试（默认） */
    Debug_SetUart(DEBUG_UART_2);
    Debug_Printf("当前使用USART2进行调试\r\n");
    printf("这条消息通过USART2输出\r\n");
    
    HAL_Delay(1000);
    
    /* 切换到USART1进行调试 */
    Debug_SetUart(DEBUG_UART_1);
    Debug_Printf("切换到USART1进行调试\r\n");
    printf("这条消息通过USART1输出\r\n");
    
    HAL_Delay(1000);
    
    /* 再切换回USART2 */
    Debug_SetUart(DEBUG_UART_2);
    Debug_Printf("切换回USART2进行调试\r\n");
}

/* ========== 示例2: 使用句柄方式选择串口 ========== */
void example2_handle_selection(void)
{
    /* 初始化串口 */
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    
    /* 启用调试 */
    Debug_Enable();
    
    /* 使用USART1句柄 */
    Debug_SetUartHandle(&huart1);
    Debug_Printf("使用USART1句柄进行调试\r\n");
    
    HAL_Delay(1000);
    
    /* 使用USART2句柄 */
    Debug_SetUartHandle(&huart2);
    Debug_Printf("使用USART2句柄进行调试\r\n");
}

/* ========== 示例3: 查询当前使用的串口 ========== */
void example3_query_uart(void)
{
    Debug_Enable();
    
    /* 设置为USART1 */
    Debug_SetUart(DEBUG_UART_1);
    
    /* 查询当前串口 */
    UART_HandleTypeDef* current_uart = Debug_GetUart();
    
    if (current_uart == &huart1) {
        Debug_Printf("当前使用USART1\r\n");
    } else if (current_uart == &huart2) {
        Debug_Printf("当前使用USART2\r\n");
    }
}

/* ========== 示例4: 根据条件选择不同串口 ========== */
void example4_conditional_selection(void)
{
    /* 假设有一个配置变量 */
    bool use_wireless_debug = true;  // true=使用USART1(无线), false=使用USART2(有线)
    
    /* 初始化串口 */
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    
    /* 根据条件选择调试串口 */
    if (use_wireless_debug) {
        Debug_SetUart(DEBUG_UART_1);
        Debug_Enable();
        Debug_Printf("使用USART1进行无线调试\r\n");
    } else {
        Debug_SetUart(DEBUG_UART_2);
        Debug_Enable();
        Debug_Printf("使用USART2进行有线调试\r\n");
    }
}

/* ========== 示例5: 同时输出到两个串口 ========== */
void example5_dual_output(void)
{
    Debug_Enable();
    
    /* 先通过USART1输出 */
    Debug_SetUart(DEBUG_UART_1);
    Debug_Printf("发送到USART1\r\n");
    
    /* 再通过USART2输出 */
    Debug_SetUart(DEBUG_UART_2);
    Debug_Printf("发送到USART2\r\n");
    
    /* 如果需要频繁切换，可以封装成函数 */
    auto send_to_both = [](const char* msg) {
        Debug_SetUart(DEBUG_UART_1);
        Debug_Printf("%s", msg);
        
        Debug_SetUart(DEBUG_UART_2);
        Debug_Printf("%s", msg);
    };
    
    send_to_both("这条消息同时发送到两个串口\r\n");
}

/* ========== 示例6: 实际应用场景 ========== */
void example6_real_world_usage(void)
{
    /*
     * 应用场景：
     * - 开发时使用USART2连接电脑进行调试
     * - 部署后使用USART1通过无线模块远程调试
     */
    
    // 判断是否连接了USB转TTL（可以通过GPIO检测）
    bool usb_connected = true;  // 实际项目中需要检测
    
    if (usb_connected) {
        // 使用USART2进行有线调试
        Debug_SetUart(DEBUG_UART_2);
        Debug_Enable();
        Debug_Printf("\r\n");
        Debug_Printf("=== 开发模式 ===\r\n");
        Debug_Printf("使用USART2 (USB转TTL) 进行调试\r\n");
        Debug_Printf("波特率: 115200\r\n");
    } else {
        // 使用USART1进行无线调试
        Debug_SetUart(DEBUG_UART_1);
        Debug_Enable();
        Debug_Printf("\r\n");
        Debug_Printf("=== 无线模式 ===\r\n");
        Debug_Printf("使用USART1 (E49无线模块) 进行调试\r\n");
        Debug_Printf("波特率: 9600\r\n");
    }
    
    Debug_Printf("\r\n");
    Debug_Printf("系统初始化完成\r\n");
}

/* ========== 示例7: 运行时动态切换 ========== */
void example7_runtime_switching(void)
{
    Debug_Enable();
    
    int count = 0;
    
    while (1) {
        count++;
        
        // 每隔5秒切换一次调试串口
        if (count % 5 == 0) {
            if (Debug_GetUart() == &huart1) {
                Debug_SetUart(DEBUG_UART_2);
                Debug_Printf("切换到USART2\r\n");
            } else {
                Debug_SetUart(DEBUG_UART_1);
                Debug_Printf("切换到USART1\r\n");
            }
        }
        
        Debug_Printf("循环计数: %d\r\n", count);
        
        HAL_Delay(1000);
    }
}

/* ========== 主程序示例 ========== */
int main(void)
{
    /* 系统初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 初始化GPIO */
    // MX_GPIO_Init();
    
    /* 初始化两个串口 */
    MX_USART1_UART_Init();  // 9600波特率，用于E49无线模块
    MX_USART2_UART_Init();  // 115200波特率，用于USB转TTL
    
    /* 默认使用USART2进行调试 */
    Debug_SetUart(DEBUG_UART_2);
    Debug_Enable();
    
    /* 输出启动信息 */
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("  动态串口选择调试系统\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("编译时间: %s %s\r\n", __DATE__, __TIME__);
    Debug_Printf("默认调试串口: USART2 @ 115200bps\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("\r\n");
    
    /* 演示切换到USART1 */
    Debug_Printf("5秒后切换到USART1...\r\n");
    HAL_Delay(5000);
    
    Debug_SetUart(DEBUG_UART_1);
    Debug_Printf("\r\n");
    Debug_Printf("已切换到USART1 @ 9600bps\r\n");
    Debug_Printf("请连接E49无线模块查看此消息\r\n");
    Debug_Printf("\r\n");
    
    HAL_Delay(5000);
    
    /* 切换回USART2 */
    Debug_SetUart(DEBUG_UART_2);
    Debug_Printf("\r\n");
    Debug_Printf("已切换回USART2 @ 115200bps\r\n");
    Debug_Printf("\r\n");
    
    /* 主循环 */
    int loop_count = 0;
    
    while (1)
    {
        loop_count++;
        
        /* 每隔10次切换一次串口（演示） */
        if (loop_count % 10 == 0) {
            if (Debug_GetUart() == &huart1) {
                Debug_SetUart(DEBUG_UART_2);
                Debug_Printf(">>> 切换到USART2\r\n");
            } else {
                Debug_SetUart(DEBUG_UART_1);
                Debug_Printf(">>> 切换到USART1\r\n");
            }
        }
        
        /* 输出调试信息 */
        Debug_Printf("循环: %d, 当前串口: %s\r\n", 
                     loop_count,
                     (Debug_GetUart() == &huart1) ? "USART1" : "USART2");
        
        HAL_Delay(1000);
    }
}

/* ========== 配置建议 ========== */

/*
 * 硬件连接建议：
 * 
 * USART1 (9600bps):
 *   PA9  (TX) -> E49无线模块 RXD
 *   PA10 (RX) -> E49无线模块 TXD
 * 
 * USART2 (115200bps):
 *   PA2  (TX) -> USB转TTL RXD
 *   PA3  (RX) -> USB转TTL TXD
 * 
 * 使用场景：
 *   - 开发阶段：使用USART2通过USB转TTL连接电脑调试
 *   - 现场调试：使用USART1通过无线模块远程调试
 *   - 双路输出：同时输出到两个串口进行对比测试
 */

/*
 * 常见应用：
 * 
 * 1. 开发/生产模式切换
 *    if (is_dev_mode) {
 *        Debug_SetUart(DEBUG_UART_2);  // 开发：有线调试
 *    } else {
 *        Debug_SetUart(DEBUG_UART_1);  // 生产：无线调试
 *    }
 * 
 * 2. 根据按键切换
 *    if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET) {
 *        Debug_SetUart(DEBUG_UART_1);  // 按下按键使用USART1
 *    } else {
 *        Debug_SetUart(DEBUG_UART_2);  // 释放按键使用USART2
 *    }
 * 
 * 3. 自动检测
 *    // 先尝试USART2
 *    Debug_SetUart(DEBUG_UART_2);
 *    // 如果检测到无响应，切换到USART1
 *    if (!uart_is_available(&huart2)) {
 *        Debug_SetUart(DEBUG_UART_1);
 *    }
 */
