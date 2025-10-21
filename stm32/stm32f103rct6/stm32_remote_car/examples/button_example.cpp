/**
 * @file    button_example.cpp
 * @brief   Button类综合测试示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 演示Button类的所有功能：
 * 1. 基本按下/释放检测
 * 2. 长按检测（1秒、2秒、3秒）
 * 3. 连击计数
 * 4. 按下时长显示
 * 5. 实时状态显示
 * 
 * 硬件连接：
 * - 按钮：PD2 <-> GND（上拉模式）
 * - 调试串口：USART1（PA9/PA10，115200）
 */

#include "button.hpp"
#include "debug.hpp"
#include "gpio.h"
#include "stm32f1xx_hal.h"
#include "usart.h"

extern "C" {
void SystemClock_Config(void);
}

/* ========== 主测试程序 ========== */

int main(void) {
    /* ========== 1. HAL库初始化 ========== */
    HAL_Init();
    SystemClock_Config();
    
    /* ========== 2. GPIO初始化 ========== */
    MX_GPIO_Init();
    
    /* ========== 3. USART1初始化（调试串口） ========== */
    MX_USART1_UART_Init();
    Debug_Enable();
    
    /* ========== 4. 按钮初始化 ========== */
    Button btn(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 50); // 50ms防抖
    btn.init();
    
    /* ========== 5. 测试变量 ========== */
    uint32_t press_count = 0;           // 按下总次数
    uint32_t long_press_1s_count = 0;   // 1秒长按次数
    uint32_t long_press_2s_count = 0;   // 2秒长按次数
    uint32_t long_press_3s_count = 0;   // 3秒长按次数
    bool long_press_1s_handled = false;
    bool long_press_2s_handled = false;
    bool long_press_3s_handled = false;
    uint32_t last_status_print = 0;     // 上次状态打印时间
    
    /* ========== 6. 欢迎信息 ========== */
    Debug_Printf("\r\n");
    Debug_Printf("╔════════════════════════════════════════════════════╗\r\n");
    Debug_Printf("║         Button 类综合测试程序                      ║\r\n");
    Debug_Printf("╚════════════════════════════════════════════════════╝\r\n");
    Debug_Printf("\r\n");
    Debug_Printf("测试功能：\r\n");
    Debug_Printf("  1. 短按检测\r\n");
    Debug_Printf("  2. 长按检测（1秒/2秒/3秒）\r\n");
    Debug_Printf("  3. 释放事件\r\n");
    Debug_Printf("  4. 按下时长显示\r\n");
    Debug_Printf("  5. 统计计数\r\n");
    Debug_Printf("\r\n");
    Debug_Printf("硬件连接：按钮 PD2 <-> GND\r\n");
    Debug_Printf("防抖时间：50ms\r\n");
    Debug_Printf("\r\n");
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("开始测试，请按下按钮...\r\n");
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("\r\n");
    
    /* ========== 7. 主循环 ========== */
    while (1) {
        uint32_t now = HAL_GetTick();
        
        /* --- 检测按下事件 --- */
        if (btn.isPressed()) {
            press_count++;
            Debug_Printf("【按下】第 %lu 次按下\r\n", press_count);
            
            // 重置长按标志
            long_press_1s_handled = false;
            long_press_2s_handled = false;
            long_press_3s_handled = false;
        }
        
        /* --- 检测1秒长按 --- */
        if (btn.isLongPressed(1000) && !long_press_1s_handled) {
            long_press_1s_count++;
            Debug_Printf("  ⏱️  【长按 1秒】已触发（第 %lu 次）\r\n", long_press_1s_count);
            long_press_1s_handled = true;
        }
        
        /* --- 检测2秒长按 --- */
        if (btn.isLongPressed(2000) && !long_press_2s_handled) {
            long_press_2s_count++;
            Debug_Printf("  ⏱️  【长按 2秒】已触发（第 %lu 次）\r\n", long_press_2s_count);
            long_press_2s_handled = true;
        }
        
        /* --- 检测3秒长按 --- */
        if (btn.isLongPressed(3000) && !long_press_3s_handled) {
            long_press_3s_count++;
            Debug_Printf("  ⏱️  【长按 3秒】已触发（第 %lu 次）\r\n", long_press_3s_count);
            long_press_3s_handled = true;
        }
        
        /* --- 检测释放事件 --- */
        if (btn.isReleased()) {
            uint32_t duration = btn.getPressedDuration();
            Debug_Printf("【释放】按下时长：%lu ms\r\n", duration);
            
            // 分类显示
            if (duration < 1000) {
                Debug_Printf("  → 短按\r\n");
            } else if (duration < 2000) {
                Debug_Printf("  → 1秒长按\r\n");
            } else if (duration < 3000) {
                Debug_Printf("  → 2秒长按\r\n");
            } else {
                Debug_Printf("  → 3秒+长按\r\n");
            }
            
            Debug_Printf("\r\n");
        }
        
        /* --- 每5秒打印一次统计信息 --- */
        if (now - last_status_print >= 5000) {
            last_status_print = now;
            
            Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
            Debug_Printf("【统计信息】运行时间：%lu 秒\r\n", now / 1000);
            Debug_Printf("  • 总按下次数：%lu\r\n", press_count);
            Debug_Printf("  • 1秒长按：%lu 次\r\n", long_press_1s_count);
            Debug_Printf("  • 2秒长按：%lu 次\r\n", long_press_2s_count);
            Debug_Printf("  • 3秒长按：%lu 次\r\n", long_press_3s_count);
            
            // 显示当前按钮状态
            if (btn.read()) {
                uint32_t duration = btn.getPressedDuration();
                Debug_Printf("  • 当前状态：按下中（已持续 %lu ms）\r\n", duration);
            } else {
                Debug_Printf("  • 当前状态：未按下\r\n");
            }
            
            Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
            Debug_Printf("\r\n");
        }
        
        /* --- 延时 --- */
        HAL_Delay(10);  // 10ms轮询间隔
    }
}

/**
 * @brief 错误处理函数
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // 停在这里
    }
}
