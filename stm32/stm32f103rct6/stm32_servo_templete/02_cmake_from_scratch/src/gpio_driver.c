/*============================================================================
 *                        GPIO驱动实现 - CMake从零开始项目
 *============================================================================*/

#include "gpio_driver.h"

/* 私有变量 */
static bool gpio_initialized = false;

/**
 * @brief  GPIO驱动初始化
 * @retval bool 初始化结果
 */
bool GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (gpio_initialized) {
        return true;
    }
    
    /* 使能GPIO时钟 */
    LED_R_CLK_ENABLE();
    LED_G_CLK_ENABLE();
    LED_B_CLK_ENABLE();
    KEY_CLK_ENABLE();
    
    /* 配置LED引脚为推挽输出 */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    /* 初始化红色LED */
    GPIO_InitStruct.Pin = LED_R_PIN;
    HAL_GPIO_Init(LED_R_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, GPIO_PIN_SET);  // 默认关闭
    
    /* 初始化绿色LED */
    GPIO_InitStruct.Pin = LED_G_PIN;
    HAL_GPIO_Init(LED_G_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN, GPIO_PIN_SET);  // 默认关闭
    
    /* 初始化蓝色LED */
    GPIO_InitStruct.Pin = LED_B_PIN;
    HAL_GPIO_Init(LED_B_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_B_PORT, LED_B_PIN, GPIO_PIN_SET);  // 默认关闭
    
    /* 配置按键引脚为上拉输入 */
    GPIO_InitStruct.Pin = KEY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY_PORT, &GPIO_InitStruct);
    
    gpio_initialized = true;
    return true;
}

/**
 * @brief  GPIO驱动反初始化
 */
void GPIO_DeInit(void)
{
    if (!gpio_initialized) {
        return;
    }
    
    /* 反初始化GPIO引脚 */
    HAL_GPIO_DeInit(LED_R_PORT, LED_R_PIN);
    HAL_GPIO_DeInit(LED_G_PORT, LED_G_PIN);
    HAL_GPIO_DeInit(LED_B_PORT, LED_B_PIN);
    HAL_GPIO_DeInit(KEY_PORT, KEY_PIN);
    
    gpio_initialized = false;
}

/**
 * @brief  点亮LED
 * @param  port GPIO端口
 * @param  pin GPIO引脚
 */
void GPIO_LED_On(GPIO_TypeDef* port, uint16_t pin)
{
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);  // 低电平点亮
}

/**
 * @brief  熄灭LED
 * @param  port GPIO端口
 * @param  pin GPIO引脚
 */
void GPIO_LED_Off(GPIO_TypeDef* port, uint16_t pin)
{
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);    // 高电平熄灭
}

/**
 * @brief  切换LED状态
 * @param  port GPIO端口
 * @param  pin GPIO引脚
 */
void GPIO_LED_Toggle(GPIO_TypeDef* port, uint16_t pin)
{
    HAL_GPIO_TogglePin(port, pin);
}

/**
 * @brief  设置RGB LED颜色
 * @param  color LED颜色
 */
void GPIO_RGB_SetColor(LED_Color_t color)
{
    /* 先关闭所有LED */
    GPIO_RGB_Off();
    
    /* 根据颜色点亮对应LED */
    switch (color) {
        case LED_COLOR_RED:
            GPIO_LED_Red_On();
            break;
            
        case LED_COLOR_GREEN:
            GPIO_LED_Green_On();
            break;
            
        case LED_COLOR_BLUE:
            GPIO_LED_Blue_On();
            break;
            
        case LED_COLOR_YELLOW:
            GPIO_LED_Red_On();
            GPIO_LED_Green_On();
            break;
            
        case LED_COLOR_PURPLE:
            GPIO_LED_Red_On();
            GPIO_LED_Blue_On();
            break;
            
        case LED_COLOR_CYAN:
            GPIO_LED_Green_On();
            GPIO_LED_Blue_On();
            break;
            
        case LED_COLOR_WHITE:
            GPIO_LED_Red_On();
            GPIO_LED_Green_On();
            GPIO_LED_Blue_On();
            break;
            
        case LED_COLOR_OFF:
        default:
            /* 已经关闭所有LED */
            break;
    }
}

/**
 * @brief  关闭RGB LED
 */
void GPIO_RGB_Off(void)
{
    GPIO_LED_Red_Off();
    GPIO_LED_Green_Off();
    GPIO_LED_Blue_Off();
}

/**
 * @brief  点亮红色LED
 */
void GPIO_LED_Red_On(void)
{
    GPIO_LED_On(LED_R_PORT, LED_R_PIN);
}

/**
 * @brief  熄灭红色LED
 */
void GPIO_LED_Red_Off(void)
{
    GPIO_LED_Off(LED_R_PORT, LED_R_PIN);
}

/**
 * @brief  点亮绿色LED
 */
void GPIO_LED_Green_On(void)
{
    GPIO_LED_On(LED_G_PORT, LED_G_PIN);
}

/**
 * @brief  熄灭绿色LED
 */
void GPIO_LED_Green_Off(void)
{
    GPIO_LED_Off(LED_G_PORT, LED_G_PIN);
}

/**
 * @brief  点亮蓝色LED
 */
void GPIO_LED_Blue_On(void)
{
    GPIO_LED_On(LED_B_PORT, LED_B_PIN);
}

/**
 * @brief  熄灭蓝色LED
 */
void GPIO_LED_Blue_Off(void)
{
    GPIO_LED_Off(LED_B_PORT, LED_B_PIN);
}

/**
 * @brief  检测按键是否按下
 * @retval bool 按键状态 (true=按下, false=释放)
 */
bool GPIO_KEY_IsPressed(void)
{
    return (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_RESET);
}

/**
 * @brief  等待按键按下
 * @param  timeout_ms 超时时间(毫秒)
 * @retval bool 是否检测到按键按下
 */
bool GPIO_KEY_WaitPress(uint32_t timeout_ms)
{
    uint32_t start_tick = HAL_GetTick();
    
    while ((HAL_GetTick() - start_tick) < timeout_ms) {
        if (GPIO_KEY_IsPressed()) {
            /* 简单的防抖处理 */
            HAL_Delay(50);
            if (GPIO_KEY_IsPressed()) {
                /* 等待按键释放 */
                while (GPIO_KEY_IsPressed()) {
                    HAL_Delay(10);
                }
                return true;
            }
        }
        HAL_Delay(10);
    }
    
    return false;
}

/**
 * @brief  写GPIO引脚
 * @param  port GPIO端口
 * @param  pin GPIO引脚
 * @param  state 引脚状态
 */
void GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state)
{
    HAL_GPIO_WritePin(port, pin, state);
}

/**
 * @brief  读GPIO引脚
 * @param  port GPIO端口
 * @param  pin GPIO引脚
 * @retval GPIO_PinState 引脚状态
 */
GPIO_PinState GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin)
{
    return HAL_GPIO_ReadPin(port, pin);
}