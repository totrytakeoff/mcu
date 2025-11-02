/**
 * @file    main.cpp
 * @brief   基于PID的智能巡线系统
 * @author  AI Assistant
 * @date    2024
 *
 * @description
 * 使用8路灰度传感器和PID控制器实现精确巡线
 * - 基于PID控制器的差速转向
 * - OLED实时显示
 * - EEPROM校准数据持久化
 * - 按钮控制校准
 */

#include <stdio.h>
#include "stm32f1xx_hal.h"

// 外设驱动
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

// 功能模块
#include "button.hpp"
#include "debug.hpp"
#include "eeprom.hpp"
#include "line_follower_pid.hpp"
#include "line_sensor.hpp"
#include "motor.hpp"
#include "oled_display.hpp"

// 第三方库
#include <U8g2lib.h>

/* ========== 全局对象 ========== */

// 硬件对象
Motor motor_lf, motor_lr, motor_rf, motor_rr;
LineSensor line_sensor;
EEPROM eeprom;
OLEDDisplay g_oled;
Button calib_button(GPIOD, GPIO_PIN_2, ButtonMode::PULL_UP, 200);

// 巡线控制器
LineFollowerPID* follower = nullptr;

// 系统状态
enum class SystemState {
    STOPPED,      // 停止（等待校准）
    CALIBRATING,  // 校准中
    RUNNING       // 运行中
};

SystemState system_state = SystemState::STOPPED;

/* ========== 函数声明 ========== */

extern "C" {
void SystemClock_Config(void);
void Error_Handler(void);
void HAL_MspInit(void);
}

void initHardware();
void initSystem();
bool loadCalibrationData();
void performCalibration();
void updateOLEDDisplay();
void setLED(bool on);

/* ========== 主程序 ========== */

int main(void) {
    // 硬件初始化
    initHardware();

    // 系统初始化
    initSystem();

    // 主循环时间控制
    uint32_t last_control_update = HAL_GetTick();
    uint32_t last_oled_update = HAL_GetTick();

    const uint32_t CONTROL_INTERVAL = 10;  // 10ms控制周期（100Hz）
    const uint32_t OLED_INTERVAL = 100;    // 100ms显示更新（10Hz）

    /* ========== 主循环 ========== */
    while (1) {
        uint32_t now = HAL_GetTick();

        // 检查校准按钮（长按3秒）
        if (calib_button.isLongPressed(3000)) {
            if (follower) follower->stop();
            setLED(true);
            performCalibration();
            setLED(false);
            if (follower) follower->start();
            system_state = SystemState::RUNNING;
        }

        // 控制循环更新（20ms）
        if (now - last_control_update >= CONTROL_INTERVAL) {
            last_control_update = now;

            if (system_state == SystemState::RUNNING && follower) {
                follower->update();
            }
        }

        // OLED显示更新（100ms）
        if (now - last_oled_update >= OLED_INTERVAL) {
            last_oled_update = now;
            updateOLEDDisplay();
        }

        // CPU空闲时进入低功耗等待，而不是阻塞延迟
        __WFI();  // Wait For Interrupt - 节能且提高响应性
    }
}

/* ========== 初始化函数 ========== */

/**
 * @brief 硬件初始化
 */
void initHardware() {
    // HAL库初始化
    HAL_Init();
    SystemClock_Config();

    // 外设初始化
    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_I2C2_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();

    // 启动PWM
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    // 按钮初始化
    calib_button.init();

    // OLED初始化
    HAL_Delay(100);
    if (g_oled.init()) {
        g_oled.clear();
        g_oled.printLine(0, "STM32 Car v2.0");
        g_oled.printLine(1, "Initializing...");
        g_oled.show();
    }

    // 电机初始化
    motor_lf.init(&htim3, TIM_CHANNEL_1);
    motor_lr.init(&htim3, TIM_CHANNEL_3);
    motor_rf.init(&htim3, TIM_CHANNEL_2);
    motor_rr.init(&htim3, TIM_CHANNEL_4);

    // EEPROM初始化
    eeprom.init();

    HAL_Delay(100);
}

/**
 * @brief 系统初始化
 */
