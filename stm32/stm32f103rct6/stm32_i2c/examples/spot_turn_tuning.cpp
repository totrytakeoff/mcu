/**
 * @file    spot_turn_tuning.cpp
 * @brief   原地转向调试程序 - 找到最佳转向速度
 * @author  AI Assistant
 * @date    2024
 * 
 * 用于测试和调整原地转向参数，找到你的小车在不同地面上能正常转动的最佳速度
 */

#include "stm32f1xx_hal.h"
#include "common.h"
#include "gpio.h"
#include "tim.h"
#include "../include/motor.hpp"
#include "../include/drive_train.hpp"

extern "C" int main(void)
{
    /* HAL 初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 外设初始化 */
    MX_GPIO_Init();
    MX_TIM3_Init();
    
    /* 启动所有电机通道的PWM */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    
    /* 初始化电机 */
    Motor leftFront(&htim3, TIM_CHANNEL_1);
    Motor leftBack(&htim3, TIM_CHANNEL_3);
    Motor rightFront(&htim3, TIM_CHANNEL_2);
    Motor rightBack(&htim3, TIM_CHANNEL_4);
    
    /* 创建小车控制对象 */
    DriveTrain robot(leftFront, leftBack, rightFront, rightBack);
    
    /* 
     * 渐进式测试原地转向
     * 从低速度逐渐增加，找到能稳定转动的最小速度
     */
    while (1)
    {
        // ========== 测试1: 低速原地左旋 (30%) ==========
        robot.drive(0, 30);  // 实际速度会被降低到 30*0.6=18%
        HAL_Delay(3000);
        robot.stop();
        HAL_Delay(2000);
        
        // ========== 测试2: 中速原地左旋 (50%) ==========
        robot.drive(0, 50);  // 实际速度会被降低到 50*0.6=30%
        HAL_Delay(3000);
        robot.stop();
        HAL_Delay(2000);
        
        // ========== 测试3: 高速原地左旋 (70%) ==========
        robot.drive(0, 70);  // 实际速度会被降低到 70*0.6=42%
        HAL_Delay(3000);
        robot.stop();
        HAL_Delay(2000);
        
        // ========== 测试4: 低速原地右旋 ==========
        robot.drive(0, -30);
        HAL_Delay(3000);
        robot.stop();
        HAL_Delay(2000);
        
        // ========== 测试5: 对比 - 带直行的转向（应该正常）==========
        robot.drive(40, 40);  // 前进+左转，摩擦力小
        HAL_Delay(3000);
        robot.stop();
        HAL_Delay(3000);  // 长暂停
    }
}

/**
 * 调试指南：
 * 
 * 1. 观察哪个测试能正常转动：
 *    - 如果测试1都转不动 → 降低 SPOT_TURN_REDUCTION 到 0.5 或 0.4
 *    - 如果测试1能转但测试2/3堵转 → 当前设置合适
 *    - 如果所有测试都能转 → 可以提高 SPOT_TURN_REDUCTION 到 0.7-0.8
 * 
 * 2. 修改 drive_train.cpp 中的参数：
 *    constexpr float SPOT_TURN_REDUCTION = 0.6f;
 *    
 *    粗糙地面建议：0.4 - 0.5
 *    光滑地面建议：0.6 - 0.7
 * 
 * 3. 如果降低速度后还是不行，考虑：
 *    - 检查电池电压是否充足
 *    - 检查电机驱动器电流限制
 *    - 减轻小车负载
 *    - 使用更大扭矩的电机
 */
