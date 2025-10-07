/*============================================================================
 *                        PWM控制实现文件 - PlatformIO版本
 *============================================================================*/

#include "pwm_control.h"

/*============================================================================
 *                          舵机控制函数
 *============================================================================*/

/**
 * @brief  将角度转换为CCR值
 * @note   角度范围0-180°，对应脉宽1.0-2.0ms，CCR值200-400
 * @param  angle: 角度值 (0-180)
 * @retval CCR值 (200-400)
 */
uint16_t Servo_AngleToCCR(uint8_t angle)
{
    /* 限制角度范围 */
    if (angle > 180) angle = 180;
    
    /* 线性插值计算CCR值
     * 0° → 1.0ms → CCR=200
     * 180° → 2.0ms → CCR=400
     * 公式: CCR = 200 + (angle/180) * 200 */
    return SERVO_MIN_CCR + ((uint32_t)angle * (SERVO_MAX_CCR - SERVO_MIN_CCR)) / 180;
}

/**
 * @brief  设置舵机角度
 * @note   通过TIM2_CH1 (PA0) 控制舵机转到指定角度
 * @param  angle: 目标角度 (0-180°)
 * @retval None
 */
void Servo_SetAngle(uint8_t angle)
{
    uint16_t ccr_value = Servo_AngleToCCR(angle);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_value);
}

/**
 * @brief  设置舵机到预定义位置
 * @note   快速设置舵机到常用位置
 * @param  position: 位置索引
 *         - 0: 最小角度 (0°)
 *         - 1: 中间角度 (90°)
 *         - 2: 最大角度 (180°)
 * @retval None
 */
void Servo_SetPosition(uint8_t position)
{
    uint16_t ccr_values[] = {SERVO_MIN_CCR, SERVO_MID_CCR, SERVO_MAX_CCR};
    
    if (position < 3) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_values[position]);
    }
}

/*============================================================================
 *                          LED PWM 控制函数
 *============================================================================*/

/**
 * @brief  设置LED亮度 (百分比)
 * @note   通过TIM2_CH2 (PA1) 控制LED亮度，使用占空比调节
 * @param  brightness: 亮度百分比 (0-100%)
 * @retval None
 */
void LED_SetBrightness(uint8_t brightness)
{
    /* 限制亮度范围 */
    if (brightness > 100) brightness = 100;
    
    /* 计算CCR值
     * 占空比 = brightness%
     * CCR = (ARR + 1) * brightness / 100 */
    uint16_t ccr_value = ((uint32_t)4000 * brightness) / 100;
    
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, ccr_value);
}

/**
 * @brief  直接设置LED的CCR值
 * @note   直接控制CCR寄存器，提供更精细的控制
 * @param  ccr_value: CCR值 (0-4000)
 * @retval None
 */
void LED_SetCCR(uint16_t ccr_value)
{
    /* 限制CCR值范围 */
    if (ccr_value > 4000) {
        ccr_value = 4000;
    }
    
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, ccr_value);
}

/*============================================================================
 *                          通用PWM控制函数
 *============================================================================*/

/**
 * @brief  设置PWM通道1的CCR值
 * @note   通道1 (PA0) - 通常用于舵机控制
 * @param  ccr_value: CCR值 (0-4000)
 * @retval None
 */
void PWM_SetChannel1(uint16_t ccr_value)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr_value);
}

/**
 * @brief  设置PWM通道2的CCR值
 * @note   通道2 (PA1) - 通常用于LED亮度控制
 * @param  ccr_value: CCR值 (0-4000)
 * @retval None
 */
void PWM_SetChannel2(uint16_t ccr_value)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, ccr_value);
}