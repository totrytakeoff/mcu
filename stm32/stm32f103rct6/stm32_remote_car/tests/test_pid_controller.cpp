/**
 * @file    test_pid_controller.cpp
 * @brief   PID控制器单元测试
 * @author  AI Assistant
 * @date    2024
 * 
 * @description
 * 测试PID控制器的各项功能：
 * 1. 基本P控制
 * 2. PD控制
 * 3. PID控制
 * 4. 输出限制
 * 5. 积分抗饱和
 * 6. 微分滤波
 * 7. 模式切换
 * 8. 方向控制
 */

#include "stm32f1xx_hal.h"
#include "pid_controller.hpp"
#include "debug.hpp"
#include <math.h>

/* ========== 测试辅助函数 ========== */

/**
 * @brief 简单的一阶系统模拟
 * @param current 当前值
 * @param input 输入（控制量）
 * @param time_constant 时间常数
 * @param dt 时间步长
 * @return 更新后的值
 */
float simulate_first_order_system(float current, float input, float time_constant, float dt) {
    float alpha = dt / (time_constant + dt);
    return current + alpha * (input - current);
}

/**
 * @brief 检查浮点数是否接近
 */
bool is_close(float a, float b, float tolerance = 0.01f) {
    return fabsf(a - b) < tolerance;
}

/**
 * @brief 打印测试结果
 */
void print_test_result(const char* test_name, bool passed) {
    if (passed) {
        Debug_Printf("[✓] %s\r\n", test_name);
    } else {
        Debug_Printf("[✗] %s - FAILED\r\n", test_name);
    }
}

/* ========== 测试用例 ========== */

/**
 * @brief 测试1: P控制器基本功能
 */
bool test_proportional_only() {
    Debug_Printf("\r\n========== 测试1: P控制 ==========\r\n");
    
    PIDController pid(1.0f, 0.0f, 0.0f);  // 只有P
    pid.setOutputLimits(-100.0f, 100.0f);
    
    float setpoint = 100.0f;
    float measured = 0.0f;
    
    // 计算输出
    float output = pid.compute(setpoint, measured);
    
    // P控制: output = Kp * error = 1.0 * (100 - 0) = 100
    Debug_Printf("  Setpoint: %.2f, Measured: %.2f\r\n", setpoint, measured);
    Debug_Printf("  Expected output: 100.00, Actual: %.2f\r\n", output);
    Debug_Printf("  P term: %.2f, I term: %.2f, D term: %.2f\r\n",
                 pid.getProportional(), pid.getIntegral(), pid.getDerivative());
    
    bool passed = is_close(output, 100.0f, 1.0f) && 
                  is_close(pid.getIntegral(), 0.0f, 0.01f) &&
                  is_close(pid.getDerivative(), 0.0f, 0.01f);
    
    print_test_result("P控制基本功能", passed);
    return passed;
}

/**
 * @brief 测试2: 输出限制功能
 */
bool test_output_limits() {
    Debug_Printf("\r\n========== 测试2: 输出限制 ==========\r\n");
    
    PIDController pid(2.0f, 0.0f, 0.0f);
    pid.setOutputLimits(-50.0f, 50.0f);  // 限制在±50
    
    float setpoint = 100.0f;
    float measured = 0.0f;
    
    // 理论输出 = 2.0 * 100 = 200，但应限制在50
    float output = pid.compute(setpoint, measured);
    
    Debug_Printf("  Unlimited output would be: 200.00\r\n");
    Debug_Printf("  Limited output: %.2f\r\n", output);
    Debug_Printf("  Limits: -50.00 to 50.00\r\n");
    
    bool passed = is_close(output, 50.0f, 1.0f);
    print_test_result("输出限制", passed);
    return passed;
}

/**
 * @brief 测试3: PD控制器
 */
