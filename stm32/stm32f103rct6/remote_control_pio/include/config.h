/**
 * @file    config.h
 * @brief   TLE100 遥控器硬件配置
 * @author  AI Assistant
 * @date    2024
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <8052.h>

// ========== 引脚定义 (使用 __at 宏) ==========
// E49 无线模块
__sbit __at (0x91) E49_M0;   // P1.1
__sbit __at (0x92) E49_M1;   // P1.2

// 方向按键 (P0口)
__sbit __at (0x80) KEY_LEFT;     // P0.0
__sbit __at (0x81) KEY_RIGHT;    // P0.1
__sbit __at (0x82) KEY_FORWARD;  // P0.2
__sbit __at (0x83) KEY_BACK;     // P0.3

// 速度调节按键 (P0口)
__sbit __at (0x84) KEY_SPEED_UP;    // P0.4
__sbit __at (0x85) KEY_SPEED_DOWN;  // P0.5

// 功能按键
__sbit __at (0x86) KEY_F1;  // P0.6
__sbit __at (0x87) KEY_F2;  // P0.7
__sbit __at (0x93) KEY_F3;  // P1.3
__sbit __at (0x94) KEY_F4;  // P1.4

// ========== 配置参数 ==========
#define FOSC            11059200UL  // 晶振频率 11.0592MHz
#define BAUD_RATE       9600        // 串口波特率
#define KEY_SCAN_INTERVAL   5       // 按键扫描间隔 (ms) - 快速扫描
#define KEY_DEBOUNCE_COUNT  3       // 去抖计数 (3次 * 5ms = 15ms)
#define KEY_REPEAT_DELAY    200     // 按键重复延时 (ms)
#define KEY_REPEAT_RATE     50      // 按键重复速率 (ms)

// ========== E49 模式定义 ==========
#define E49_MODE_TRANSPARENT  0  // 透传模式 (M0=0, M1=0)
#define E49_MODE_WAKEUP       1  // 唤醒模式 (M0=1, M1=0)
#define E49_MODE_POWERSAVE    2  // 省电模式 (M0=0, M1=1)
#define E49_MODE_CONFIG       3  // 配置模式 (M0=1, M1=1)

#endif // __CONFIG_H__
