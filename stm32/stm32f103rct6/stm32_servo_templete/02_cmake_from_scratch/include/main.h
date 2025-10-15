/*============================================================================
 *                        Main Header - CMake从零开始项目
 *============================================================================*/

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含文件 */
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* 系统配置 */
#define SYSTEM_CLOCK_FREQ   72000000    // 系统时钟频率 72MHz
#define APB1_CLOCK_FREQ     36000000    // APB1时钟频率 36MHz  
#define APB2_CLOCK_FREQ     72000000    // APB2时钟频率 72MHz

/* 调试配置 */
#ifdef DEBUG
    #define DBG_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DBG_PRINTF(fmt, ...)
#endif

/* 错误处理 */
void Error_Handler(void);

/* 系统函数 */
void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */