/**
 * @file    drive_train_demo.cpp
 * @brief   差速转向控制演示程序
 * @author  AI Assistant
 * @date    2024
 * 
 * 本文件展示 DriveTrain 类的各种使用方法
 * 如需测试，可以替换 main.cpp 中的 main() 函数
 */

#include "stm32f1xx_hal.h"
#include "common.h"
#include "gpio.h"
#include "tim.h"
#include "motor.hpp"
#include "drive_train.hpp"

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
    
    /* 主控制循环 */
    while (1)
    {
        // ========== 测试1: 直行前进 ==========
        robot.drive(60, 0);  // 60%速度前进，不转向
        HAL_Delay(2000);
        
        robot.stop();
        HAL_Delay(500);
        
        // ========== 测试2: 前进同时左转 ==========
        robot.drive(50, 40);  // 50%前进，40%左转
        HAL_Delay(2000);
        
        robot.stop();
        HAL_Delay(500);
        
        // ========== 测试3: 前进同时右转 ==========
        robot.drive(50, -40);  // 50%前进，40%右转
        HAL_Delay(2000);
        
        robot.stop();
        HAL_Delay(500);
        
        // ========== 测试4: 原地左旋 ==========
        robot.drive(0, 60);  // 不前进，60%原地左旋
        HAL_Delay(1500);
        
        robot.stop();
        HAL_Delay(500);
        
        // ========== 测试5: 原地右旋 ==========
        robot.drive(0, -60);  // 不前进，60%原地右旋
        HAL_Delay(1500);
        
        robot.stop();
        HAL_Delay(500);
        
        // ========== 测试6: 直行后退 ==========
        robot.drive(-60, 0);  // 60%后退，不转向
        HAL_Delay(2000);
        
        robot.stop();
        HAL_Delay(500);
        
        // ========== 测试7: 后退同时右转 ==========
        robot.drive(-50, -30);  // 50%后退，30%右转
        HAL_Delay(2000);
        
        robot.stop();
        HAL_Delay(2000);  // 循环前长暂停
    }
}
