/*============================================================================
 *                        PWM驱动实现 - CMake从零开始项目
 *============================================================================*/

#include "pwm_driver.h"

/* 私有变量 */
static PWM_Driver_t pwm_driver = {0};

/**
 * @brief  PWM驱动初始化
 * @retval bool 初始化结果
 */
bool PWM_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 检查是否已经初始化 */
    if (pwm_driver.initialized) {
        return true;
    }
    
    /* 使能时钟 */
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* 配置GPIO引脚 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* 配置定时器基本参数 */
    pwm_driver.htim.Instance = PWM_TIMER_INSTANCE;
    pwm_driver.htim.Init.Prescaler = PWM_PRESCALER;         // 72MHz/(359+1) = 200kHz
    pwm_driver.htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    pwm_driver.htim.Init.Period = PWM_PERIOD - 1;           // (4000)/200kHz = 20ms = 50Hz
    pwm_driver.htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    pwm_driver.htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    if (HAL_TIM_Base_Init(&pwm_driver.htim) != HAL_OK) {
        return false;
    }
    
    /* 配置时钟源 */
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&pwm_driver.htim, &sClockSourceConfig) != HAL_OK) {
        return false;
    }
    
    /* 初始化PWM模式 */
    if (HAL_TIM_PWM_Init(&pwm_driver.htim) != HAL_OK) {
        return false;
    }
    
    /* 配置主定时器 */
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&pwm_driver.htim, &sMasterConfig) != HAL_OK) {
        return false;
    }
    
    /* 配置PWM输出比较 */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    /* 配置通道1 (舵机控制) */
    sConfigOC.Pulse = SERVO_MID_PULSE;  // 初始位置：中位
    if (HAL_TIM_PWM_ConfigChannel(&pwm_driver.htim, &sConfigOC, PWM_TIMER_CHANNEL_1) != HAL_OK) {
        return false;
    }
    
    /* 配置通道2 (LED控制) */
    sConfigOC.Pulse = 0;  // 初始亮度：关闭
    if (HAL_TIM_PWM_ConfigChannel(&pwm_driver.htim, &sConfigOC, PWM_TIMER_CHANNEL_2) != HAL_OK) {
        return false;
    }
    
    /* 启动PWM输出 */
    if (HAL_TIM_PWM_Start(&pwm_driver.htim, PWM_TIMER_CHANNEL_1) != HAL_OK) {
        return false;
    }
    
    if (HAL_TIM_PWM_Start(&pwm_driver.htim, PWM_TIMER_CHANNEL_2) != HAL_OK) {
        return false;
    }
    
    /* 设置初始值 */
    pwm_driver.servo_position = SERVO_MID_PULSE;
    pwm_driver.led_brightness = 0;
    pwm_driver.initialized = true;
    
    return true;
}

/**
 * @brief  PWM驱动反初始化
 * @retval bool 反初始化结果
 */
bool PWM_DeInit(void)
{
    if (!pwm_driver.initialized) {
        return true;
    }
    
    /* 停止PWM输出 */
    HAL_TIM_PWM_Stop(&pwm_driver.htim, PWM_TIMER_CHANNEL_1);
    HAL_TIM_PWM_Stop(&pwm_driver.htim, PWM_TIMER_CHANNEL_2);
    
    /* 反初始化定时器 */
    HAL_TIM_Base_DeInit(&pwm_driver.htim);
    
    /* 重置GPIO */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1);
    
    /* 禁用时钟 */
    __HAL_RCC_TIM2_CLK_DISABLE();
    
    pwm_driver.initialized = false;
    
    return true;
}

/**
 * @brief  设置舵机角度
 * @param  angle 角度值 (0-180°)
 * @retval bool 设置结果
 */
bool PWM_SetServoAngle(uint8_t angle)
{
    if (!pwm_driver.initialized) {
        return false;
    }
    
    uint16_t pulse = PWM_AngleToPulse(angle);
    return PWM_SetServoPosition(pulse);
}

/**
 * @brief  设置舵机脉宽
 * @param  pulse 脉宽值
 * @retval bool 设置结果
 */
bool PWM_SetServoPosition(uint16_t pulse)
{
    if (!pwm_driver.initialized) {
        return false;
    }
    
    /* 限制脉宽范围 */
    if (pulse < SERVO_MIN_PULSE) pulse = SERVO_MIN_PULSE;
    if (pulse > SERVO_MAX_PULSE) pulse = SERVO_MAX_PULSE;
    
    __HAL_TIM_SET_COMPARE(&pwm_driver.htim, PWM_TIMER_CHANNEL_1, pulse);
    pwm_driver.servo_position = pulse;
    
    return true;
}

/**
 * @brief  角度转脉宽
 * @param  angle 角度值 (0-180°)
 * @retval uint16_t 对应的脉宽值
 */
uint16_t PWM_AngleToPulse(uint8_t angle)
{
    if (angle > 180) angle = 180;
    
    /* 线性插值：0° → 200, 180° → 400 */
    return SERVO_MIN_PULSE + ((uint32_t)angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 180;
}

/**
 * @brief  设置LED亮度
 * @param  brightness 亮度百分比 (0-100%)
 * @retval bool 设置结果
 */
bool PWM_SetLEDBrightness(uint8_t brightness)
{
    if (!pwm_driver.initialized) {
        return false;
    }
    
    if (brightness > 100) brightness = 100;
    
    /* 计算对应的脉宽值 */
    uint16_t pulse = ((uint32_t)brightness * PWM_PERIOD) / 100;
    
    return PWM_SetLEDPulse(pulse);
}

/**
 * @brief  设置LED脉宽
 * @param  pulse 脉宽值
 * @retval bool 设置结果
 */
bool PWM_SetLEDPulse(uint16_t pulse)
{
    if (!pwm_driver.initialized) {
        return false;
    }
    
    /* 限制脉宽范围 */
    if (pulse > PWM_PERIOD) pulse = PWM_PERIOD;
    
    __HAL_TIM_SET_COMPARE(&pwm_driver.htim, PWM_TIMER_CHANNEL_2, pulse);
    pwm_driver.led_brightness = pulse;
    
    return true;
}

/**
 * @brief  设置指定通道的脉宽
 * @param  channel 通道号
 * @param  pulse 脉宽值
 * @retval bool 设置结果
 */
bool PWM_SetChannelPulse(uint32_t channel, uint16_t pulse)
{
    if (!pwm_driver.initialized) {
        return false;
    }
    
    __HAL_TIM_SET_COMPARE(&pwm_driver.htim, channel, pulse);
    return true;
}

/**
 * @brief  获取指定通道的脉宽
 * @param  channel 通道号
 * @retval uint16_t 当前脉宽值
 */
uint16_t PWM_GetChannelPulse(uint32_t channel)
{
    if (!pwm_driver.initialized) {
        return 0;
    }
    
    return __HAL_TIM_GET_COMPARE(&pwm_driver.htim, channel);
}

/**
 * @brief  获取PWM驱动结构体指针
 * @retval PWM_Driver_t* 驱动结构体指针
 */
PWM_Driver_t* PWM_GetDriver(void)
{
    return &pwm_driver;
}