bool test_pd_controller() {
    Debug_Printf("\r\n========== 测试3: PD控制 ==========\r\n");
    
    PIDController pid(1.0f, 0.0f, 0.5f);
    pid.setOutputLimits(-100.0f, 100.0f);
    
    float setpoint = 100.0f;
    float measured = 50.0f;
    
    // 第一次调用
    pid.reset();
    float output1 = pid.compute(setpoint, measured);
    Debug_Printf("  First call - Error: 50, Output: %.2f (D=0)\r\n", output1);
    
    // 第二次调用（测量值增加，误差减小）
    measured = 60.0f;
    HAL_Delay(20);
    float output2 = pid.compute(setpoint, measured);
    Debug_Printf("  Second call - Error: 40, Output: %.2f (D<0, 减速)\r\n", output2);
    
    // D项应该是负数（因为误差在减小）
    bool passed = output2 < output1;
    Debug_Printf("  P term: %.2f, D term: %.2f\r\n", 
                 pid.getProportional(), pid.getDerivative());
    
    print_test_result("PD控制（D项抑制作用）", passed);
    return passed;
}

/**
 * @brief 测试4: PID完整控制
 */
bool test_full_pid() {
    Debug_Printf("\r\n========== 测试4: PID完整控制 ==========\r\n");
    
    PIDController pid(1.0f, 0.1f, 0.2f);
    pid.setOutputLimits(-100.0f, 100.0f);
    
    float setpoint = 100.0f;
    float measured = 50.0f;
    
    pid.reset();
    
    // 运行几步，观察积分累积
    for (int i = 0; i < 5; i++) {
        float output = pid.compute(setpoint, measured);
        Debug_Printf("  Step %d: Error=%.1f, P=%.2f, I=%.2f, D=%.2f, Out=%.2f\r\n",
                     i, pid.getError(),
                     pid.getProportional(),
                     pid.getIntegral(),
                     pid.getDerivative(),
                     output);
        HAL_Delay(20);
        measured += 5.0f;  // 模拟接近目标
    }
    
    // 积分项应该在增长
    bool passed = pid.getIntegral() > 0.1f;
    print_test_result("PID完整控制（积分累积）", passed);
    return passed;
}

/**
 * @brief 测试5: 积分抗饱和
 */
bool test_anti_windup() {
    Debug_Printf("\r\n========== 测试5: 积分抗饱和 ==========\r\n");
    
    // 创建两个PID：一个有抗饱和，一个没有
    PIDController pid_with(1.0f, 0.5f, 0.0f);
    pid_with.setOutputLimits(-50.0f, 50.0f);
    pid_with.setAntiWindup(true);
    
    PIDController pid_without(1.0f, 0.5f, 0.0f);
    pid_without.setOutputLimits(-50.0f, 50.0f);
    pid_without.setAntiWindup(false);
    
    float setpoint = 100.0f;
    float measured = 0.0f;
    
    // 运行若干步，输出会饱和
    for (int i = 0; i < 20; i++) {
        pid_with.compute(setpoint, measured);
        pid_without.compute(setpoint, measured);
        HAL_Delay(20);
    }
    
    Debug_Printf("  有抗饱和 - I term: %.2f\r\n", pid_with.getIntegral());
    Debug_Printf("  无抗饱和 - I term: %.2f\r\n", pid_without.getIntegral());
    
    // 有抗饱和的积分项应该更小
    bool passed = pid_with.getIntegral() < pid_without.getIntegral();
    print_test_result("积分抗饱和", passed);
    return passed;
}

/**
 * @brief 测试6: 反向控制
 */
