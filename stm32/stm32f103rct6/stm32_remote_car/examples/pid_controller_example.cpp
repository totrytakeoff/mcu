/**
 * @file    pid_controller_example.cpp
 * @brief   PID控制器使用示例
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 演示PID控制器类的各种使用场景：
 * 1. 基本PID控制
 * 2. 速度控制
 * 3. 位置控制
 * 4. 温度控制
 * 5. 高级功能（抗饱和、微分滤波等）
 */

#include "stm32f1xx_hal.h"
#include "pid_controller.hpp"
#include "debug.hpp"

/* ========== 示例1：基本PID控制 ========== */

/**
 * @brief 最简单的PID使用示例
 */
void example_basic_pid() {
    // 创建PID控制器
    PIDController pid(1.0f, 0.1f, 0.05f);  // Kp, Ki, Kd
    
    // 设置输出限制
    pid.setOutputLimits(-100.0f, 100.0f);
    
    // 模拟控制循环
    float setpoint = 50.0f;  // 目标值
    float measured = 0.0f;   // 当前测量值
    
    for (int i = 0; i < 100; i++) {
        // 计算PID输出
        float output = pid.compute(setpoint, measured);
        
        // 模拟系统响应（一阶系统）
        measured += output * 0.1f;
        
        // 打印调试信息
        Debug_Printf("Step %d: Setpoint=%.2f, Measured=%.2f, Output=%.2f, Error=%.2f\r\n",
                     i, setpoint, measured, output, pid.getError());
        
        HAL_Delay(20);  // 20ms控制周期
    }
}

/* ========== 示例2：电机速度控制 ========== */

/**
 * @brief 电机速度PID控制示例
 */
void example_motor_speed_control() {
    // 速度PID控制器
    PIDController speed_pid(0.5f, 0.2f, 0.01f);
    speed_pid.setOutputLimits(-100.0f, 100.0f);
    speed_pid.setSampleTime(0.02f);  // 20ms采样
    
    float target_speed = 100.0f;  // 目标速度 (RPM)
    float current_speed = 0.0f;   // 当前速度
    
    Debug_Printf("\r\n========== 电机速度控制 ==========\r\n");
    
    while (1) {
        // 假设从编码器读取当前速度
        // current_speed = encoder.getSpeed();
        
        // 计算PWM输出
        float pwm = speed_pid.compute(target_speed, current_speed);
        
        // 设置电机PWM
        // motor.setPWM(pwm);
        
        // 调试输出
        Debug_Printf("Target: %.1f RPM, Current: %.1f RPM, PWM: %.1f%%\r\n",
                     target_speed, current_speed, pwm);
        Debug_Printf("  P: %.2f, I: %.2f, D: %.2f\r\n",
                     speed_pid.getProportional(),
                     speed_pid.getIntegral(),
                     speed_pid.getDerivative());
        
        HAL_Delay(20);
    }
}

/* ========== 示例3：位置控制（串级PID）========== */

/**
 * @brief 位置控制示例（位置环+速度环）
 */
void example_position_control() {
    // 外环：位置PID
    PIDController position_pid(2.0f, 0.0f, 0.5f);
    position_pid.setOutputLimits(-100.0f, 100.0f);  // 输出是目标速度
    
    // 内环：速度PID
    PIDController speed_pid(0.5f, 0.2f, 0.01f);
    speed_pid.setOutputLimits(-100.0f, 100.0f);  // 输出是PWM
    
    float target_position = 1000.0f;  // 目标位置
    float current_position = 0.0f;    // 当前位置
    float current_speed = 0.0f;       // 当前速度
    
    Debug_Printf("\r\n========== 串级位置控制 ==========\r\n");
    
    while (1) {
        // 位置环计算目标速度
        float target_speed = position_pid.compute(target_position, current_position);
        
        // 速度环计算PWM输出
        float pwm = speed_pid.compute(target_speed, current_speed);
        
        // 设置电机
        // motor.setPWM(pwm);
        
        Debug_Printf("Pos: %.1f/%.1f, Speed: %.1f/%.1f, PWM: %.1f\r\n",
                     current_position, target_position,
                     current_speed, target_speed, pwm);
        
        HAL_Delay(20);
    }
}

