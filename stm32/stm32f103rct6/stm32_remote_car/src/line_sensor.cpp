#include "adc.h"
#include "button.hpp"
#include "common.h"
#include "debug.hpp"
#include "gpio.h"
#include "line_sensor.hpp"
#include "stm32f1xx_hal.h"
#include <math.h>
/* ========== 内部辅助函数 ========== */

/**
 * @brief 简单的冒泡排序（用于中值滤波）
 * @param arr 数组
 * @param n 数组长度
 * @note 专为5个元素优化，避免使用STL
 */
static void bubbleSort(uint16_t arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                uint16_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* ========== LineSensor类实现 ========== */

LineSensor::LineSensor() { MX_ADC1_Init(); }

void LineSensor::getRawData(uint16_t data[8]) {
    ADC_ReadAll(data);
    Debug_Printf("[LineSensor] Raw Data: %d, %d, %d, %d, %d, %d, %d, %d\n", data[0], data[1],
                 data[2], data[3], data[4], data[5], data[6], data[7]);
}

void LineSensor::getData(uint16_t data[8]) {
    medianFilter(data);
    lowPassFilter(data);

    // 应用传感器偏移补偿
    for (int i = 0; i < 8; i++) {
        int32_t compensated = (int32_t)data[i] + sensor_offsets_[i];

        // 限幅保护（0-4095）
        if (compensated < 0) {
            compensated = 0;
        } else if (compensated > 4095) {
            compensated = 4095;
        }

        data[i] = (uint16_t)compensated;
    }
}

void LineSensor::medianFilter(uint16_t data[8]) {
    // 限制采样次数范围在1~5
    uint8_t samples = median_samples_;
    if (samples < 1) samples = 1;
    if (samples > 5) samples = 5;

    uint16_t temp[5][8];
    for (uint8_t i = 0; i < samples; i++) {
        ADC_ReadAll(temp[i]);
    }
    for (int i = 0; i < 8; i++) {
        uint16_t temp_data[5];
        for (uint8_t j = 0; j < samples; j++) {
            temp_data[j] = temp[j][i];
        }
        // 使用自定义排序
        bubbleSort(temp_data, samples);
        // 选择中位索引
        uint8_t mid = samples / 2;
        data[i] = temp_data[mid];
    }
    // Debug_Printf("[LineSensor] Median Filter: %d, %d, %d, %d, %d, %d, %d, %d\n", data[0],
    // data[1],
    //              data[2], data[3], data[4], data[5], data[6], data[7]);
}

/**
 * @brief 一阶IIR低通滤波器（指数移动平均）
 * @param data 输入/输出数据数组（8个传感器值）
 *
 * 算法原理：
 * 低通滤波器用于去除高频噪声，保留低频信号（传感器的实际读数）
 *
 * IIR滤波器公式：
 *   Y(n) = α * X(n) + (1-α) * Y(n-1)
 *
 * 其中：
 *   Y(n)   - 本次输出（滤波后的值）
 *   X(n)   - 本次输入（当前采样值）
 *   Y(n-1) - 上次输出（上次滤波结果）
 *   α      - 滤波系数（0 < α < 1）
 *
 * 滤波系数α的选择：
 *   α越大：响应速度快，但滤波效果弱（更接近原始值）
 *   α越小：滤波效果好，但响应速度慢（更平滑）
 *
 * 推荐值：
 *   α = 0.3 - 0.5  适合循迹传感器（平衡响应速度和滤波效果）
 *   α = 0.2        更平滑，适合低速运行
 *   α = 0.7        更快响应，适合高速运行
 *
 * 优点：
 *   1. 计算简单，效率高
 *   2. 内存占用小（只需存储上一次结果）
 *   3. 对ADC噪声有很好的抑制效果
 *   4. 没有相位延迟问题
 */
void LineSensor::lowPassFilter(uint16_t data[8]) {
    // 如果是第一次调用，直接使用当前值初始化
    if (!filter_initialized_) {
        for (int i = 0; i < 8; i++) {
            filtered_data_[i] = data[i];
        }
        filter_initialized_ = true;

        Debug_Printf("[LineSensor] 低通滤波器已初始化 (α=%.2f)\r\n",
                     (float)alpha_numerator_ / ALPHA_DENOMINATOR);
        return;  // 第一次不进行滤波，直接返回
    }

    // 对每个传感器应用IIR滤波
    for (int i = 0; i < 8; i++) {
        // 公式：Y(n) = α * X(n) + (1-α) * Y(n-1)
        //
        // 使用定点数运算（避免浮点运算，提高效率）：
        // Y(n) = (α * X(n) + (256-α) * Y(n-1)) / 256
        //
        // 拆解计算：
        //   part1 = α * X(n)
        //   part2 = (256-α) * Y(n-1)
        //   Y(n) = (part1 + part2) >> 8    // 除以256用右移8位代替

        uint32_t current_value = data[i];                // 当前采样值 X(n)
        uint32_t previous_filtered = filtered_data_[i];  // 上次滤波值 Y(n-1)

        // 计算：α * X(n)
        uint32_t weighted_current = alpha_numerator_ * current_value;

        // 计算：(1-α) * Y(n-1)
        uint32_t weighted_previous = (ALPHA_DENOMINATOR - alpha_numerator_) * previous_filtered;

        // 合并并除以256（右移8位）
        uint32_t filtered = (weighted_current + weighted_previous) >> 8;

        // 限幅保护（防止溢出）
        if (filtered > 4095) {  // ADC最大值是12位 = 4095
            filtered = 4095;
        }

        // 保存滤波结果
        filtered_data_[i] = (uint16_t)filtered;
        data[i] = (uint16_t)filtered;
    }

    // 调试输出（可选，注释掉以提高性能）
    // Debug_Printf("[LineSensor] 滤波后: %d, %d, %d, %d, %d, %d, %d, %d\r\n",
    //              data[0], data[1], data[2], data[3],
    //              data[4], data[5], data[6], data[7]);
}

void LineSensor::setThreshold(uint16_t black_line_threshold, uint16_t white_line_threshold) {
    // 为所有传感器设置相同的阈值
    for (int i = 0; i < 8; i++) {
        thresholds_[i] = (black_line_threshold + white_line_threshold) / 2;
    }
}

// ========== 滤波器控制接口实现 ==========

/**
 * @brief 设置低通滤波系数（浮点数方式）
 * @param alpha 滤波系数 (0.0 - 1.0)
 */
void LineSensor::setFilterAlpha(float alpha) {
    // 限制范围在 [0.0, 1.0]
    if (alpha < 0.0f) {
        alpha = 0.0f;
    } else if (alpha > 1.0f) {
        alpha = 1.0f;
    }

    // 转换为定点数：α * 256
    alpha_numerator_ = (uint16_t)(alpha * ALPHA_DENOMINATOR);

    Debug_Printf("[LineSensor] 滤波系数已设置: α=%.2f (%d/256)\r\n", alpha, alpha_numerator_);
}

/**
 * @brief 设置低通滤波系数（整数方式）
 * @param alpha_numerator α的分子 (0 - 256)
 */
void LineSensor::setFilterAlphaRaw(uint16_t alpha_numerator) {
    // 限制范围在 [0, 256]
    if (alpha_numerator > ALPHA_DENOMINATOR) {
        alpha_numerator = ALPHA_DENOMINATOR;
    }

    alpha_numerator_ = alpha_numerator;

    Debug_Printf("[LineSensor] 滤波系数已设置: α=%d/256 (%.2f)\r\n", alpha_numerator_,
                 (float)alpha_numerator_ / ALPHA_DENOMINATOR);
}

/**
 * @brief 获取当前滤波系数
 * @return 当前α值（浮点数）
 */
float LineSensor::getFilterAlpha() const { return (float)alpha_numerator_ / ALPHA_DENOMINATOR; }

/**
 * @brief 重置滤波器（清除历史数据）
 */
void LineSensor::resetFilter() {
    // 清零历史数据
    for (int i = 0; i < 8; i++) {
        filtered_data_[i] = 0;
    }

    // 标记为未初始化
    filter_initialized_ = false;

    Debug_Printf("[LineSensor] 滤波器已重置\r\n");
}

/**
 * @brief 检查滤波器是否已初始化
 * @return true-已初始化，false-未初始化
 */
bool LineSensor::isFilterInitialized() const { return filter_initialized_; }

/**
 * @brief 根据速度自动调整滤波系数
 * @param speed_mps 小车速度（米/秒）
 *
 * 速度越快，使用越大的α（响应快，滤波弱）
 * 速度越慢，使用越小的α（响应慢，滤波强）
 */
void LineSensor::setFilterBySpeed(float speed_mps) {
    uint16_t new_alpha;

    if (speed_mps < 0.3f) {
        // 低速：强滤波，确保数据稳定
        new_alpha = 77;  // α = 0.3
        Debug_Printf("[LineSensor] 低速模式: α=0.3\r\n");
    } else if (speed_mps < 0.6f) {
        // 中速：平衡滤波
        new_alpha = 102;  // α = 0.4
        Debug_Printf("[LineSensor] 中速模式: α=0.4\r\n");
    } else {
        // 高速：弱滤波，确保快速响应
        new_alpha = 179;  // α = 0.7
        Debug_Printf("[LineSensor] 高速模式: α=0.7\r\n");
    }

    alpha_numerator_ = new_alpha;
}

// ========== 校准功能实现 ==========

/**
 * @brief 白色校准
 * @note 采集当前传感器在白色区域的读数
 */
void LineSensor::calibrateWhite() {
    Debug_Printf("[LineSensor] 开始白色校准...\r\n");
    Debug_Printf("[LineSensor] 请将传感器放在白色区域上\r\n");
    // 延迟让用户看到提示
    HAL_Delay(2000);

    // 采集多次求平均值（提高精度）
    constexpr int SAMPLES = 10;
    uint32_t sum[8] = {0};

    for (int sample = 0; sample < SAMPLES; sample++) {
        uint16_t raw_data[8];
        ADC_ReadAll(raw_data);

        for (int i = 0; i < 8; i++) {
            sum[i] += raw_data[i];
        }

        HAL_Delay(50);  // 每次采样间隔50ms
    }

    // 计算平均值
    for (int i = 0; i < 8; i++) {
        white_calibration_[i] = sum[i] / SAMPLES;
    }

    Debug_Printf("[LineSensor] 白色校准完成: ");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%d ", white_calibration_[i]);
    }
    Debug_Printf("\r\n");
}