void initSystem() {
    // 尝试加载校准数据
    bool calibration_loaded = loadCalibrationData();

    // 创建巡线控制器
    follower = new LineFollowerPID(line_sensor, motor_lf, motor_lr, motor_rf, motor_rr);

    // 配置巡线参数
    follower->setLineMode(LineSensor::LineMode::BLACK_ON_WHITE);

    // ========== PID参数调优方案 ==========
    // 当前问题：响应不够灵敏，容易跑偏，不能及时纠正

    /**
     * PID参数调节指南：
     *
     * Kp（比例增益）：
     * - 作用：根据当前误差大小调整响应强度
     * - 过小：响应迟缓，纠正不及时
     * - 过大：响应过激，容易震荡
     *
     * Kd（微分增益）：
     * - 作用：预测误差变化趋势，提供阻尼
     * - 过小：超调严重，震荡不止
     * - 过大：响应迟钝，抑制过度
     *
     * Ki（积分增益）：
     * - 作用：累积历史误差，消除稳态偏差
     * - 过小：稳态误差大，容易跑偏
     * - 过大：积分饱和，响应延迟
     */

    /**
     * 针对传感器梯度跳跃问题的PID参数调整：
     *
     * 问题分析：8个传感器权重间隔285.7单位，当线在传感器间切换时
     * 会产生大的位置跳跃，导致PID过度响应。
     *
     * 解决策略：大幅降低PID增益，减少对梯度跳跃的敏感度
     */

    // 方案Aggressive：快速见效，后续再抑振微调
    follower->setPID(0.20f, 0.001f, 0.20f);
    // 适度提高Kp：从0.03→0.06，提高响应灵敏度
    // 适度提高Kd：从0.08→0.12，增加阻尼减少震荡
    // 添加少量Ki：消除稳态误差，避免持续跑偏

    // 方案2：极低响应（如果方案1还是太快）
    // follower->setPID(0.02f, 0.0f, 0.05f);

    // 方案3：保守响应（需要稳定时使用）
    // follower->setPID(0.025f, 0.0f, 0.06f);

    follower->setBaseSpeed(24);

    // ========== 可调参数设置（解决调整效果不明显问题）==========

    // 调参说明：BaseSpeed=24时，不同方案的实际效果
    // - 方案1（当前启用）：最大调整±40% = ±9.6速度变化，小偏差±15% = ±3.6
    // - 方案2（平衡）：最大调整±35% = ±8.4速度变化，小偏差±10% = ±2.4
    // - 方案3（保守）：最大调整±20% = ±4.8速度变化，小偏差±5% = ±1.2

        // 调整控制参数以适应新的传感器算法（严重问题修复）
    // 放大差速幅度，降低内侧最小速度，提高PID输出上限（并适度增大最高速度比例）
    follower->setControlParameters(0.7f, 0.22f, 1.8f, 1.0f);
    // 最大调整幅度：30%（BaseSpeed=18时±5.4）
    // 最小速度：50%（确保不会停止或反转）
    // 最大速度：130%（避免过度加速）
    // PID输出限制：60%（配合PID参数调整）

    // 方案2：平衡调参（如果方案1太激进）
    // follower->setControlParameters(0.35f, 0.15f, 1.7f, 0.5f);

    // 方案3：精确微调（原方案2，过于保守）
    // follower->setControlParameters(0.2f, 0.1f, 1.5f, 0.3f);

    // 非线性映射参数（优化增强小偏差响应）
    // 保留默认非线性参数（当前代码路径未使用这些参数）

    // follower 不再需要设置阈值，使用传感器的独立阈值
    follower->setLineLostThreshold(1);
    follower->enableDebug(true);
    // 提升传感器响应（α越大越快）
    line_sensor.setFilterAlpha(0.8f);

    follower->init();

    // 根据校准状态决定是否启动
    if (calibration_loaded) {
        follower->start();
        system_state = SystemState::RUNNING;
        Debug_Printf("[系统] 自动启动巡线\r\n");
    } else {
        system_state = SystemState::STOPPED;
        g_oled.clear();
        g_oled.printLine(0, "Need Calibration");
        g_oled.printLine(1, "Hold BTN 3s");
        g_oled.show();
        Debug_Printf("[系统] 等待校准\r\n");
    }
}

/**
 * @brief 加载校准数据
 * @return true=成功，false=失败
 */
bool loadCalibrationData() {
    Debug_Printf("[系统] 加载校准数据...\r\n");

    if (!line_sensor.loadCalibration(eeprom)) {
        Debug_Printf("[系统] 无校准数据\r\n");
        return false;
    }

    Debug_Printf("[系统] 校准数据加载成功\r\n");

    // 显示校准数据
    uint16_t white_vals[8], black_vals[8];
    line_sensor.getCalibrationValues(white_vals, black_vals);

    g_oled.clear();
    g_oled.setFont(u8g2_font_5x7_tf);
    g_oled.printAt(0, 8, "Calibration OK");

    // 显示白色值
    char line[32];
    snprintf(line, sizeof(line), "W:%d %d %d %d", white_vals[0], white_vals[1], white_vals[2],
             white_vals[3]);
    g_oled.printAt(0, 22, line);
    snprintf(line, sizeof(line), "  %d %d %d %d", white_vals[4], white_vals[5], white_vals[6],
             white_vals[7]);
    g_oled.printAt(0, 30, line);

    // 显示黑色值
    snprintf(line, sizeof(line), "B:%d %d %d %d", black_vals[0], black_vals[1], black_vals[2],
             black_vals[3]);
    g_oled.printAt(0, 42, line);
    snprintf(line, sizeof(line), "  %d %d %d %d", black_vals[4], black_vals[5], black_vals[6],
             black_vals[7]);
    g_oled.printAt(0, 54, line);

    g_oled.setFont(u8g2_font_6x10_tf);
    g_oled.show();
    HAL_Delay(3000);

    return true;
}

