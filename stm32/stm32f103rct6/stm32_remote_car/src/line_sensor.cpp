#include "adc.h"
#include "debug.hpp"
#include "line_sensor.hpp"

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

void LineSensor::setMode(Mode mode) { mode_ = mode; }

void LineSensor::getRawData(uint16_t data[8]) {
    ADC_ReadAll(data);
    Debug_Printf("[LineSensor] Raw Data: %d, %d, %d, %d, %d, %d, %d, %d\n", data[0], data[1],
                 data[2], data[3], data[4], data[5], data[6], data[7]);
}

void LineSensor::getData(uint16_t data[8]) {
    medianFilter(data);
    lowPassFilter(data);
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
    // Debug_Printf("[LineSensor] Median Filter: %d, %d, %d, %d, %d, %d, %d, %d\n", data[0], data[1],
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
        
        uint32_t current_value = data[i];                    // 当前采样值 X(n)
        uint32_t previous_filtered = filtered_data_[i];      // 上次滤波值 Y(n-1)
        
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
    
    Debug_Printf("[LineSensor] 滤波系数已设置: α=%.2f (%d/256)\r\n", 
                 alpha, alpha_numerator_);
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
    
    Debug_Printf("[LineSensor] 滤波系数已设置: α=%d/256 (%.2f)\r\n", 
                 alpha_numerator_, (float)alpha_numerator_ / ALPHA_DENOMINATOR);
}

/**
 * @brief 获取当前滤波系数
 * @return 当前α值（浮点数）
 */
float LineSensor::getFilterAlpha() const {
    return (float)alpha_numerator_ / ALPHA_DENOMINATOR;
}

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
bool LineSensor::isFilterInitialized() const {
    return filter_initialized_;
}

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
        new_alpha = 77;   // α = 0.3
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