/**
 * @brief 黑色校准
 * @note 采集当前传感器在黑色线上的读数
 */
void LineSensor::calibrateBlack() {
    Debug_Printf("[LineSensor] 开始黑色校准...\r\n");
    Debug_Printf("[LineSensor] 请将传感器放在黑色线上\r\n");

    // 延迟让用户看到提示
    HAL_Delay(2000);

    // 采集多次求平均值
    constexpr int SAMPLES = 10;
    uint32_t sum[8] = {0};

    for (int sample = 0; sample < SAMPLES; sample++) {
        uint16_t raw_data[8];
        ADC_ReadAll(raw_data);

        for (int i = 0; i < 8; i++) {
            sum[i] += raw_data[i];
        }

        HAL_Delay(50);
    }

    // 计算平均值
    for (int i = 0; i < 8; i++) {
        black_calibration_[i] = sum[i] / SAMPLES;
    }

    Debug_Printf("[LineSensor] 黑色校准完成: ");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%d ", black_calibration_[i]);
    }
    Debug_Printf("\r\n");
}

/**
 * @brief 手动分步校准（推荐使用）
 * @param button 校准按钮引用
 * @note 等待按钮按下，分三步完成校准
 */
void LineSensor::autoCalibrate(Button& button) {
    Debug_Printf("\r\n╔══════════════════════════════════════════╗\r\n");
    Debug_Printf("║      传感器手动分步校准                  ║\r\n");
    Debug_Printf("╚══════════════════════════════════════════╝\r\n");
    
    /* ========== 等待按钮释放（避免长按触发后直接进入下一步） ========== */
    Debug_Printf("\r\n⏳ 请先释放按钮...\r\n");
    while (button.read()) {
        HAL_Delay(10);  // 等待按钮释放
    }
    Debug_Printf("✅ 按钮已释放\r\n");
    
    // 重置按钮状态，清除之前的触发标志
    button.reset();
    HAL_Delay(500);  // 给用户缓冲时间

    /* ========== 步骤1：白色校准 ========== */
    Debug_Printf("\r\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("📍 步骤 1/3：白色校准\r\n");
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("请将传感器放在【白色区域】上\r\n");
    Debug_Printf("准备好后，按下按钮开始采集...\r\n\r\n");

    // LED闪烁等待
    while (!button.isPressed()) {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    Debug_Printf("✅ 按钮已按下，开始采集白色值...\r\n");
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);  // LED常亮
    HAL_Delay(200);  // 防抖延迟

    calibrateWhite();

    Debug_Printf("✅ 白色校准完成！\r\n\r\n");
    HAL_Delay(500);

    /* ========== 步骤2：黑色校准 ========== */
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("📍 步骤 2/3：黑色校准\r\n");
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("请将传感器放在【黑色线】上\r\n");
    Debug_Printf("准备好后，按下按钮开始采集...\r\n\r\n");

    // LED闪烁等待
    while (!button.isPressed()) {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    Debug_Printf("✅ 按钮已按下，开始采集黑色值...\r\n");
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);  // LED常亮
    HAL_Delay(200);                                        // 防抖延迟

    calibrateBlack();

    Debug_Printf("✅ 黑色校准完成！\r\n\r\n");
    HAL_Delay(500);

    /* ========== 步骤3：计算阈值 ========== */
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("📍 步骤 3/3：计算阈值并保存\r\n");
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    Debug_Printf("按下按钮完成校准...\r\n\r\n");

    // LED快速闪烁等待
    while (!button.isPressed()) {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(50);
    }

    Debug_Printf("✅ 按钮已按下，开始计算阈值...\r\n");
    HAL_Delay(200);  // 防抖延迟

    // 计算平均值和阈值
    uint32_t white_avg = 0;
    uint32_t black_avg = 0;

    for (int i = 0; i < 8; i++) {
        white_avg += white_calibration_[i];
        black_avg += black_calibration_[i];
    }

    white_avg /= 8;
    black_avg /= 8;

    // 计算每个传感器的独立阈值（取白色和黑色校准值的平均值）
    for (int i = 0; i < 8; i++) {
        thresholds_[i] = (white_calibration_[i] + black_calibration_[i]) / 2;
    }

    // 显示校准结果
    Debug_Printf("\r\n传感器  白色值  黑色值  阈值\r\n");
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("  [%d]   %4d    %4d    %4d\r\n", i, white_calibration_[i], black_calibration_[i], thresholds_[i]);
    }
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n");

    Debug_Printf("\r\n[LineSensor] 白色平均值: %lu\r\n", white_avg);
    Debug_Printf("[LineSensor] 黑色平均值: %lu\r\n", black_avg);

    Debug_Printf("\r\n╔══════════════════════════════════════════╗\r\n");
    Debug_Printf("║      ✅ 校准完成！                       ║\r\n");
    Debug_Printf("╚══════════════════════════════════════════╝\r\n");
    Debug_Printf("提示：调用 saveCalibration() 保存到EEPROM\r\n\r\n");

    // LED熄灭
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

/**
 * @brief 从EEPROM加载校准数据
 * @param eeprom EEPROM对象引用
 * @return true 加载成功
 * @return false 加载失败（使用默认值）
 */
bool LineSensor::loadCalibration(EEPROM& eeprom) {
    Debug_Printf("[LineSensor] 正在从EEPROM加载校准数据...\r\n");

    SensorCalibration calib;

    // 从EEPROM读取校准数据（带CRC校验）
    if (eeprom.readStructCRC(CALIBRATION_EEPROM_ADDR, calib)) {
        // CRC校验通过，检查魔术数字
        if (calib.magic_number == CALIBRATION_MAGIC) {
            Debug_Printf("[LineSensor] 校准数据有效，应用配置\r\n");

            // 应用校准数据
            applyCalibration(calib);

            Debug_Printf("[LineSensor] 各传感器阈值已计算并应用\r\n");

            return true;
        } else {
            Debug_Printf("[LineSensor] 魔术数字不匹配，使用默认值\r\n");
        }
    } else {
        Debug_Printf("[LineSensor] CRC校验失败或数据未初始化，使用默认值\r\n");
    }

    // 使用默认阈值（所有传感器设置为相同值）
    uint16_t default_threshold = (1550 + 150) / 2;  // (黑线 + 白线) / 2
    for (int i = 0; i < 8; i++) {
        thresholds_[i] = default_threshold;
    }
    Debug_Printf("[LineSensor] 使用默认阈值: %d\r\n", default_threshold);

    return false;
}

/**
 * @brief 保存校准数据到EEPROM
 * @param eeprom EEPROM对象引用
 * @return true 保存成功
 * @return false 保存失败
 */
bool LineSensor::saveCalibration(EEPROM& eeprom) {
    Debug_Printf("[LineSensor] 正在保存校准数据到EEPROM...\r\n");

    SensorCalibration calib;

    // 获取当前校准数据
    getCalibration(calib);

    // 保存到EEPROM（带CRC校验）
    if (eeprom.writeStructCRC(CALIBRATION_EEPROM_ADDR, calib)) {
        Debug_Printf("[LineSensor] 校准数据保存成功！\r\n");
        Debug_Printf("[LineSensor] 地址: 0x%02X\r\n", CALIBRATION_EEPROM_ADDR);
        Debug_Printf("[LineSensor] 大小: %d 字节（含CRC）\r\n", sizeof(calib) + 1);
        return true;
    } else {
        Debug_Printf("[LineSensor] 校准数据保存失败！\r\n");
        return false;
    }
}

/**
 * @brief 获取校准数据
 * @param calib 校准数据结构体
 */
void LineSensor::getCalibration(SensorCalibration& calib) const {
    calib.magic_number = CALIBRATION_MAGIC;

    for (int i = 0; i < 8; i++) {
        calib.white_values[i] = white_calibration_[i];
        calib.black_values[i] = black_calibration_[i];
    }
}

/**
 * @brief 应用校准数据
 * @param calib 校准数据结构体
 */
void LineSensor::applyCalibration(const SensorCalibration& calib) {
    // 复制校准值
    for (int i = 0; i < 8; i++) {
        white_calibration_[i] = calib.white_values[i];
        black_calibration_[i] = calib.black_values[i];
    }

    // 重新计算阈值
    uint32_t white_avg = 0;
    uint32_t black_avg = 0;

    for (int i = 0; i < 8; i++) {
        white_avg += white_calibration_[i];
        black_avg += black_calibration_[i];
    }

    white_avg /= 8;
    black_avg /= 8;

    // 计算每个传感器的独立阈值（取白色和黑色校准值的平均值）
    for (int i = 0; i < 8; i++) {
        thresholds_[i] = (white_calibration_[i] + black_calibration_[i]) / 2;
    }
}

/* ========== 传感器补偿接口实现 ========== */

/**
 * @brief 设置传感器偏移补偿值
 * @param offsets 8个传感器的偏移值数组
 */
void LineSensor::setSensorOffsets(const int16_t offsets[8]) {
    for (int i = 0; i < 8; i++) {
        sensor_offsets_[i] = offsets[i];
    }

    Debug_Printf("[LineSensor] 传感器补偿已设置: ");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%+d ", sensor_offsets_[i]);
    }
    Debug_Printf("\r\n");
}