/**
 * @brief 执行传感器校准
 */
void performCalibration() {
    system_state = SystemState::CALIBRATING;

    Debug_Printf("\r\n========== 开始校准 ==========\r\n");

    // 进入校准界面（阻塞流程下主动刷新一次显示）
    if (g_oled.isInitialized()) {
        g_oled.clear();
        g_oled.setFont(u8g2_font_6x10_tf);
        g_oled.printLine(0, "CALIBRATING...");
        g_oled.printLine(2, "Step 1: WHITE");
        g_oled.printLine(3, "Step 2: BLACK");
        g_oled.printLine(4, "Step 3: SAVE");
        g_oled.show();
    }

    // 执行校准流程
    line_sensor.autoCalibrate(calib_button);

    // 保存到EEPROM
    if (line_sensor.saveCalibration(eeprom)) {
        Debug_Printf("[校准] 保存成功\r\n");
    } else {
        Debug_Printf("[校准] 保存失败\r\n");
    }

    // 重新初始化控制器
    if (follower) {
        follower->init();
        follower->resetPID();
    }

    Debug_Printf("========== 校准完成 ==========\r\n\r\n");
    HAL_Delay(500);
}

/**
 * @brief 更新OLED显示
 */
void updateOLEDDisplay() {
    if (!g_oled.isInitialized() || !follower) {
        return;
    }

    g_oled.clear();
    g_oled.setFont(u8g2_font_6x10_tf);

    // 获取状态
    int left_speed = follower->getLeftSpeed();
    int right_speed = follower->getRightSpeed();
    float position = follower->getPosition();  // 获取线位置
    float error = follower->getError();

    const char* state_str;
    if (system_state == SystemState::RUNNING) {
        switch (follower->getState()) {
            case LineFollowerPID::State::RUNNING:
                state_str = "RUN";
                break;
            case LineFollowerPID::State::LINE_LOST:
                state_str = "LOST";
                break;
            case LineFollowerPID::State::STOPPED:
                state_str = "STOP";
                break;
            default:
                state_str = "???";
        }
    } else {
        state_str = (system_state == SystemState::CALIBRATING) ? "CALIB" : "WAIT";
    }


    g_oled.printAt(90, 10, state_str);
    // 第0行：速度和位置
    char line[32];

    snprintf(line, sizeof(line), "L:%d R:%d", left_speed, right_speed);
    g_oled.printLine(0, line);

    // 第1行：位置和误差（显示实际值，乘以1000以显示整数）
    snprintf(line, sizeof(line), "P:%4d E:%4d", (int)(position), (int)(error));
    g_oled.printLine(2, line);

    // 第2-4行：传感器位图（使用Follower缓存，避免重复采样）
    uint16_t sensor_data[8];
    bool binary_data[8];
    follower->getLastSensorData(sensor_data);
    follower->getLastBinaryData(binary_data);

    // 绘制传感器矩形
    const uint8_t sensor_width = 14;
    const uint8_t sensor_height = 20;
    const uint8_t start_y = 32;  // 调整起始位置，为误差行留出空间
    const uint8_t spacing = 2;

    for (int i = 0; i < 8; i++) {
        // 翻转索引：传感器0显示在最右边，传感器7显示在最左边
        uint8_t sensor_index = 7 - i;
        uint8_t x = i * (sensor_width + spacing);

        if (binary_data[sensor_index]) {
            g_oled.drawBox(x, start_y, sensor_width, sensor_height);  // 实心
        } else {
            g_oled.drawRect(x, start_y, sensor_width, sensor_height);  // 空心
        }
    }

    g_oled.show();
}

/**
 * @brief LED控制
 */
void setLED(bool on) { HAL_GPIO_WritePin(LED_PORT, LED_PIN, on ? GPIO_PIN_RESET : GPIO_PIN_SET); }

/* ========== 系统配置函数 ========== */

#ifdef __cplusplus
extern "C" {
#endif

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    // HSE配置：8MHz外部晶振
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;

    // PLL配置：8MHz * 9 = 72MHz
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // 时钟配置：SYSCLK=72MHz, AHB=72MHz, APB1=36MHz, APB2=72MHz
    RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }

    // ADC时钟配置
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // 可以添加LED闪烁指示错误
    }
}

void HAL_MspInit(void) {
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();  // 禁用JTAG，启用SWD
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
    Debug_Printf("Assert failed: %s:%lu\r\n", file, line);
}
#endif

#ifdef __cplusplus
}
#endif
