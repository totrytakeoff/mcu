/*============================================================================
 *                        GPIO驱动头文件 - CMake从零开始项目
 *============================================================================*/

#ifndef __GPIO_DRIVER_H
#define __GPIO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* LED引脚定义 */
#define LED_R_PORT              GPIOB
#define LED_R_PIN               GPIO_PIN_5
#define LED_R_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define LED_G_PORT              GPIOB  
#define LED_G_PIN               GPIO_PIN_0
#define LED_G_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define LED_B_PORT              GPIOB
#define LED_B_PIN               GPIO_PIN_1
#define LED_B_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

/* 按键引脚定义（如果有的话） */
#define KEY_PORT                GPIOA
#define KEY_PIN                 GPIO_PIN_8
#define KEY_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()

/* LED颜色枚举 */
typedef enum {
    LED_COLOR_OFF = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_YELLOW,
    LED_COLOR_PURPLE,
    LED_COLOR_CYAN,
    LED_COLOR_WHITE
} LED_Color_t;

/* GPIO驱动函数 */
bool GPIO_Init(void);
void GPIO_DeInit(void);

/* LED控制函数 */
void GPIO_LED_On(GPIO_TypeDef* port, uint16_t pin);
void GPIO_LED_Off(GPIO_TypeDef* port, uint16_t pin);
void GPIO_LED_Toggle(GPIO_TypeDef* port, uint16_t pin);

/* RGB LED控制函数 */
void GPIO_RGB_SetColor(LED_Color_t color);
void GPIO_RGB_Off(void);

/* 单色LED控制函数 */
void GPIO_LED_Red_On(void);
void GPIO_LED_Red_Off(void);
void GPIO_LED_Green_On(void);
void GPIO_LED_Green_Off(void);
void GPIO_LED_Blue_On(void);
void GPIO_LED_Blue_Off(void);

/* 按键检测函数 */
bool GPIO_KEY_IsPressed(void);
bool GPIO_KEY_WaitPress(uint32_t timeout_ms);

/* 通用GPIO函数 */
void GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);
GPIO_PinState GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_DRIVER_H */