/**
 * @brief 清除传感器偏移补偿
 */
void LineSensor::clearSensorOffsets() {
    for (int i = 0; i < 8; i++) {
        sensor_offsets_[i] = 0;
    }
    Debug_Printf("[LineSensor] 传感器补偿已清除\r\n");
}

/**
 * @brief 获取当前的传感器偏移补偿值
 * @param offsets 输出数组（8个元素）
 */
void LineSensor::getSensorOffsets(int16_t offsets[8]) const {
    for (int i = 0; i < 8; i++) {
        offsets[i] = sensor_offsets_[i];
    }
}

/**
 * @brief 获取校准后的白色/黑色原始值
 * @param white_vals 输出白色校准值数组（8个元素）
 * @param black_vals 输出黑色校准值数组（8个元素）
 */
void LineSensor::getCalibrationValues(uint16_t white_vals[8], uint16_t black_vals[8]) const {
    for (int i = 0; i < 8; i++) {
        white_vals[i] = white_calibration_[i];
        black_vals[i] = black_calibration_[i];
    }
}

// ========== 线检测接口实现 ==========

/**
 * @brief 传感器权重（用于加权算法计算线位置）
 * 8个传感器从左到右的位置权重
 */
static constexpr float SENSOR_WEIGHTS[8] = {
    -1000.0f,  // 传感器0（最左）
    -714.3f,   // 传感器1
    -428.6f,   // 传感器2
    -142.9f,   // 传感器3
    +142.9f,   // 传感器4
    +428.6f,   // 传感器5
    +714.3f,   // 传感器6
    +1000.0f   // 传感器7（最右）
};

