/*============================================================================
 *                        LED控制实现文件 - PlatformIO版本
 *============================================================================*/

#include "led_control.h"

/**
 * @brief  LED初始化函数
 * @note   初始化RGB LED的GPIO引脚
 * @param  None
 * @retval None
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能GPIOB时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 配置LED引脚 */
    HAL_GPIO_WritePin(LED_R_GPIO_PORT, LED_R_PIN, GPIO_PIN_SET);  // 默认关闭
    HAL_GPIO_WritePin(LED_G_GPIO_PORT, LED_G_PIN, GPIO_PIN_SET);  // 默认关闭
    HAL_GPIO_WritePin(LED_B_GPIO_PORT, LED_B_PIN, GPIO_PIN_SET);  // 默认关闭

    GPIO_InitStruct.Pin = LED_R_PIN | LED_G_PIN | LED_B_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief  设置LED颜色
 * @param  color: LED颜色
 * @retval None
 */
void LED_SetColor(LED_Color_t color)
{
    /* 先关闭所有LED */
    LED_R_Off();
    LED_G_Off();
    LED_B_Off();

    /* 根据颜色设置对应的LED */
    switch(color) {
        case LED_RED:
            LED_R_On();
            break;
        case LED_GREEN:
            LED_G_On();
            break;
        case LED_BLUE:
            LED_B_On();
            break;
        case LED_YELLOW:
            LED_R_On();
            LED_G_On();
            break;
        case LED_PURPLE:
            LED_R_On();
            LED_B_On();
            break;
        case LED_CYAN:
            LED_G_On();
            LED_B_On();
            break;
        case LED_WHITE:
            LED_R_On();
            LED_G_On();
            LED_B_On();
            break;
        case LED_OFF:
        default:
            /* 已经关闭所有LED */
            break;
    }
}

/**
 * @brief  红色LED开启
 */
void LED_R_On(void)
{
    HAL_GPIO_WritePin(LED_R_GPIO_PORT, LED_R_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  红色LED关闭
 */
void LED_R_Off(void)
{
    HAL_GPIO_WritePin(LED_R_GPIO_PORT, LED_R_PIN, GPIO_PIN_SET);
}

/**
 * @brief  绿色LED开启
 */
void LED_G_On(void)
{
    HAL_GPIO_WritePin(LED_G_GPIO_PORT, LED_G_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  绿色LED关闭
 */
void LED_G_Off(void)
{
    HAL_GPIO_WritePin(LED_G_GPIO_PORT, LED_G_PIN, GPIO_PIN_SET);
}

/**
 * @brief  蓝色LED开启
 */
void LED_B_On(void)
{
    HAL_GPIO_WritePin(LED_B_GPIO_PORT, LED_B_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  蓝色LED关闭
 */
void LED_B_Off(void)
{
    HAL_GPIO_WritePin(LED_B_GPIO_PORT, LED_B_PIN, GPIO_PIN_SET);
}