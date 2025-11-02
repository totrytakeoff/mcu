/**
 * @file    adc.c
 * @brief   ADC 配置实现 - 8路灰度传感器采样
 * @author  AI Assistant
 * @date    2024
 */

#include "adc.h"
#include "gpio.h"

/* ADC 句柄 */
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/**
 * @brief  初始化 ADC1（8通道，DMA模式）
 */
void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    /* ========== ADC1 基本配置 ========== */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;           // 扫描模式（多通道）
    hadc1.Init.ContinuousConvMode = DISABLE;             // 单次转换（由DMA触发）
    hadc1.Init.DiscontinuousConvMode = DISABLE;          // 不使用间断模式
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;    // 软件触发
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;          // 右对齐（0-4095）
    hadc1.Init.NbrOfConversion = 8;                      // 8个通道
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }
    
    /* ========== 配置 8 个通道（按DMA扫描顺序） ========== */
    
    // 通道1: ADC_CH8 (PB0) - SIG1 (最左侧传感器)
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;    // 采样时间约4us
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 通道2: ADC_CH9 (PB1) - SIG2 (左2)
    sConfig.Channel = ADC_CHANNEL_9;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 通道3: ADC_CH10 (PC0) - SIG3 (左3)
    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 通道4: ADC_CH11 (PC1) - SIG4 (中左)
    sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank = ADC_REGULAR_RANK_4;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 通道5: ADC_CH12 (PC2) - SIG5 (中右)
    sConfig.Channel = ADC_CHANNEL_12;
    sConfig.Rank = ADC_REGULAR_RANK_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 通道6: ADC_CH13 (PC3) - SIG6 (右3)
    sConfig.Channel = ADC_CHANNEL_13;
    sConfig.Rank = ADC_REGULAR_RANK_6;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 通道7: ADC_CH14 (PC4) - SIG7 (右2)
    sConfig.Channel = ADC_CHANNEL_14;
    sConfig.Rank = ADC_REGULAR_RANK_7;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 通道8: ADC_CH15 (PC5) - SIG8 (最右侧传感器)
    sConfig.Channel = ADC_CHANNEL_15;
    sConfig.Rank = ADC_REGULAR_RANK_8;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/**
 * @brief  ADC MSP 初始化（GPIO + DMA + 时钟）
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (adcHandle->Instance == ADC1)
    {
        /* ========== 1. 使能时钟 ========== */
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();
        
        /* ========== 2. 配置 GPIO 为模拟输入 ========== */
        // PB0, PB1
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        // PC0, PC1, PC2, PC3, PC4, PC5
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | 
                              GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        
        /* ========== 3. 配置 DMA ========== */
        hdma_adc1.Instance = DMA1_Channel1;
        hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;       // 外设到内存
        hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;           // 外设地址不增
        hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;               // 内存地址递增
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  // 16位
        hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;     // 16位
        hdma_adc1.Init.Mode = DMA_CIRCULAR;                    // 循环模式
        hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
        
        if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
            Error_Handler();
        }
        
        __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);
    }
}

/**
 * @brief  ADC MSP 反初始化
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
    if (adcHandle->Instance == ADC1)
    {
        __HAL_RCC_ADC1_CLK_DISABLE();
        
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_1);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | 
                               GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
        
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
}

/**
 * @brief  读取单个通道的 ADC 值（阻塞方式）
 * @param  channel: ADC 通道号（如 ADC_CHANNEL_8）
 * @retval ADC 值（0-4095）
 */
uint16_t ADC_ReadChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    // 配置通道
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    // 启动转换
    HAL_ADC_Start(&hadc1);
    
    // 等待转换完成
    HAL_ADC_PollForConversion(&hadc1, 100);
    
    // 读取结果
    uint16_t value = HAL_ADC_GetValue(&hadc1);
    
    HAL_ADC_Stop(&hadc1);
    
    return value;
}

/**
 * @brief  读取所有 8 路传感器值（DMA方式，阻塞）
 * @param  buffer: 存储结果的数组，至少8个元素
 * @note   buffer[0]=最左侧, buffer[7]=最右侧
 */
void ADC_ReadAll(uint16_t *buffer)
{
    // 启动 DMA 转换
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer, 8);

    // 等待转换完成（使用忙等待，避免1ms阻塞延迟）
    // 8通道DMA转换实际只需要约64us，使用微秒级等待
    volatile uint32_t timeout = 1000;  // 1000次循环，约100us
    while (timeout-- && !__HAL_ADC_GET_FLAG(&hadc1, ADC_FLAG_EOC)) {
        // 忙等待转换完成标志
    }

    // 停止 DMA
    HAL_ADC_Stop_DMA(&hadc1);
}

/**
 * @brief  开始 DMA 连续转换（非阻塞）
 * @param  buffer: 存储结果的数组
 * @param  length: 数组长度（应为8）
 */
void ADC_StartDMA(uint16_t *buffer, uint32_t length)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer, length);
}
