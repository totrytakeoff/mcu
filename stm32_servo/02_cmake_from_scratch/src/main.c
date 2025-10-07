/*============================================================================
 *                        主程序 - CMake从零开始项目
 *============================================================================
 * 这是一个从零开始构建的STM32F103项目示例
 * 展示如何使用CMake构建系统和手动配置所有必要组件
 * 
 * 功能:
 *   - 系统时钟配置 (72MHz)
 *   - GPIO控制 (RGB LED)
 *   - PWM输出 (舵机和LED控制)
 *   - 模块化的驱动程序设计
 * 
 * 硬件连接:
 *   - PA0: TIM2_CH1 - 舵机控制
 *   - PA1: TIM2_CH2 - LED PWM控制
 *   - PB0/PB1/PB5: RGB LED
 *============================================================================*/

#include "main.h"
#include "clock_config.h"
#include "gpio_driver.h"
#include "pwm_driver.h"

/* 私有变量 */
static uint32_t system_tick = 0;

/* 私有函数声明 */
static void System_Init(void);
static void Demo_ServoSweep(void);
static void Demo_LEDFade(void);
static void Demo_Combined(void);

/**
 * @brief  主函数
 * @retval int 程序退出码
 */
int main(void)
{
    /* 系统初始化 */
    System_Init();
    
    /* 显示启动状态 */
    GPIO_RGB_SetColor(LED_COLOR_WHITE);
    HAL_Delay(1000);
    GPIO_RGB_SetColor(LED_COLOR_OFF);
    
    /* 主循环 */
    while (1)
    {
        /* 演示1: 舵机扫描 */
        GPIO_RGB_SetColor(LED_COLOR_RED);
        Demo_ServoSweep();
        HAL_Delay(1000);
        
        /* 演示2: LED渐变 */
        GPIO_RGB_SetColor(LED_COLOR_GREEN);
        Demo_LEDFade();
        HAL_Delay(1000);
        
        /* 演示3: 组合控制 */
        GPIO_RGB_SetColor(LED_COLOR_BLUE);
        Demo_Combined();
        HAL_Delay(1000);
        
        GPIO_RGB_SetColor(LED_COLOR_OFF);
        HAL_Delay(500);
    }
}

/**
 * @brief  系统初始化
 * @note   初始化所有必要的硬件模块
 */
static void System_Init(void)
{
    /* HAL库初始化 */
    HAL_Init();
    
    /* 配置系统时钟为72MHz */
    if (!Clock_Config()) {
        Error_Handler();
    }
    
    /* 初始化GPIO */
    if (!GPIO_Init()) {
        Error_Handler();
    }
    
    /* 初始化PWM */
    if (!PWM_Init()) {
        Error_Handler();
    }
}

/**
 * @brief  舵机扫描演示
 * @note   舵机在0°到180°之间平滑扫描
 */
static void Demo_ServoSweep(void)
{
    /* 从0°扫描到180° */
    for (int angle = 0; angle <= 180; angle += 5) {
        PWM_SetServoAngle(angle);
        HAL_Delay(50);
    }
    
    HAL_Delay(500);
    
    /* 从180°扫描回0° */
    for (int angle = 180; angle >= 0; angle -= 5) {
        PWM_SetServoAngle(angle);
        HAL_Delay(50);
    }
}

/**
 * @brief  LED渐变演示
 * @note   LED亮度在0%到100%之间渐变
 */
static void Demo_LEDFade(void)
{
    /* 亮度递增 */
    for (int brightness = 0; brightness <= 100; brightness += 5) {
        PWM_SetLEDBrightness(brightness);
        HAL_Delay(50);
    }
    
    HAL_Delay(500);
    
    /* 亮度递减 */
    for (int brightness = 100; brightness >= 0; brightness -= 5) {
        PWM_SetLEDBrightness(brightness);
        HAL_Delay(50);
    }
}

/**
 * @brief  组合控制演示
 * @note   同时控制舵机角度和LED亮度
 */
static void Demo_Combined(void)
{
    typedef struct {
        uint8_t angle;
        uint8_t brightness;
    } demo_step_t;
    
    demo_step_t steps[] = {
        {0,   0},
        {45,  25},
        {90,  50},
        {135, 75},
        {180, 100},
        {90,  50}
    };
    
    size_t num_steps = sizeof(steps) / sizeof(steps[0]);
    
    for (size_t i = 0; i < num_steps; i++) {
        PWM_SetServoAngle(steps[i].angle);
        PWM_SetLEDBrightness(steps[i].brightness);
        HAL_Delay(800);
    }
}

/**
 * @brief  系统时钟配置为72MHz
 * @note   使用外部8MHz晶振，通过PLL倍频到72MHz
 */
void SystemClock_Config(void)
{
    if (!Clock_Config()) {
        Error_Handler();
    }
}

/**
 * @brief  错误处理函数
 * @note   系统发生错误时调用，进入死循环并闪烁红色LED
 */
void Error_Handler(void)
{
    /* 禁用中断 */
    __disable_irq();
    
    /* 错误指示：快速闪烁红色LED */
    while (1) {
        GPIO_RGB_SetColor(LED_COLOR_RED);
        for (volatile int i = 0; i < 100000; i++);
        GPIO_RGB_SetColor(LED_COLOR_OFF);
        for (volatile int i = 0; i < 100000; i++);
    }
}

/**
 * @brief  SysTick中断回调函数
 * @note   每1ms调用一次，用于系统计时
 */
void HAL_IncTick(void)
{
    system_tick++;
}

/**
 * @brief  获取系统滴答计数
 * @retval uint32_t 当前系统滴答数
 */
uint32_t HAL_GetTick(void)
{
    return system_tick;
}

/**
 * @brief  断言失败处理函数
 * @param  file 源文件名
 * @param  line 行号
 */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* 用户可以在这里添加自己的断言失败处理代码 */
    DBG_PRINTF("Assert failed: file %s on line %d\r\n", file, line);
    Error_Handler();
}
#endif /* USE_FULL_ASSERT */