/**
 * @brief 获取二值化数据（黑白位图）
 */
void LineSensor::getBinaryData(bool binary_data[8], LineMode mode, uint16_t threshold) {
    // 读取传感器数据（物理顺序）
    uint16_t sensor_data[8];
    getData(sensor_data);

    // 将物理顺序映射为逻辑左→右
    for (int i = 0; i < 8; i++) {
        int src = reverse_order_ ? (7 - i) : i;
        uint16_t sensor_threshold;

        // 如果提供了全局阈值，使用它；否则使用物理索引对应的独立阈值
        if (threshold != 0) {
            sensor_threshold = threshold;
        } else {
            sensor_threshold = thresholds_[src];
        }

        if (mode == LineMode::WHITE_ON_BLACK) {
            // 黑底白线：高值表示白线
            binary_data[i] = (sensor_data[src] > sensor_threshold);
        } else {
            // 白底黑线：低值表示黑线
            binary_data[i] = (sensor_data[src] < sensor_threshold);
        }
    }
}

/**
 * @brief 计算线位置（加权算法）
 */
float LineSensor::getLinePosition(LineMode mode, uint16_t threshold) {
    uint16_t sensor_data[8];
    bool binary_data[8];
    return getLinePositionWithData(sensor_data, binary_data, mode, threshold);
}

