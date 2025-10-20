/**
 * @file    debug_with_config_example.cpp
 * @brief   使用配置文件的调试系统示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 本示例展示如何使用 debug_config.h 配置文件进行模块化调试
 */

#include "stm32f1xx_hal.h"
#include "debug.hpp"
#include "debug_config.h"

/* ========== 模拟的功能函数 ========== */

/**
 * @brief 电机控制示例
 */
void motor_set_speed(int motor_id, int speed)
{
    // 使用电机调试宏（仅在DEBUG_MOTOR_ENABLE=1时输出）
    DEBUG_MOTOR("设置电机%d速度: %d%%\r\n", motor_id, speed);
    
    // 输入验证
    DEBUG_ASSERT(speed >= -100 && speed <= 100);
    
    if (speed < -100 || speed > 100)
    {
        DEBUG_ERROR("电机速度超出范围: %d\r\n", speed);
        return;
    }
    
    // 实际的电机控制代码
    // ...
    
    DEBUG_MOTOR("电机%d速度设置成功\r\n", motor_id);
}

/**
 * @brief 传感器读取示例
 */
int sensor_read(int sensor_id)
{
    DEBUG_SENSOR("读取传感器%d\r\n", sensor_id);
    
    // 模拟读取传感器
    int value = sensor_id * 100 + 50;
    
    DEBUG_SENSOR("传感器%d值: %d\r\n", sensor_id, value);
    
    // 异常检测
    if (value > 3000)
    {
        DEBUG_WARN("传感器%d值异常: %d\r\n", sensor_id, value);
    }
    
    return value;
}

/**
 * @brief 蓝牙通信示例
 */
void bluetooth_send(const char* data)
{
    // 蓝牙调试（根据DEBUG_BLUETOOTH_ENABLE开关）
    DEBUG_BT("发送数据: %s\r\n", data);
    
    // 实际发送代码
    // ...
    
    DEBUG_BT("数据发送完成\r\n");
}

/**
 * @brief 巡线控制示例
 */
void line_follow_control(void)
{
    // 读取传感器
    int sensors[8];
    for (int i = 0; i < 8; i++)
    {
        sensors[i] = sensor_read(i);
    }
    
    // 计算误差
    int error = 0;  // 简化计算
    
    DEBUG_LINE("巡线误差: %d\r\n", error);
    
    // 控制电机
    int left_speed = 50 - error;
    int right_speed = 50 + error;
    
    motor_set_speed(1, left_speed);
    motor_set_speed(2, right_speed);
}

/**
 * @brief 系统初始化
 */
void system_init(void)
{
    DEBUG_SYSTEM("开始系统初始化...\r\n");
    
    // 各种初始化
    DEBUG_INFO("初始化GPIO\r\n");
    // MX_GPIO_Init();
    
    DEBUG_INFO("初始化定时器\r\n");
    // MX_TIM3_Init();
    
    DEBUG_INFO("初始化ADC\r\n");
    // MX_ADC1_Init();
    
    DEBUG_SYSTEM("系统初始化完成\r\n");
}

/**
 * @brief 带时间戳的调试示例
 */
void timestamp_debug_example(void)
{
    DEBUG_INFO("===== 时间戳调试示例 =====\r\n");
    
    for (int i = 0; i < 5; i++)
    {
        DEBUG_TIMESTAMP();
        DEBUG_INFO("循环 %d\r\n", i);
        HAL_Delay(100);
    }
}

/**
 * @brief 错误处理示例
 */
void error_handling_example(void)
{
    DEBUG_INFO("===== 错误处理示例 =====\r\n");
    
    int ret = -1;  // 模拟错误返回值
    
    if (ret != 0)
    {
        // 错误信息总是输出，不受调试开关控制
        DEBUG_ERROR("操作失败，错误代码: %d\r\n", ret);
    }
    
    float voltage = 2.8;  // 模拟电压值
    
    if (voltage < 3.0)
    {
        // 警告信息也总是输出
        DEBUG_WARN("电池电压过低: %.2fV\r\n", voltage);
    }
}

/**
 * @brief 断言使用示例
 */
void assert_example(void)
{
    DEBUG_INFO("===== 断言使用示例 =====\r\n");
    
    int speed = 80;
    
    // 断言：速度应该在合理范围内
    DEBUG_ASSERT(speed >= -100 && speed <= 100);
    
    int sensor_count = 8;
    
    // 断言：传感器数量应该正确
    DEBUG_ASSERT(sensor_count == 8);
    
    DEBUG_INFO("所有断言通过\r\n");
}

/**
 * @brief 性能测试示例
 */
void performance_test(void)
{
    DEBUG_INFO("===== 性能测试 =====\r\n");
    
    uint32_t start_time = HAL_GetTick();
    
    // 测试调试输出的性能影响
    Debug_Enable();
    for (int i = 0; i < 100; i++)
    {
        DEBUG_MOTOR("测试 %d\r\n", i);
    }
    uint32_t time_enabled = HAL_GetTick() - start_time;
    
    // 测试禁用调试时的性能
    start_time = HAL_GetTick();
    Debug_Disable();
    for (int i = 0; i < 100; i++)
    {
        DEBUG_MOTOR("测试 %d\r\n", i);
    }
    uint32_t time_disabled = HAL_GetTick() - start_time;
    
    Debug_Enable();
    DEBUG_INFO("调试启用时耗时: %lu ms\r\n", time_enabled);
    DEBUG_INFO("调试禁用时耗时: %lu ms\r\n", time_disabled);
    DEBUG_INFO("性能提升: %lu%%\r\n", 
               ((time_enabled - time_disabled) * 100) / time_enabled);
}

