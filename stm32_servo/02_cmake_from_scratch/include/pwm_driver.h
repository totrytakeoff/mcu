/*============================================================================
 *                        PWM驱动头文件 - CMake从零开始项目
 *============================================================================*/

#ifndef __PWM_DRIVER_H
#define __PWM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* PWM配置参数 */
#define PWM_TIMER_INSTANCE      TIM2
#define PWM_TIMER_CHANNEL_1     TIM_CHANNEL_1   // PA0 - 舵机控制
#define PWM_TIMER_CHANNEL_2     TIM_CHANNEL_2   // PA1 - LED控制

#define PWM_FREQUENCY           50              // PWM频率 50Hz
#define PWM_PERIOD              4000            // PWM周期计数值
#define PWM_PRESCALER           359             // 预分频值

/* 舵机控制参数 */
#define SERVO_MIN_PULSE         200             // 1.0ms 对应的CCR值
#define SERVO_MID_PULSE         300             // 1.5ms 对应的CCR值  
#define SERVO_MAX_PULSE         400             // 2.0ms 对应的CCR值

/* LED控制参数 */
#define LED_PWM_MIN             0               // 最小亮度
#define LED_PWM_MAX             PWM_PERIOD      // 最大亮度

/* PWM驱动结构体 */
typedef struct {
    TIM_HandleTypeDef   htim;
    bool                initialized;
    uint16_t            servo_position;        // 当前舵机位置
    uint16_t            led_brightness;        // 当前LED亮度
} PWM_Driver_t;

/* 函数声明 */
bool PWM_Init(void);
bool PWM_DeInit(void);

/* 舵机控制函数 */
bool PWM_SetServoAngle(uint8_t angle);          // 设置舵机角度 (0-180°)
bool PWM_SetServoPosition(uint16_t pulse);      // 设置舵机脉宽
uint16_t PWM_AngleToPulse(uint8_t angle);       // 角度转脉宽

/* LED控制函数 */
bool PWM_SetLEDBrightness(uint8_t brightness);  // 设置LED亮度 (0-100%)
bool PWM_SetLEDPulse(uint16_t pulse);           // 设置LED脉宽

/* 通用PWM函数 */
bool PWM_SetChannelPulse(uint32_t channel, uint16_t pulse);
uint16_t PWM_GetChannelPulse(uint32_t channel);

/* 获取驱动状态 */
PWM_Driver_t* PWM_GetDriver(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWM_DRIVER_H */