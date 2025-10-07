/*============================================================================
 *                        PWM控制头文件 - PlatformIO版本
 *============================================================================*/

#ifndef __PWM_CONTROL_H
#define __PWM_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/*============================================================================
 *                          舵机控制参数
 *============================================================================*/

/* 舵机 CCR 值定义 (基于 5μs 分辨率)
 * 1.0ms → 200 ticks → 0°
 * 1.5ms → 300 ticks → 90° (中位)
 * 2.0ms → 400 ticks → 180°
 */
#define SERVO_MIN_CCR                 200     // 1.0ms - 最小角度
#define SERVO_MID_CCR                 300     // 1.5ms - 中间角度
#define SERVO_MAX_CCR                 400     // 2.0ms - 最大角度

/*============================================================================
 *                          LED PWM 控制参数
 *============================================================================*/

/* LED PWM 控制 (基于相同的 50Hz 频率)
 * 对于LED控制，我们关心的是占空比而非绝对脉宽
 * 占空比 = CCR / (ARR + 1) * 100%
 */
#define LED_PWM_MIN                   0       // 0% 占空比 - 最暗
#define LED_PWM_25                    1000    // 25% 占空比
#define LED_PWM_50                    2000    // 50% 占空比
#define LED_PWM_75                    3000    // 75% 占空比
#define LED_PWM_MAX                   4000    // 100% 占空比 - 最亮

/*============================================================================
 *                          函数声明
 *============================================================================*/

/* 舵机控制函数 */
void Servo_SetAngle(uint8_t angle);           // 设置舵机角度 (0-180°)
void Servo_SetPosition(uint8_t position);     // 设置舵机到预定义位置
uint16_t Servo_AngleToCCR(uint8_t angle);     // 角度转CCR值

/* LED PWM 控制函数 */
void LED_SetBrightness(uint8_t brightness);   // 设置LED亮度 (0-100%)
void LED_SetCCR(uint16_t ccr_value);          // 直接设置LED的CCR值

/* 通用PWM控制函数 */
void PWM_SetChannel1(uint16_t ccr_value);     // 设置通道1 CCR值
void PWM_SetChannel2(uint16_t ccr_value);     // 设置通道2 CCR值

/* 外部变量 */
extern TIM_HandleTypeDef htim2;

#ifdef __cplusplus
}
#endif

#endif /* __PWM_CONTROL_H */