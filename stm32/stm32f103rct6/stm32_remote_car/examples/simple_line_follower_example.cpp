/**
 * @file    simple_line_follower_example.cpp
 * @brief   简易双传感器巡线系统使用示例
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 演示如何使用 SimpleLineFollower 类实现巡线控制
 * 
 * 硬件连接：
 * - 8路灰度传感器（仅使用传感器0和传感器7）
 * - 4个电机（前左、前右、后左、后右）
 * - 校准按钮（PD2）
 * 
 * 使用步骤：
 * 1. 上电后，按下校准按钮进行传感器校准
 * 2. 校准完成后，小车自动开始巡线
 * 3. 可通过串口调试查看实时状态
 */

#include "stm32f1xx_hal.h"
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

#include "simple_line_follower.hpp"
#include "line_sensor.hpp"
#include "motor.hpp"
#include "button.hpp"
#include "eeprom.hpp"
#include "debug.hpp"

/* ========== 全局对象 ========== */

// 电机对象（4个独立电机）
Motor motor_fl, motor_fr, motor_rl, motor_rr;

// 传感器对象
LineSensor line_sensor;

// EEPROM对象
EEPROM eeprom;

// 校准按钮（PD2，上拉模式，按下为低电平，200ms防抖）
Button calib_button(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 200);

// 简易巡线控制器
SimpleLineFollower* simple_follower = nullptr;

// 系统状态
enum class SystemState {
    CALIBRATING,  // 校准中
    RUNNING,      // 运行中
    STOPPED       // 停止
};

SystemState system_state = SystemState::STOPPED;

/* ========== 函数声明 ========== */
extern "C" {
void SystemClock_Config(void);
void Error_Handler(void);
}

void performCalibration();

/* ========== 主程序 ========== */

int main(void) {
    // HAL初始化
    HAL_Init();
    SystemClock_Config();
    
    // 外设初始化
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_I2C2_Init();
    MX_USART2_UART_Init();
    MX_ADC1_Init();
    
    // 启动定时器（PWM）
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    
    // 调试串口初始化
    Debug_Init();
    Debug_Printf("\r\n========================================\r\n");
    Debug_Printf("简易双传感器巡线系统\r\n");
    Debug_Printf("========================================\r\n");
    
    // 电机初始化（假设使用TIM1的4个通道）
    motor_fl.init(&htim1, TIM_CHANNEL_1);  // 前左
    motor_fr.init(&htim1, TIM_CHANNEL_2);  // 前右
    motor_rl.init(&htim1, TIM_CHANNEL_3);  // 后左
    motor_rr.init(&htim1, TIM_CHANNEL_4);  // 后右
    
    // EEPROM初始化
    eeprom.init(&hi2c2, 0xA0);
    
    // 传感器初始化
    HAL_Delay(100);
    
    // 尝试从EEPROM加载校准数据
    Debug_Printf("[系统] 尝试加载校准数据...\r\n");
    if (line_sensor.loadCalibration(eeprom)) {
        Debug_Printf("[系统] 校准数据加载成功！\r\n");
        system_state = SystemState::RUNNING;
    } else {
        Debug_Printf("[系统] 未找到校准数据，请进行校准\r\n");
        Debug_Printf("[系统] 按下校准按钮开始...\r\n");
        system_state = SystemState::STOPPED;
    }
    
    // 创建简易巡线控制器
    simple_follower = new SimpleLineFollower(line_sensor, 
                                             motor_fl, motor_fr, 
                                             motor_rl, motor_rr);
    
    // 配置巡线参数
    simple_follower->setLineMode(SimpleLineFollower::LineMode::WHITE_LINE_ON_BLACK);
    simple_follower->setBaseSpeed(20);  // 基础速度 20
    simple_follower->setSpeedGradient(1, 3, 6);  // 轻/中/重 偏离时的速度增量
    simple_follower->setThresholds(15.0f, 15.0f, 70.0f);  // 丢线/急转/在线 阈值
    simple_follower->enableDebug(true);  // 启用调试输出
    
    // 初始化巡线控制器
    simple_follower->init();
    
    Debug_Printf("[系统] 初始化完成！\r\n\r\n");
    
    uint32_t last_update_time = HAL_GetTick();
    const uint32_t UPDATE_INTERVAL = 20;  // 20ms更新一次
    
    /* ========== 主循环 ========== */
    while (1) {
        // 更新按钮状态
        calib_button.update();
        
        // 检查是否按下校准按钮
        if (calib_button.isPressed()) {
            Debug_Printf("\r\n[系统] 检测到校准按钮按下\r\n");
            performCalibration();
            system_state = SystemState::RUNNING;
        }
        
        // 根据系统状态执行相应操作
        switch (system_state) {
            case SystemState::RUNNING: {
                // 定时更新巡线控制（20ms一次）
                uint32_t current_time = HAL_GetTick();
                if (current_time - last_update_time >= UPDATE_INTERVAL) {
                    simple_follower->update();
                    last_update_time = current_time;
                }
                break;
            }
            
            case SystemState::STOPPED:
                // 停止状态，等待校准
                simple_follower->stop();
                HAL_Delay(100);
                break;
                
            case SystemState::CALIBRATING:
                // 校准状态（在 performCalibration 中处理）
                break;
        }
        
        HAL_Delay(1);  // 防止CPU占用过高
    }
}

/**
 * @brief 执行传感器校准
 */
void performCalibration() {
    system_state = SystemState::CALIBRATING;
    simple_follower->stop();
    
    Debug_Printf("\r\n========================================\r\n");
    Debug_Printf("开始传感器校准\r\n");
    Debug_Printf("========================================\r\n");
    
    // 使用自动校准功能
    line_sensor.autoCalibrate(calib_button);
    
    // 保存校准数据到EEPROM
    Debug_Printf("[校准] 保存校准数据到EEPROM...\r\n");
    if (line_sensor.saveCalibration(eeprom)) {
        Debug_Printf("[校准] 保存成功！\r\n");
    } else {
        Debug_Printf("[校准] 保存失败！\r\n");
    }
    
    // 重新初始化巡线控制器（加载新的校准数据）
    simple_follower->init();
    
    Debug_Printf("[校准] 校准完成！开始巡线...\r\n");
    Debug_Printf("========================================\r\n\r\n");
    
    HAL_Delay(500);
}

/* ========== 系统配置函数（由CubeMX生成）========== */

extern "C" void SystemClock_Config(void) {
    // CubeMX生成的时钟配置代码...
}

extern "C" void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
extern "C" void assert_failed(uint8_t* file, uint32_t line) {
    Debug_Printf("Assert failed: file %s on line %d\r\n", file, line);
}
#endif