/**
 * @brief 主程序示例
 */
void main_example(void)
{
    /* 系统初始化 */
    HAL_Init();
    // SystemClock_Config();
    // MX_USART2_UART_Init();
    
    /* 根据配置设置默认调试状态 */
    #if DEBUG_DEFAULT_ENABLED
        Debug_Enable();
    #else
        Debug_Disable();
    #endif
    
    /* 启动横幅 */
    #if DEBUG_SHOW_STARTUP_BANNER
        Debug_Print_Always("\r\n");
        Debug_Print_Always("========================================\r\n");
        Debug_Print_Always("  调试系统配置示例\r\n");
        Debug_Print_Always("========================================\r\n");
        Debug_Print_Always("编译时间: %s %s\r\n", __DATE__, __TIME__);
        Debug_Print_Always("调试配置:\r\n");
        Debug_Print_Always("  - 电机调试  : %s\r\n", 
                          DEBUG_MOTOR_ENABLE ? "启用" : "禁用");
        Debug_Print_Always("  - 传感器调试: %s\r\n", 
                          DEBUG_SENSOR_ENABLE ? "启用" : "禁用");
        Debug_Print_Always("  - 蓝牙调试  : %s\r\n", 
                          DEBUG_BLUETOOTH_ENABLE ? "启用" : "禁用");
        Debug_Print_Always("  - 巡线调试  : %s\r\n", 
                          DEBUG_LINE_FOLLOW_ENABLE ? "启用" : "禁用");
        Debug_Print_Always("========================================\r\n");
        Debug_Print_Always("\r\n");
    #endif
    
    /* 系统初始化 */
    system_init();
    
    /* 运行各种示例 */
    
    // 1. 基本模块调试
    DEBUG_INFO("\r\n===== 模块调试示例 =====\r\n");
    motor_set_speed(1, 75);
    motor_set_speed(2, -50);
    sensor_read(3);
    bluetooth_send("Hello");
    
    // 2. 时间戳示例
    #if DEBUG_SHOW_TIMESTAMP
        timestamp_debug_example();
    #endif
    
    // 3. 错误处理示例
    error_handling_example();
    
    // 4. 断言示例
    assert_example();
    
    // 5. 性能测试
    performance_test();
    
    // 6. 巡线控制（综合示例）
    DEBUG_INFO("\r\n===== 巡线控制示例 =====\r\n");
    for (int i = 0; i < 3; i++)
    {
        DEBUG_TIMESTAMP();
        DEBUG_INFO("巡线循环 %d\r\n", i);
        line_follow_control();
        HAL_Delay(500);
    }
    
    /* 主循环 */
    DEBUG_INFO("\r\n进入主循环\r\n");
    
    #if !DEBUG_SHOW_LOOP_INFO
        // 主循环中关闭调试以提高性能
        Debug_Disable();
    #endif
    
    uint32_t count = 0;
    while (1)
    {
        count++;
        
        #if DEBUG_SHOW_LOOP_INFO
            if (count % 100 == 0)
            {
                DEBUG_INFO("主循环计数: %lu\r\n", count);
            }
        #endif
        
        // 主要功能代码
        // ...
        
        HAL_Delay(10);
    }
}

/**
 * @brief 实际项目中的最佳实践示例
 */
void best_practice_example(void)
{
    /*
     * 最佳实践：
     * 
     * 1. 开发阶段：
     *    - 在 debug_config.h 中启用所有需要的模块调试
     *    - DEBUG_GLOBAL_ENABLE = 1
     *    - DEBUG_DEFAULT_ENABLED = 1
     *    - 各模块调试开关根据需要设置
     * 
     * 2. 测试阶段：
     *    - 保持 DEBUG_GLOBAL_ENABLE = 1
     *    - 设置 DEBUG_DEFAULT_ENABLED = 0
     *    - 在需要时手动启用调试
     * 
     * 3. 发布版本：
     *    - 设置 DEBUG_GLOBAL_ENABLE = 0
     *    - 所有调试代码会被编译器优化掉，不占用空间
     *    - 保留 DEBUG_ERROR 和 DEBUG_WARN 的关键信息
     * 
     * 4. 调试技巧：
     *    - 使用模块化调试宏，便于管理
     *    - 重要的错误信息使用 DEBUG_ERROR（总是输出）
     *    - 在性能关键代码中禁用调试
     *    - 使用时间戳追踪时序问题
     *    - 使用断言捕获逻辑错误
     */
    
    // 示例：根据运行状态动态控制调试
    
    // 正常运行时关闭调试
    Debug_Disable();
    
    // 检测到异常时启用调试
    int error_count = 0;
    if (error_count > 10)
    {
        Debug_Enable();
        DEBUG_WARN("错误次数过多: %d\r\n", error_count);
        // 输出详细诊断信息
        DEBUG_SYSTEM("系统状态: ...\r\n");
    }
    
    // 进入特殊模式时启用特定模块调试
    bool calibration_mode = true;
    if (calibration_mode)
    {
        Debug_Enable();
        // 标定过程中的详细信息
        DEBUG_SENSOR("开始传感器标定...\r\n");
    }
}