bool test_reverse_direction() {
    Debug_Printf("\r\n========== 测试6: 反向控制 ==========\r\n");
    
    PIDController pid_direct(1.0f, 0.0f, 0.0f);
    pid_direct.setDirection(PIDController::Direction::DIRECT);
    pid_direct.setOutputLimits(-100.0f, 100.0f);
    
    PIDController pid_reverse(1.0f, 0.0f, 0.0f);
    pid_reverse.setDirection(PIDController::Direction::REVERSE);
    pid_reverse.setOutputLimits(-100.0f, 100.0f);
    
    float setpoint = 100.0f;
    float measured = 50.0f;
    
    float out_direct = pid_direct.compute(setpoint, measured);
    float out_reverse = pid_reverse.compute(setpoint, measured);
    
    Debug_Printf("  正向控制输出: %.2f\r\n", out_direct);
    Debug_Printf("  反向控制输出: %.2f\r\n", out_reverse);
    
    // 反向控制的输出应该是相反的符号
    bool passed = is_close(out_direct, -out_reverse, 1.0f);
    print_test_result("反向控制", passed);
    return passed;
}

/**
 * @brief 测试7: 手动/自动模式切换
 */
bool test_mode_switching() {
    Debug_Printf("\r\n========== 测试7: 模式切换 ==========\r\n");
    
    PIDController pid(1.0f, 0.0f, 0.0f);
    pid.setOutputLimits(-100.0f, 100.0f);
    
    float setpoint = 100.0f;
    float measured = 50.0f;
    
    // 自动模式
    pid.setMode(PIDController::Mode::AUTOMATIC);
    float auto_output = pid.compute(setpoint, measured);
    Debug_Printf("  自动模式输出: %.2f\r\n", auto_output);
    
    // 手动模式
    pid.setMode(PIDController::Mode::MANUAL);
    float manual_output = pid.compute(setpoint, 0.0f);  // 即使输入变了
    Debug_Printf("  手动模式输出: %.2f (应该不变)\r\n", manual_output);
    
    // 手动模式下输出不应改变
    bool passed = is_close(auto_output, manual_output, 0.01f);
    print_test_result("模式切换", passed);
    return passed;
}

/**
 * @brief 测试8: 重置功能
 */
bool test_reset() {
    Debug_Printf("\r\n========== 测试8: 重置功能 ==========\r\n");
    
    PIDController pid(1.0f, 0.5f, 0.2f);
    pid.setOutputLimits(-100.0f, 100.0f);
    
    // 运行几步累积状态
    for (int i = 0; i < 10; i++) {
        pid.compute(100.0f, 50.0f);
        HAL_Delay(20);
    }
    
    Debug_Printf("  重置前 - I: %.2f, Error: %.2f\r\n", 
                 pid.getIntegral(), pid.getError());
    
    // 重置
    pid.reset();
    
    Debug_Printf("  重置后 - I: %.2f, Error: %.2f\r\n", 
                 pid.getIntegral(), pid.getError());
    
    // 所有状态应该清零
    bool passed = is_close(pid.getIntegral(), 0.0f, 0.01f) &&
                  is_close(pid.getError(), 0.0f, 0.01f) &&
                  is_close(pid.getOutput(), 0.0f, 0.01f);
    
    print_test_result("重置功能", passed);
    return passed;
}

/**
 * @brief 测试9: 微分滤波
 */
bool test_derivative_filter() {
    Debug_Printf("\r\n========== 测试9: 微分滤波 ==========\r\n");
    
    PIDController pid_no_filter(1.0f, 0.0f, 1.0f);
    pid_no_filter.setOutputLimits(-100.0f, 100.0f);
    pid_no_filter.setDerivativeFilter(0.0f);  // 无滤波
    
    PIDController pid_with_filter(1.0f, 0.0f, 1.0f);
    pid_with_filter.setOutputLimits(-100.0f, 100.0f);
    pid_with_filter.setDerivativeFilter(0.5f);  // 强滤波
    
    // 模拟突变输入
    float measured = 50.0f;
    pid_no_filter.compute(100.0f, measured);
    pid_with_filter.compute(100.0f, measured);
    
    HAL_Delay(20);
    
    measured = 80.0f;  // 突变
    pid_no_filter.compute(100.0f, measured);
    pid_with_filter.compute(100.0f, measured);
    
    float d_no_filter = pid_no_filter.getDerivative();
    float d_with_filter = pid_with_filter.getDerivative();
    
    Debug_Printf("  无滤波D项: %.2f\r\n", d_no_filter);
    Debug_Printf("  有滤波D项: %.2f\r\n", d_with_filter);
    
    // 滤波后的微分应该更小（更平滑）
    bool passed = fabsf(d_with_filter) < fabsf(d_no_filter);
    print_test_result("微分滤波", passed);
    return passed;
}

