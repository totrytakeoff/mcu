/**
 * @file    line_follower_pid_example.cpp
 * @brief   基于PID的巡线系统使用示例
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 演示如何使用LineFollowerPID类实现精确的巡线控制
 * 
 * 硬件连接：
 * - 8路灰度传感器
 * - 4个电机（左前、左后、右前、右后）
 * - 校准按钮（PD2）
 * 
 * 使用步骤：
 * 1. 上电后，长按PD2按钮3秒进行传感器校准
 * 2. 校准完成后，小车自动开始巡线
 * 3. 可通过串口查看实时调试信息
 */

#include "stm32f1xx_hal.h"
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

#include "line_follower_pid.hpp"
#include "line_sensor.hpp"
#include "motor.hpp"
#include "button.hpp"
#include "eeprom.hpp"
#include "debug.hpp"

/* ========== 全局对象 ========== */

// 电机对象（4个独立电机）
Motor motor_lf, motor_lr, motor_rf, motor_rr;

// 传感器对象
LineSensor line_sensor;

// EEPROM对象
EEPROM eeprom;

// 校准按钮（PD2，上拉模式，按下为低电平，200ms防抖）
Button calib_button(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 200);

// 巡线控制器指针
LineFollowerPID* follower = nullptr;

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
void printWelcomeMessage();
void printHelp();

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
    printWelcomeMessage();
    
    // 电机初始化（假设使用TIM1的4个通道）
    motor_lf.init(&htim1, TIM_CHANNEL_1);  // 左前
    motor_lr.init(&htim1, TIM_CHANNEL_3);  // 左后
    motor_rf.init(&htim1, TIM_CHANNEL_2);  // 右前
    motor_rr.init(&htim1, TIM_CHANNEL_4);  // 右后
    
    Debug_Printf("[系统] 电机初始化完成\r\n");
    
    // EEPROM初始化
    eeprom.init(&hi2c2, 0xA0);
    Debug_Printf("[系统] EEPROM初始化完成\r\n");
    
    // 传感器初始化
    HAL_Delay(100);
    
    // 尝试从EEPROM加载校准数据
    Debug_Printf("[系统] 尝试加载传感器校准数据...\r\n");
    if (line_sensor.loadCalibration(eeprom)) {
        Debug_Printf("[系统] 校准数据加载成功！\r\n");
        system_state = SystemState::RUNNING;
    } else {
        Debug_Printf("[系统] 未找到校准数据\r\n");
        Debug_Printf("[系统] 请长按PD2按钮进行校准...\r\n");
        system_state = SystemState::STOPPED;
    }
    
    // 创建巡线控制器
    follower = new LineFollowerPID(line_sensor, motor_lf, motor_lr, motor_rf, motor_rr);
    
    // 配置巡线参数
    follower->setLineMode(LineFollowerPID::LineMode::WHITE_ON_BLACK);  // 黑底白线
    follower->setPID(0.06f, 0.0f, 1.0f);  // PID参数
    follower->setBaseSpeed(30);           // 基础速度
    follower->setThreshold(2000);         // 黑白阈值
    follower->setLineLostThreshold(1);    // 丢线阈值
    follower->enableDebug(true);          // 启用调试输出
    
    // 初始化巡线控制器
    follower->init();
    
    Debug_Printf("[系统] 初始化完成！\r\n");
    printHelp();
    
    // 如果已有校准数据，自动开始巡线
    if (system_state == SystemState::RUNNING) {
        follower->start();
        Debug_Printf("[系统] 开始巡线...\r\n\r\n");
    }
    
    uint32_t last_update_time = HAL_GetTick();
    uint32_t last_status_time = HAL_GetTick();
    const uint32_t UPDATE_INTERVAL = 20;     // 20ms更新一次
    const uint32_t STATUS_INTERVAL = 5000;   // 5秒输出一次状态
    
    /* ========== 主循环 ========== */
    while (1) {
        uint32_t current_time = HAL_GetTick();
        
        // 更新按钮状态
        calib_button.update();
        
        // 检查是否长按校准按钮（3秒）
        if (calib_button.isLongPressed(3000)) {
            Debug_Printf("\r\n[系统] 检测到校准按钮长按\r\n");
            follower->stop();
            performCalibration();
            follower->start();
            system_state = SystemState::RUNNING;
        }
        
        // 定时更新巡线控制（20ms一次）
        if (current_time - last_update_time >= UPDATE_INTERVAL) {
            last_update_time = current_time;
            
            if (system_state == SystemState::RUNNING) {
                follower->update();
            }
        }
        
        // 定时输出状态信息（5秒一次）
        if (current_time - last_status_time >= STATUS_INTERVAL) {
            last_status_time = current_time;
            
            if (system_state == SystemState::RUNNING) {
                Debug_Printf("\r\n========== 状态信息 ==========\r\n");
                Debug_Printf("状态: %s\r\n", 
                           follower->getState() == LineFollowerPID::State::RUNNING ? "运行中" :
                           follower->getState() == LineFollowerPID::State::LINE_LOST ? "丢线" : "停止");
                Debug_Printf("误差: %.1f\r\n", follower->getError());
                Debug_Printf("PID输出: %.1f\r\n", follower->getPIDOutput());
                Debug_Printf("左速: %d, 右速: %d\r\n", 
                           follower->getLeftSpeed(), follower->getRightSpeed());
                Debug_Printf("============================\r\n\r\n");
            }
        }
        
        HAL_Delay(1);  // 防止CPU占用过高
    }
}

