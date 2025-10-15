/**
 * @file    line_sensor.cpp
 * @brief   8路灰度传感器类实现
 * @author  AI Assistant
 * @date    2024
 */

#include "line_sensor.hpp"
#include "adc.h"
#include <algorithm>

// 位置权重数组（从左到右：-1000 到 +1000）
const int16_t LineSensor::POSITION_WEIGHTS[NUM_SENSORS] = {
    -1000,  // [0] 最左侧
    -700,   // [1] 左2
    -400,   // [2] 左3
    -150,   // [3] 左4/中左
    +150,   // [4] 右4/中右
    +400,   // [5] 右3
    +700,   // [6] 右2
    +1000   // [7] 最右侧
};

/**
 * @brief 构造函数
 */
LineSensor::LineSensor()
    : threshold_(DEFAULT_THRESHOLD)
    , whiteValue_(0)
    , blackValue_(4095)
    , initialized_(false)
    , lineMode_(LineMode::BLACK_ON_WHITE)  // 默认白底黑线模式
{
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        rawValues_[i] = 0;
    }
}

/**
 * @brief 析构函数
 */
LineSensor::~LineSensor()
{
}

/**
 * @brief 初始化传感器
 */
void LineSensor::init()
{
    // 初始化 ADC1（8通道，DMA模式）
    MX_ADC1_Init();
    
    // 首次读取
    update();
    
    initialized_ = true;
}

/**
 * @brief 更新传感器读数
 */
void LineSensor::update()
{
    // 使用 DMA 方式读取所有 8 路传感器
    ADC_ReadAll(rawValues_);
}

/**
 * @brief 获取原始 ADC 值
 */
uint16_t LineSensor::getRawValue(uint8_t index) const
{
    if (index >= NUM_SENSORS) return 0;
    return rawValues_[index];
}

/**
 * @brief 检测是否为黑色（实际是检测目标线）
 * @note 在白底黑线模式下，检测黑色
 *       在黑底白线模式下，检测白色
 */
bool LineSensor::isBlack(uint8_t index) const
{
    if (index >= NUM_SENSORS) return false;
    
    if (lineMode_ == LineMode::WHITE_ON_BLACK) {
        // 黑底白线模式：低电压 = 白线 = 目标线
        return rawValues_[index] < threshold_;
    } else {
        // 白底黑线模式：高电压 = 黑线 = 目标线
        return rawValues_[index] > threshold_;
    }
}

/**
 * @brief 获取黑白状态字节
 */
uint8_t LineSensor::getBlackPattern() const
{
    uint8_t pattern = 0;
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (isBlack(i)) {
            pattern |= (1 << i);
        }
    }
    return pattern;
}

/**
 * @brief 计算线条位置（加权平均法）
 */
int16_t LineSensor::getPosition() const
{
    int32_t weightSum = 0;
    int32_t sensorSum = 0;
    
    // 加权求和
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (isBlack(i)) {
            weightSum += POSITION_WEIGHTS[i];
            sensorSum++;
        }
    }
    
    // 如果没有检测到线，返回特殊值
    if (sensorSum == 0) {
        return INT16_MIN;
    }
    
    // 计算加权平均位置
    return (int16_t)(weightSum / sensorSum);
}

/**
 * @brief 检测是否在线上
 */
bool LineSensor::isOnLine() const
{
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (isBlack(i)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 检测是否是十字路口
 */
bool LineSensor::isCrossroad() const
{
    // 至少 6 个传感器检测到黑线
    uint8_t blackCount = 0;
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (isBlack(i)) {
            blackCount++;
        }
    }
    return blackCount >= 6;
}

/**
 * @brief 检测是否完全丢线
 */
bool LineSensor::isLost() const
{
    return !isOnLine();
}

/**
 * @brief 设置黑白判断阈值
 */
void LineSensor::setThreshold(uint16_t threshold)
{
    threshold_ = threshold;
}

/**
 * @brief 自动校准阈值（白色地面）
 */
void LineSensor::calibrateWhite()
{
    update();
    
    // 取所有传感器的平均值
    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        sum += rawValues_[i];
    }
    whiteValue_ = sum / NUM_SENSORS;
}

/**
 * @brief 自动校准阈值（黑色线条）
 */
void LineSensor::calibrateBlack()
{
    update();
    
    // 取所有传感器的平均值
    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        sum += rawValues_[i];
    }
    blackValue_ = sum / NUM_SENSORS;
}

/**
 * @brief 完成校准（计算最终阈值）
 */
void LineSensor::finishCalibration()
{
    // 阈值取中间值
    threshold_ = (whiteValue_ + blackValue_) / 2;
}

/**
 * @brief 设置巡线模式
 */
void LineSensor::setLineMode(LineMode mode)
{
    lineMode_ = mode;
}