/* ========== 示例4：巡线控制 ========== */

/**
 * @brief 巡线PID控制示例
 */
void example_line_following() {
    // 巡线PID控制器
    PIDController line_pid(0.06f, 0.0f, 1.0f);
    line_pid.setOutputLimits(-60.0f, 60.0f);  // 转向输出限制
    
    // 启用微分滤波（减少噪声影响）
    line_pid.setDerivativeFilter(0.2f);
    
    float line_position = 0.0f;  // 线的位置 (-1000 to 1000)
    float base_speed = 30.0f;    // 基础速度
    
    Debug_Printf("\r\n========== 巡线控制 ==========\r\n");
    
    while (1) {
        // 读取线位置
        // line_position = line_sensor.getPosition();
        
        // PID计算转向量
        float steering = line_pid.compute(0.0f, line_position);
        
        // 差速控制
        float left_speed = base_speed + steering;
        float right_speed = base_speed - steering;
        
        // 设置电机速度
        // left_motor.setSpeed(left_speed);
        // right_motor.setSpeed(right_speed);
        
        Debug_Printf("Line Pos: %.1f, Steering: %.1f, L/R: %.1f/%.1f\r\n",
                     line_position, steering, left_speed, right_speed);
        
        HAL_Delay(20);
    }
}

/* ========== 示例5：温度控制 ========== */

/**
 * @brief 温度PID控制示例
 */
void example_temperature_control() {
    // 温度PID（通常需要积分项来消除稳态误差）
    PIDController temp_pid(5.0f, 0.5f, 1.0f);
    temp_pid.setOutputLimits(0.0f, 100.0f);  // 加热功率 0-100%
    
    float target_temp = 50.0f;   // 目标温度
    float current_temp = 25.0f;  // 当前温度
    
    Debug_Printf("\r\n========== 温度控制 ==========\r\n");
    
    while (1) {
        // 读取温度传感器
        // current_temp = temp_sensor.read();
        
        // PID计算加热功率
        float power = temp_pid.compute(target_temp, current_temp);
        
        // 设置加热器PWM
        // heater.setPWM(power);
        
        Debug_Printf("Target: %.1f°C, Current: %.1f°C, Power: %.1f%%\r\n",
                     target_temp, current_temp, power);
        
        HAL_Delay(1000);  // 温度控制周期较慢
    }
}

/* ========== 示例6：高级功能演示 ========== */

/**
 * @brief 演示PID高级功能
 */
void example_advanced_features() {
    PIDController pid(1.0f, 0.1f, 0.05f);
    
    Debug_Printf("\r\n========== PID高级功能 ==========\r\n");
    
    // 1. 设置输出限制
    pid.setOutputLimits(-100.0f, 100.0f);
    Debug_Printf("✓ 输出限制: -100 to 100\r\n");
    
    // 2. 设置采样时间
    pid.setSampleTime(0.02f);  // 20ms
    Debug_Printf("✓ 采样时间: 20ms\r\n");
    
    // 3. 启用积分抗饱和
    pid.setAntiWindup(true);
    Debug_Printf("✓ 积分抗饱和: 启用\r\n");
    
    // 4. 设置微分滤波
    pid.setDerivativeFilter(0.3f);
    Debug_Printf("✓ 微分滤波系数: 0.3\r\n");
    
    // 5. 反向控制（用于冷却等场景）
    pid.setDirection(PIDController::Direction::REVERSE);
    Debug_Printf("✓ 控制方向: 反向\r\n");
    
    // 6. 手动/自动切换
    pid.setMode(PIDController::Mode::MANUAL);
    Debug_Printf("✓ 控制模式: 手动\r\n");
    
    pid.setMode(PIDController::Mode::AUTOMATIC);
    Debug_Printf("✓ 控制模式: 自动\r\n");
    
    // 7. 重置控制器
    pid.reset();
    Debug_Printf("✓ 控制器已重置\r\n");
    
    // 8. 运行时调参
    float setpoint = 100.0f;
    float measured = 50.0f;
    
    // 初始参数
    pid.setTunings(1.0f, 0.1f, 0.05f);
    float out1 = pid.compute(setpoint, measured);
    Debug_Printf("\r\n参数1 (Kp=1.0, Ki=0.1, Kd=0.05): Output=%.2f\r\n", out1);
    
    // 修改参数（更激进）
    pid.reset();
    pid.setTunings(2.0f, 0.5f, 0.1f);
    float out2 = pid.compute(setpoint, measured);
    Debug_Printf("参数2 (Kp=2.0, Ki=0.5, Kd=0.1): Output=%.2f\r\n", out2);
    
    // 9. 读取PID各项
    Debug_Printf("\r\nPID各项详情:\r\n");
    Debug_Printf("  比例项 P: %.2f\r\n", pid.getProportional());
    Debug_Printf("  积分项 I: %.2f\r\n", pid.getIntegral());
    Debug_Printf("  微分项 D: %.2f\r\n", pid.getDerivative());
    Debug_Printf("  总输出:   %.2f\r\n", pid.getOutput());
    Debug_Printf("  误差:     %.2f\r\n", pid.getError());
}