/**
 * @brief 测试10: 实际系统仿真（一阶系统）
 */
bool test_system_simulation() {
    Debug_Printf("\r\n========== 测试10: 系统仿真 ==========\r\n");
    
    PIDController pid(0.5f, 0.1f, 0.2f);
    pid.setOutputLimits(-100.0f, 100.0f);
    
    float setpoint = 100.0f;
    float measured = 0.0f;
    float time_constant = 0.5f;  // 系统时间常数
    float dt = 0.02f;  // 20ms
    
    Debug_Printf("  模拟一阶系统响应（目标: 100）\r\n");
    Debug_Printf("  Step | Measured | Error | Output\r\n");
    Debug_Printf("  -----|----------|-------|--------\r\n");
    
    bool converged = false;
    for (int i = 0; i < 100; i++) {
        float output = pid.compute(setpoint, measured, dt);
        measured = simulate_first_order_system(measured, output, time_constant, dt);
        
        if (i % 10 == 0) {
            Debug_Printf("  %4d | %8.2f | %5.2f | %6.2f\r\n",
                         i, measured, pid.getError(), output);
        }
        
        // 检查是否收敛到目标值附近
        if (is_close(measured, setpoint, 2.0f) && i > 50) {
            converged = true;
            Debug_Printf("  系统在第%d步收敛到目标值附近\r\n", i);
            break;
        }
        
        HAL_Delay(20);
    }
    
    Debug_Printf("  最终值: %.2f (目标: %.2f)\r\n", measured, setpoint);
    print_test_result("系统仿真（收敛性）", converged);
    return converged;
}

/* ========== 主测试函数 ========== */

extern "C" {
void SystemClock_Config(void);
void Error_Handler(void);
}

int main(void) {
    // HAL初始化
    HAL_Init();
    SystemClock_Config();
    
    // 初始化串口
    Debug_Init();
    
    Debug_Printf("\r\n");
    Debug_Printf("========================================\r\n");
    Debug_Printf("     PID控制器单元测试\r\n");
    Debug_Printf("========================================\r\n");
    
    int passed = 0;
    int total = 10;
    
    // 运行所有测试
    if (test_proportional_only()) passed++;
    HAL_Delay(500);
    
    if (test_output_limits()) passed++;
    HAL_Delay(500);
    
    if (test_pd_controller()) passed++;
    HAL_Delay(500);
    
    if (test_full_pid()) passed++;
    HAL_Delay(500);
    
    if (test_anti_windup()) passed++;
    HAL_Delay(500);
    
    if (test_reverse_direction()) passed++;
    HAL_Delay(500);
    
    if (test_mode_switching()) passed++;
    HAL_Delay(500);
    
    if (test_reset()) passed++;
    HAL_Delay(500);
    
    if (test_derivative_filter()) passed++;
    HAL_Delay(500);
    
    if (test_system_simulation()) passed++;
    HAL_Delay(500);
    
    // 打印总结
    Debug_Printf("\r\n========================================\r\n");
    Debug_Printf("测试完成: %d/%d 通过\r\n", passed, total);
    if (passed == total) {
        Debug_Printf("✓ 所有测试通过！\r\n");
    } else {
        Debug_Printf("✗ 有 %d 个测试失败\r\n", total - passed);
    }
    Debug_Printf("========================================\r\n");
    
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
