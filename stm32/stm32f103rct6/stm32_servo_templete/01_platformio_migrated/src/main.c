/*============================================================================
 *                        STM32F103 PWM 控制演示程序 (PlatformIO版本)
 *============================================================================
 * 从Keil项目迁移到PlatformIO的STM32F103 PWM控制demo
 * 
 * 功能:
 *   1. 舵机控制演示 (TIM2_CH1 - PA0)
 *   2. LED亮度控制演示 (TIM2_CH2 - PA1)  
 *   3. 展示标准50Hz PWM的两种应用场景
 * 
 * 硬件连接:
 *   - PA0: 舵机信号线 (舵机电源请使用外部5-6V)
 *   - PA1: LED正极 (通过限流电阻连接，负极接GND)
 *   - 板载RGB LED: 状态指示
 * 
 * PlatformIO迁移说明:
 *   - 使用STM32Cube HAL库替代标准外设库
 *   - 保持原有的PWM控制逻辑
 *   - 适配PlatformIO的项目结构
 *============================================================================*/

#include "main.h"
#include "pwm_control.h"
#include "led_control.h"

/* 私有变量 */
TIM_HandleTypeDef htim2;

/* 私有函数声明 */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
void Error_Handler(void);

/* 延时函数 */
void Delay_Ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void Delay_Us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}

/**
 * @brief  主函数
 * @retval int
 */
int main(void)
{
    /* HAL库初始化 */
    HAL_Init();

    /* 配置系统时钟 */
    SystemClock_Config();

    /* 初始化GPIO */
    MX_GPIO_Init();

    /* 初始化定时器2 */
    MX_TIM2_Init();

    /* 启动PWM输出 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  // 舵机通道
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // LED通道

    /* 初始化LED控制 */
    LED_Init();

    /* 显示启动状态 */
    LED_SetColor(LED_WHITE);
    HAL_Delay(1000);

    /* 主循环 - 平滑舵机扫描演示 */
    while (1)
    {
        /* 舵机从0°扫描到180° */
        for(int angle = 0; angle <= 180; angle++)
        {
            Servo_SetAngle(angle);
            HAL_Delay(20);  // 20ms延时，形成平滑运动
        }

        /* 在180°位置停留 */
        HAL_Delay(1000);

        /* 舵机从180°扫描回0° */
        for(int angle = 180; angle >= 0; angle--)
        {
            Servo_SetAngle(angle);
            HAL_Delay(20);
        }

        /* 在0°位置停留 */
        HAL_Delay(1000);
    }
}

/**
 * @brief  系统时钟配置
 * @note   配置为72MHz主频 (STM32F1系列)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** 初始化RCC振荡器 */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** 初始化CPU、AHB和APB总线时钟 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  TIM2初始化函数
 * @note   配置为50Hz PWM输出，用于舵机和LED控制
 */
static void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 359;           // 预分频：72MHz/(359+1) = 200kHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 3999;             // 周期：(3999+1)/200kHz = 20ms = 50Hz
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* 配置PWM通道1 (舵机控制) */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 300;                // 初始脉宽：1.5ms (中位)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    /* 配置PWM通道2 (LED控制) */
    sConfigOC.Pulse = 2000;               // 初始亮度：50%
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }

    /* GPIO配置 - 手动调用MSP初始化 */
    HAL_TIM_PWM_MspInit(&htim2);
}

/**
 * @brief  GPIO初始化函数
 */
static void MX_GPIO_Init(void)
{
    /* GPIO端口时钟使能 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
 * @brief  错误处理函数
 */
void Error_Handler(void)
{
    /* 用户可以在这里添加自己的错误处理代码 */
    __disable_irq();
    while (1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  断言失败时调用的函数
 * @param  file: 源文件名指针
 * @param  line: 断言失败的行号
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* 用户可以在这里添加自己的断言失败处理代码 */
}
#endif /* USE_FULL_ASSERT */