/**
 * @brief 执行传感器校准
 */
void performCalibration() {
    system_state = SystemState::CALIBRATING;
    
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
    follower->init();
    follower->resetPID();
    
    Debug_Printf("[校准] 校准完成！开始巡线...\r\n");
    Debug_Printf("========================================\r\n\r\n");
    
    HAL_Delay(500);
}

/**
 * @brief 打印欢迎信息
 */
void printWelcomeMessage() {
    Debug_Printf("\r\n\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("    基于PID的智能巡线系统 v1.0\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("硬件配置:\r\n");
    Debug_Printf("  - MCU: STM32F103RCT6\r\n");
    Debug_Printf("  - 传感器: 8路灰度传感器\r\n");
    Debug_Printf("  - 电机: 4个直流电机\r\n");
    Debug_Printf("  - 控制器: PID控制器\r\n");
    Debug_Printf("========================================\r\n\r\n");
}

/**
 * @brief 打印帮助信息
 */
void printHelp() {
    Debug_Printf("\r\n========== 操作说明 ==========\r\n");
    Debug_Printf("1. 校准传感器:\r\n");
    Debug_Printf("   - 长按PD2按钮3秒\r\n");
    Debug_Printf("   - 按提示完成白色/黑色校准\r\n");
    Debug_Printf("\r\n");
    Debug_Printf("2. 调整参数:\r\n");
    Debug_Printf("   - 在代码中修改PID参数\r\n");
    Debug_Printf("   - follower->setPID(Kp, Ki, Kd)\r\n");
    Debug_Printf("   - 基础速度: setBaseSpeed(speed)\r\n");
    Debug_Printf("\r\n");
    Debug_Printf("3. 查看调试信息:\r\n");
    Debug_Printf("   - 启用调试: enableDebug(true)\r\n");
    Debug_Printf("   - 串口实时输出位置、误差、速度\r\n");
    Debug_Printf("\r\n");
    Debug_Printf("4. 参数调节建议:\r\n");
    Debug_Printf("   - 低速(20-30): Kp=0.04-0.06, Kd=0.8-1.2\r\n");
    Debug_Printf("   - 中速(30-50): Kp=0.06-0.08, Kd=1.2-1.8\r\n");
    Debug_Printf("   - 高速(50-70): Kp=0.10-0.15, Kd=2.0-3.0\r\n");
    Debug_Printf("============================\r\n\r\n");
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