/**
 * @brief 计算线位置并输出传感器数据和二值化数据
 */
float LineSensor::getLinePositionWithData(uint16_t sensor_data[8], bool binary_data[8], 
                                           LineMode mode, uint16_t threshold) {
    // 读取传感器数据（物理顺序）
    uint16_t phys_data[8];
    getData(phys_data);

    // 将物理顺序映射为逻辑左→右，同时输出映射后的原始数据（供显示）
    for (int i = 0; i < 8; i++) {
        int src = reverse_order_ ? (7 - i) : i;
        sensor_data[i] = phys_data[src];
    }

    // 二值化处理：对应物理索引选择各自阈值，但输出为逻辑顺序
    for (int i = 0; i < 8; i++) {
        int src = reverse_order_ ? (7 - i) : i;
        uint16_t sensor_threshold;
        if (threshold != 0) {
            sensor_threshold = threshold;
        } else {
            sensor_threshold = thresholds_[src];
        }
        if (mode == LineMode::WHITE_ON_BLACK) {
            binary_data[i] = (sensor_data[i] > sensor_threshold);
        } else {
            binary_data[i] = (sensor_data[i] < sensor_threshold);
        }
    }

    // 丢线快速判断：全白或全黑均视为丢线
    int detected_count = 0;
    for (int i = 0; i < 8; i++) {
        if (binary_data[i]) detected_count++;
    }
    if (detected_count == 0 || detected_count == 8) {
        return __builtin_nanf("");
    }
    
    // 改进的加权算法：使用模拟值实现亚像素级精度
    float weighted_sum = 0.0f;
    float total_weight = 0.0f;

    for (int i = 0; i < 8; i++) {
        // 计算每个传感器的"线强度"（0-1范围）
        float line_strength = 0.0f;

        if (mode == LineMode::WHITE_ON_BLACK) {
            // 黑底白线：传感器值越高，线强度越大
            int src = reverse_order_ ? (7 - i) : i;
            uint16_t sensor_threshold = (threshold != 0) ? threshold : thresholds_[src];
            if (sensor_data[i] > sensor_threshold) {
                // 归一化到0-1范围，避免除零
                uint16_t max_value = 4095;  // 12位ADC最大值
                line_strength = (float)(sensor_data[i] - sensor_threshold) / (float)(max_value - sensor_threshold);
                line_strength = fminf(line_strength, 1.0f);  // 限制在0-1
            }
        } else {
            // 白底黑线：传感器值越低，线强度越大（BLACK_ON_WHITE模式）
            int src = reverse_order_ ? (7 - i) : i;
            uint16_t sensor_threshold = (threshold != 0) ? threshold : thresholds_[src];
            // 阈值保护：为0时跳过，避免除0
            if (sensor_threshold > 0 && sensor_data[i] < sensor_threshold) {
                line_strength = (float)(sensor_threshold - sensor_data[i]) / (float)sensor_threshold;
                line_strength = fminf(line_strength, 1.0f);  // 限制在0-1
            }
        }

        // 使用线强度作为权重，实现亚像素级精度
        if (line_strength > 0.01f) {  // 过滤掉噪声
            weighted_sum += SENSOR_WEIGHTS[i] * line_strength;
            total_weight += line_strength;
        }
    }

    // 如果有传感器检测到线，计算加权平均位置
    if (total_weight > 0.0f) {
        float position = weighted_sum / total_weight;

        // 强化位置限制，防止异常值
        if (position > 1000.0f) position = 1000.0f;
        if (position < -1000.0f) position = -1000.0f;

        // 添加异常检测：如果权值分布异常，返回丢线
        if (total_weight < 0.1f || total_weight > 8.0f) {
            return __builtin_nanf("");  // 异常情况，返回丢线
        }

        return position;
    }

    // 丢线时返回NAN
    return __builtin_nanf("");  // GCC内置函数生成NaN
}

/**
 * @brief 检查是否检测到线
 */
bool LineSensor::isLineDetected(int min_sensors, LineMode mode, uint16_t threshold) {
    bool binary_data[8];
    getBinaryData(binary_data, mode, threshold);
    
    int detected_count = 0;
    for (int i = 0; i < 8; i++) {
        if (binary_data[i]) {
            detected_count++;
        }
    }
    
    return detected_count >= min_sensors;
}

void LineSensor::setMedianSamples(uint8_t samples) {
    if (samples < 1) samples = 1;
    if (samples > 5) samples = 5;
    median_samples_ = samples;
    Debug_Printf("[LineSensor] 中值采样次数=%d\r\n", median_samples_);
}
