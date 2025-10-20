/**
 * @file    line_sensor_filter_config_example.cpp
 * @brief   灰度传感器滤波器配置示例
 * @author  AI Assistant
 * @date    2024
 * 
 * 本示例展示如何使用LineSensor类的滤波器控制接口
 */

#include "stm32f1xx_hal.h"
#include "line_sensor.hpp"
#include "debug.hpp"

/* ========== 示例1: 基本使用（使用默认参数） ========== */
void example1_default_filter() {
    LineSensor sensor;
    uint16_t data[8];
    
    // 默认使用 α = 0.4 的滤波系数
    // 无需额外配置，直接使用
    
    while (1) {
        sensor.getData(data);  // 自动应用滤波
        
        Debug_Printf("传感器值: %d, %d, %d, %d, %d, %d, %d, %d\r\n",
                     data[0], data[1], data[2], data[3],
                     data[4], data[5], data[6], data[7]);
        
        HAL_Delay(100);
    }
}

/* ========== 示例2: 设置滤波系数（浮点数方式） ========== */
void example2_set_alpha_float() {
    LineSensor sensor;
    uint16_t data[8];
    
    // 设置滤波系数为 0.3（更平滑）
    sensor.setFilterAlpha(0.3f);
    // 输出: [LineSensor] 滤波系数已设置: α=0.30 (77/256)
    
    // 使用滤波器
    sensor.getData(data);
    
    // 可以随时修改滤波系数
    sensor.setFilterAlpha(0.5f);  // 改为中等滤波
    sensor.getData(data);
    
    sensor.setFilterAlpha(0.7f);  // 改为弱滤波
    sensor.getData(data);
}

/* ========== 示例3: 设置滤波系数（整数方式） ========== */
void example3_set_alpha_raw() {
    LineSensor sensor;
    uint16_t data[8];
    
    // 直接设置整数值（避免浮点运算）
    sensor.setFilterAlphaRaw(77);   // α = 0.3
    // sensor.setFilterAlphaRaw(102);  // α = 0.4
    // sensor.setFilterAlphaRaw(128);  // α = 0.5
    // sensor.setFilterAlphaRaw(179);  // α = 0.7
    
    sensor.getData(data);
}

/* ========== 示例4: 查询当前滤波系数 ========== */
void example4_get_alpha() {
    LineSensor sensor;
    
    // 获取当前滤波系数
    float current_alpha = sensor.getFilterAlpha();
    Debug_Printf("当前滤波系数: α=%.2f\r\n", current_alpha);
    // 输出: 当前滤波系数: α=0.40 (默认值)
    
    // 修改后再查询
    sensor.setFilterAlpha(0.3f);
    current_alpha = sensor.getFilterAlpha();
    Debug_Printf("新的滤波系数: α=%.2f\r\n", current_alpha);
    // 输出: 新的滤波系数: α=0.30
}

/* ========== 示例5: 重置滤波器 ========== */
void example5_reset_filter() {
    LineSensor sensor;
    uint16_t data[8];
    
    // 使用一段时间后
    for (int i = 0; i < 10; i++) {
        sensor.getData(data);
        HAL_Delay(10);
    }
    
    // 检查是否已初始化
    if (sensor.isFilterInitialized()) {
        Debug_Printf("滤波器已初始化\r\n");
    }
    
    // 重置滤波器（清除历史数据）
    sensor.resetFilter();
    // 输出: [LineSensor] 滤波器已重置
    
    // 下次调用getData时会重新初始化
    sensor.getData(data);
}

/* ========== 示例6: 根据速度自动调整滤波 ========== */
void example6_speed_adaptive() {
    LineSensor sensor;
    uint16_t data[8];
    
    float speed = 0.2f;  // 当前速度：0.2 m/s
    
    // 根据速度自动调整滤波系数
    sensor.setFilterBySpeed(speed);
    // 输出: [LineSensor] 低速模式: α=0.3
    
    sensor.getData(data);
    
    // 加速到中速
    speed = 0.5f;
    sensor.setFilterBySpeed(speed);
    // 输出: [LineSensor] 中速模式: α=0.4
    
    // 加速到高速
    speed = 0.8f;
    sensor.setFilterBySpeed(speed);
    // 输出: [LineSensor] 高速模式: α=0.7
}

/* ========== 示例7: 实际应用 - 自适应巡线 ========== */
void example7_real_world_adaptive() {
    LineSensor sensor;
    uint16_t data[8];
    
    // 初始设置：低速启动
    sensor.setFilterAlpha(0.3f);
    
    float current_speed = 0.0f;
    
    while (1) {
        // 读取传感器
        sensor.getData(data);
        
        // 计算当前速度（假设）
        current_speed = calculate_speed();
        
        // 根据速度动态调整滤波
        sensor.setFilterBySpeed(current_speed);
        
        // 巡线控制
        // ...
        
        HAL_Delay(10);
    }
}

