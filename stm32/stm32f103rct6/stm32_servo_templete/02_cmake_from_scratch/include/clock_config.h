/*============================================================================
 *                        时钟配置头文件 - CMake从零开始项目
 *============================================================================*/

#ifndef __CLOCK_CONFIG_H
#define __CLOCK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 时钟配置参数 */
#define HSE_FREQUENCY           8000000     // 外部晶振频率 8MHz
#define HSI_FREQUENCY           8000000     // 内部振荡器频率 8MHz
#define LSE_FREQUENCY           32768       // 外部低速晶振频率 32.768kHz
#define LSI_FREQUENCY           40000       // 内部低速振荡器频率 40kHz

#define TARGET_SYSCLK_FREQ      72000000    // 目标系统时钟频率 72MHz
#define TARGET_HCLK_FREQ        72000000    // AHB时钟频率 72MHz
#define TARGET_PCLK1_FREQ       36000000    // APB1时钟频率 36MHz
#define TARGET_PCLK2_FREQ       72000000    // APB2时钟频率 72MHz

/* PLL配置参数 */
#define PLL_SOURCE              RCC_PLLSOURCE_HSE       // PLL时钟源：外部晶振
#define PLL_MUL                 RCC_PLL_MUL9            // PLL倍频：9倍
#define HSE_PREDIV              RCC_HSE_PREDIV_DIV1     // HSE预分频：1分频

/* 总线分频配置 */
#define AHB_PRESCALER           RCC_SYSCLK_DIV1         // AHB预分频：1分频
#define APB1_PRESCALER          RCC_HCLK_DIV2           // APB1预分频：2分频
#define APB2_PRESCALER          RCC_HCLK_DIV1           // APB2预分频：1分频

/* 时钟配置结构体 */
typedef struct {
    uint32_t sysclk_freq;       // 系统时钟频率
    uint32_t hclk_freq;         // AHB时钟频率
    uint32_t pclk1_freq;        // APB1时钟频率
    uint32_t pclk2_freq;        // APB2时钟频率
    uint32_t tim_clk_freq;      // 定时器时钟频率
    bool     hse_ready;         // HSE是否就绪
    bool     pll_ready;         // PLL是否就绪
} Clock_Status_t;

/* 函数声明 */
bool Clock_Config(void);
bool Clock_HSE_Config(void);
bool Clock_PLL_Config(void);
bool Clock_SystemClockConfig(void);

/* 时钟状态获取函数 */
Clock_Status_t Clock_GetStatus(void);
uint32_t Clock_GetSysClockFreq(void);
uint32_t Clock_GetHCLKFreq(void);
uint32_t Clock_GetPCLK1Freq(void);
uint32_t Clock_GetPCLK2Freq(void);
void Clock_UpdateStatus(void);

/* 时钟输出函数（用于调试） */
void Clock_MCO_Config(uint32_t mco_source);

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_CONFIG_H */