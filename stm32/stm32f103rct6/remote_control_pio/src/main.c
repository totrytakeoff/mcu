/**
 * @file    main.c
 * @brief   TLE100 遥控器主程序 - 优化版
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能说明:
 * - 支持同时按下多个按键
 * - 快速响应 (5ms 扫描间隔)
 * - 消息队列防止丢失按键
 * - 非阻塞设计
 * - 按键长按自动重复
 */

#include <8052.h>
#include "config.h"
#include "uart.h"
#include "keys.h"

// ========== 定时器变量 ==========
static unsigned int system_tick = 0;        // 系统滴答计数 (每 1ms 递增)
static unsigned int last_key_scan = 0;      // 上次按键扫描时间

/**
 * @brief Timer0 中断服务函数 (1ms 定时)
 * @note  用于系统时钟和定时任务
 */
void Timer0_ISR(void) __interrupt(1)
{
    // 重装定时器 (1ms @ 11.0592MHz)
    // Timer0 模式1: 16位定时器
    // 定时1ms = 65536 - (11059200 / 12 / 1000) = 65536 - 921.6 = 64614 = 0xFC66
    TH0 = 0xFC;
    TL0 = 0x66;
    
    // 系统滴答递增
    system_tick++;
}

/**
 * @brief 初始化 Timer0 (1ms 定时中断)
 */
void Timer0_Init(void)
{
    // Timer0 模式1 (16位定时器)
    TMOD &= 0xF0;
    TMOD |= 0x01;
    
    // 设置初值 (1ms)
    TH0 = 0xFC;
    TL0 = 0x66;
    
    // 使能 Timer0 中断
    ET0 = 1;
    EA = 1;
    
    // 启动 Timer0
    TR0 = 1;
}

/**
 * @brief 获取系统时间 (毫秒)
 */
unsigned int GetTick(void)
{
    unsigned int tick;
    
    EA = 0;  // 关闭中断
    tick = system_tick;
    EA = 1;  // 开启中断
    
    return tick;
}

/**
 * @brief 初始化 E49 无线模块
 */
void E49_Init(void)
{
    E49_M0 = 0;  // M0 = 0
    E49_M1 = 0;  // M1 = 0
    // 透传模式：UART 数据直接无线发送
}

/**
 * @brief 主函数
 */
void main(void)
{
    KeyMessage msg;
    unsigned int current_tick;
    
    // ========== 初始化 ==========
    Timer0_Init();   // 初始化定时器 (1ms 定时)
    UART_Init();     // 初始化串口 (9600)
    Keys_Init();     // 初始化按键系统
    E49_Init();      // 初始化 E49 模块
    
    // 延时等待模块稳定
    Delay_ms(500);
    
    // ========== 主循环 ==========
    while (1)
    {
        current_tick = GetTick();
        
        // 1. 按键扫描任务 (每 5ms)
        if (current_tick - last_key_scan >= KEY_SCAN_INTERVAL)
        {
            last_key_scan = current_tick;
            Keys_Task();  // 非阻塞按键扫描
        }
        
        // 2. 处理消息队列 (立即处理所有待发送的消息)
        while (Keys_GetMessage(&msg))
        {
            // 发送按键码到 E49 无线模块
            UART_SendChar(msg.cmd);
        }
        
        // 3. 其他任务 (预留)
        // ...
    }
}
