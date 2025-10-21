#include "adc.h"
#include "button.hpp"
#include "common.h"
#include "debug.hpp"
#include "gpio.h"
#include "line_sensor.hpp"
#include "stm32f1xx_hal.h"
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
    uint16_t temp[5][8];
    for (int i = 0; i < 5; i++) {
        ADC_ReadAll(temp[i]);
    }
    for (int i = 0; i < 8; i++) {
        uint16_t temp_data[5];
        for (int j = 0; j < 5; j++) {
            temp_data[j] = temp[j][i];
        }
        // 使用自定义排序代替std::sort
        bubbleSort(temp_data, 5);
        data[i] = temp_data[2];  // 中值是排序后的第3个元素
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
    black_line_threszhold_ = black_line_threshold;
    white_line_threszhold_ = white_line_threshold;
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

    // 黑线阈值：取黑白值的60%位置（偏向黑色）
    // 白线阈值：取黑白值的40%位置（偏向白色）
    black_line_threszhold_ = white_avg + (black_avg - white_avg) * 6 / 10;
    white_line_threszhold_ = white_avg + (black_avg - white_avg) * 4 / 10;

    // 显示校准结果
    Debug_Printf("\r\n传感器  白色值  黑色值\r\n");
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━\r\n");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("  [%d]   %4d    %4d\r\n", i, white_calibration_[i], black_calibration_[i]);
    }
    Debug_Printf("━━━━━━━━━━━━━━━━━━━━━━\r\n");

    Debug_Printf("\r\n[LineSensor] 白色平均值: %lu\r\n", white_avg);
    Debug_Printf("[LineSensor] 黑色平均值: %lu\r\n", black_avg);
    Debug_Printf("[LineSensor] 黑线阈值: %d\r\n", black_line_threszhold_);
    Debug_Printf("[LineSensor] 白线阈值: %d\r\n", white_line_threszhold_);

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

            Debug_Printf("[LineSensor] 黑线阈值: %d\r\n", black_line_threszhold_);
            Debug_Printf("[LineSensor] 白线阈值: %d\r\n", white_line_threszhold_);

            return true;
        } else {
            Debug_Printf("[LineSensor] 魔术数字不匹配，使用默认值\r\n");
        }
    } else {
        Debug_Printf("[LineSensor] CRC校验失败或数据未初始化，使用默认值\r\n");
    }

    // 使用默认阈值
    Debug_Printf("[LineSensor] 使用默认阈值: 黑线=%d, 白线=%d\r\n", black_line_threszhold_,
                 white_line_threszhold_);

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

    // 计算阈值
    black_line_threszhold_ = white_avg + (black_avg - white_avg) * 6 / 10;
    white_line_threszhold_ = white_avg + (black_avg - white_avg) * 4 / 10;
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
