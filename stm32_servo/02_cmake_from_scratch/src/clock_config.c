/*============================================================================
 *                        时钟配置实现 - CMake从零开始项目
 *============================================================================*/

#include "clock_config.h"

/* 私有变量 */
static Clock_Status_t clock_status = {0};

/**
 * @brief  系统时钟配置
 * @note   配置系统时钟为72MHz
 * @retval bool 配置结果
 */
bool Clock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* 配置主内部调节器输出电压 */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    /* 配置振荡器 */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = HSE_PREDIV;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = PLL_SOURCE;
    RCC_OscInitStruct.PLL.PLLMUL = PLL_MUL;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        clock_status.hse_ready = false;
        clock_status.pll_ready = false;
        return false;
    }
    
    clock_status.hse_ready = true;
    clock_status.pll_ready = true;
    
    /* 配置CPU、AHB和APB总线时钟 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = AHB_PRESCALER;
    RCC_ClkInitStruct.APB1CLKDivider = APB1_PRESCALER;
    RCC_ClkInitStruct.APB2CLKDivider = APB2_PRESCALER;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        return false;
    }
    
    /* 更新时钟状态 */
    Clock_UpdateStatus();
    
    return true;
}

/**
 * @brief  HSE时钟配置
 * @retval bool 配置结果
 */
bool Clock_HSE_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = HSE_PREDIV;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        clock_status.hse_ready = false;
        return false;
    }
    
    clock_status.hse_ready = true;
    return true;
}

/**
 * @brief  PLL时钟配置
 * @retval bool 配置结果
 */
bool Clock_PLL_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = PLL_SOURCE;
    RCC_OscInitStruct.PLL.PLLMUL = PLL_MUL;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        clock_status.pll_ready = false;
        return false;
    }
    
    clock_status.pll_ready = true;
    return true;
}

/**
 * @brief  系统时钟切换配置
 * @retval bool 配置结果
 */
bool Clock_SystemClockConfig(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        return false;
    }
    
    Clock_UpdateStatus();
    return true;
}

/**
 * @brief  更新时钟状态信息
 */
void Clock_UpdateStatus(void)
{
    clock_status.sysclk_freq = HAL_RCC_GetSysClockFreq();
    clock_status.hclk_freq = HAL_RCC_GetHCLKFreq();
    clock_status.pclk1_freq = HAL_RCC_GetPCLK1Freq();
    clock_status.pclk2_freq = HAL_RCC_GetPCLK2Freq();
    
    /* 定时器时钟频率计算 */
    if (clock_status.pclk1_freq == clock_status.hclk_freq) {
        clock_status.tim_clk_freq = clock_status.pclk1_freq;
    } else {
        clock_status.tim_clk_freq = clock_status.pclk1_freq * 2;
    }
}

/**
 * @brief  获取时钟状态
 * @retval Clock_Status_t 时钟状态结构体
 */
Clock_Status_t Clock_GetStatus(void)
{
    Clock_UpdateStatus();
    return clock_status;
}

/**
 * @brief  获取系统时钟频率
 * @retval uint32_t 系统时钟频率
 */
uint32_t Clock_GetSysClockFreq(void)
{
    return HAL_RCC_GetSysClockFreq();
}

/**
 * @brief  获取HCLK频率
 * @retval uint32_t HCLK频率
 */
uint32_t Clock_GetHCLKFreq(void)
{
    return HAL_RCC_GetHCLKFreq();
}

/**
 * @brief  获取PCLK1频率
 * @retval uint32_t PCLK1频率
 */
uint32_t Clock_GetPCLK1Freq(void)
{
    return HAL_RCC_GetPCLK1Freq();
}

/**
 * @brief  获取PCLK2频率
 * @retval uint32_t PCLK2频率
 */
uint32_t Clock_GetPCLK2Freq(void)
{
    return HAL_RCC_GetPCLK2Freq();
}

/**
 * @brief  配置MCO输出用于时钟调试
 * @param  mco_source MCO时钟源
 */
void Clock_MCO_Config(uint32_t mco_source)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能GPIOA时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* 配置PA8为MCO输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* 配置MCO */
    HAL_RCC_MCOConfig(RCC_MCO1, mco_source, RCC_MCODIV_1);
}