/* ========== 示例7：动态调参（自整定模拟）========== */

/**
 * @brief 演示动态调整PID参数
 */
void example_dynamic_tuning() {
    PIDController pid(0.5f, 0.0f, 0.0f);
    pid.setOutputLimits(-100.0f, 100.0f);
    
    float setpoint = 100.0f;
    float measured = 0.0f;
    
    Debug_Printf("\r\n========== 动态调参演示 ==========\r\n");
    
    // 阶段1：只有P
    Debug_Printf("\r\n阶段1: 只有P控制\r\n");
    pid.setTunings(0.5f, 0.0f, 0.0f);
    for (int i = 0; i < 20; i++) {
        float output = pid.compute(setpoint, measured);
        measured += output * 0.1f;
        Debug_Printf("Step %d: Measured=%.2f, Error=%.2f\r\n", 
                     i, measured, pid.getError());
        HAL_Delay(20);
    }
    
    // 阶段2：加入D
    Debug_Printf("\r\n阶段2: P+D控制\r\n");
    pid.reset();
    measured = 0.0f;
    pid.setTunings(0.5f, 0.0f, 0.1f);
    for (int i = 0; i < 20; i++) {
        float output = pid.compute(setpoint, measured);
        measured += output * 0.1f;
        Debug_Printf("Step %d: Measured=%.2f, Error=%.2f\r\n", 
                     i, measured, pid.getError());
        HAL_Delay(20);
    }
    
    // 阶段3：加入I
    Debug_Printf("\r\n阶段3: P+I+D控制\r\n");
    pid.reset();
    measured = 0.0f;
    pid.setTunings(0.5f, 0.05f, 0.1f);
    for (int i = 0; i < 20; i++) {
        float output = pid.compute(setpoint, measured);
        measured += output * 0.1f;
        Debug_Printf("Step %d: Measured=%.2f, P=%.2f, I=%.2f, D=%.2f\r\n", 
                     i, measured,
                     pid.getProportional(),
                     pid.getIntegral(),
                     pid.getDerivative());
        HAL_Delay(20);
    }
}

/* ========== 主函数 ========== */

extern "C" {
void SystemClock_Config(void);
void Error_Handler(void);
}

int main(void) {
    // HAL初始化
    HAL_Init();
    SystemClock_Config();
    
    // 外设初始化
    // ... (根据需要初始化UART、GPIO等)
    
    // 调试串口初始化
    Debug_Init();
    
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("       PID控制器使用示例集合\r\n");
    Debug_Printf("========================================\r\n");
    
    // 选择要运行的示例
    // 取消注释你想运行的示例：
    
    // example_basic_pid();
    // example_motor_speed_control();
    // example_position_control();
    // example_line_following();
    // example_temperature_control();
    example_advanced_features();
    // example_dynamic_tuning();
    
    while (1) {
        HAL_Delay(1000);
    }
}

extern "C" void SystemClock_Config(void) {
    // CubeMX生成的时钟配置...
}

extern "C" void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
