/*============================================================================
 *                        LED控制头文件 - PlatformIO版本
 *============================================================================*/

#ifndef __LED_CONTROL_H
#define __LED_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/*============================================================================
 *                          GPIO引脚定义
 *============================================================================*/

/* RGB LED引脚定义 (假设使用PB0, PB1, PB5) */
#define LED_R_GPIO_PORT               GPIOB
#define LED_R_PIN                     GPIO_PIN_5

#define LED_G_GPIO_PORT               GPIOB
#define LED_G_PIN                     GPIO_PIN_0

#define LED_B_GPIO_PORT               GPIOB
#define LED_B_PIN                     GPIO_PIN_1

/*============================================================================
 *                          LED颜色定义
 *============================================================================*/

typedef enum {
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_YELLOW,
    LED_PURPLE,
    LED_CYAN,
    LED_WHITE
} LED_Color_t;

/*============================================================================
 *                          函数声明
 *============================================================================*/

void LED_Init(void);
void LED_SetColor(LED_Color_t color);
void LED_Toggle(LED_Color_t color);

/* 单独控制函数 */
void LED_R_On(void);
void LED_R_Off(void);
void LED_G_On(void);
void LED_G_Off(void);
void LED_B_On(void);
void LED_B_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_CONTROL_H */