/* ========== 示例8: 不同场景使用不同滤波 ========== */
void example8_scenario_based() {
    LineSensor sensor;
    uint16_t data[8];
    
    enum class LineType {
        STRAIGHT,    // 直线
        CURVE,       // 弯道
        CROSSING     // 十字路口
    };
    
    LineType current_type = LineType::STRAIGHT;
    
    while (1) {
        // 根据路况调整滤波
        switch (current_type) {
            case LineType::STRAIGHT:
                // 直线：可以用弱滤波，响应快
                sensor.setFilterAlpha(0.5f);
                break;
                
            case LineType::CURVE:
                // 弯道：中等滤波，平衡性能
                sensor.setFilterAlpha(0.4f);
                break;
                
            case LineType::CROSSING:
                // 十字路口：强滤波，避免误判
                sensor.setFilterAlpha(0.3f);
                break;
        }
        
        sensor.getData(data);
        
        // 判断路况（简化）
        current_type = detect_line_type(data);
        
        HAL_Delay(10);
    }
}

/* ========== 示例9: 对比不同滤波系数的效果 ========== */
void example9_compare_alpha() {
    LineSensor sensor;
    uint16_t data_alpha_03[8];
    uint16_t data_alpha_05[8];
    uint16_t data_alpha_07[8];
    
    Debug_Printf("\r\n========== 滤波系数对比测试 ==========\r\n");
    
    // 测试 α = 0.3（强滤波）
    sensor.resetFilter();
    sensor.setFilterAlpha(0.3f);
    for (int i = 0; i < 10; i++) {
        sensor.getData(data_alpha_03);
        HAL_Delay(10);
    }
    Debug_Printf("α=0.3: %d, %d, %d, %d\r\n", 
                 data_alpha_03[0], data_alpha_03[1], 
                 data_alpha_03[2], data_alpha_03[3]);
    
    // 测试 α = 0.5（中等滤波）
    sensor.resetFilter();
    sensor.setFilterAlpha(0.5f);
    for (int i = 0; i < 10; i++) {
        sensor.getData(data_alpha_05);
        HAL_Delay(10);
    }
    Debug_Printf("α=0.5: %d, %d, %d, %d\r\n", 
                 data_alpha_05[0], data_alpha_05[1], 
                 data_alpha_05[2], data_alpha_05[3]);
    
    // 测试 α = 0.7（弱滤波）
    sensor.resetFilter();
    sensor.setFilterAlpha(0.7f);
    for (int i = 0; i < 10; i++) {
        sensor.getData(data_alpha_07);
        HAL_Delay(10);
    }
    Debug_Printf("α=0.7: %d, %d, %d, %d\r\n", 
                 data_alpha_07[0], data_alpha_07[1], 
                 data_alpha_07[2], data_alpha_07[3]);
    
    Debug_Printf("========================================\r\n\r\n");
}

/* ========== 示例10: 完整的配置流程 ========== */
void example10_complete_setup() {
    LineSensor sensor;
    uint16_t data[8];
    
    Debug_Printf("\r\n========== 传感器配置 ==========\r\n");
    
    // 1. 设置阈值
    sensor.setThreshold(1550, 150);
    Debug_Printf("✓ 阈值已设置\r\n");
    
    // 2. 设置滤波系数
    sensor.setFilterAlpha(0.4f);
    Debug_Printf("✓ 滤波系数: α=%.2f\r\n", sensor.getFilterAlpha());
    
    // 3. 首次读取（自动初始化滤波器）
    sensor.getData(data);
    Debug_Printf("✓ 滤波器已初始化: %s\r\n", 
                 sensor.isFilterInitialized() ? "是" : "否");
    
    Debug_Printf("============================\r\n\r\n");
    
    // 4. 开始巡线
    while (1) {
        sensor.getData(data);
        
        // 你的巡线逻辑
        // ...
        
        HAL_Delay(10);
    }
}

/* ========== 主程序示例 ========== */
int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    // 初始化调试串口
    MX_USART1_UART_Init();
    Debug_SetUart(DEBUG_UART_1);
    Debug_Enable();
    
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("  灰度传感器滤波器配置示例\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("\r\n");
    
    // 创建传感器对象
    LineSensor sensor;
    
    // 初始配置
    Debug_Printf("默认滤波系数: α=%.2f\r\n", sensor.getFilterAlpha());
    
    // 修改滤波系数
    sensor.setFilterAlpha(0.35f);
    Debug_Printf("新的滤波系数: α=%.2f\r\n", sensor.getFilterAlpha());
    
    // 主循环
    uint16_t data[8];
    float speed = 0.3f;  // 模拟速度
    
    while (1) {
        // 读取传感器数据
        sensor.getData(data);
        
        // 根据速度调整滤波（每秒调整一次）
        static uint32_t last_adjust = 0;
        if (HAL_GetTick() - last_adjust > 1000) {
            sensor.setFilterBySpeed(speed);
            last_adjust = HAL_GetTick();
            
            // 模拟速度变化
            speed += 0.1f;
            if (speed > 0.8f) speed = 0.2f;
        }
        
        // 输出数据
        Debug_Printf("传感器: %4d %4d %4d %4d %4d %4d %4d %4d | α=%.2f\r\n",
                     data[0], data[1], data[2], data[3],
                     data[4], data[5], data[6], data[7],
                     sensor.getFilterAlpha());
        
        HAL_Delay(100);
    }
}

/* ========== 辅助函数 ========== */

float calculate_speed() {
    // 实际项目中计算小车速度
    // 这里返回模拟值
    return 0.5f;
}

LineType detect_line_type(uint16_t data[8]) {
    // 实际项目中检测线型
    // 这里返回模拟值
    return LineType::STRAIGHT;
}

void SystemClock_Config(void) {
    // 时钟配置
}
