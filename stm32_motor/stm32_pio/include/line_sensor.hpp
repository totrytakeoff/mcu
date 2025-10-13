/**
 * @file    line_sensor.hpp
 * @brief   8路灰度传感器类 - 黑白线检测
 * @author  AI Assistant
 * @date    2024
 * 
 * @usage   LineSensor sensor;
 *          sensor.init();
 *          sensor.update();
 *          int position = sensor.getPosition();
 */

#ifndef __LINE_SENSOR_HPP
#define __LINE_SENSOR_HPP

#include "stm32f1xx_hal.h"
#include <stdint.h>

/**
 * @class LineSensor
 * @brief 灰度传感器管理类
 */
class LineSensor
{
public:
    /* 传感器数量 */
    static constexpr uint8_t NUM_SENSORS = 8;
    
    /* 默认黑白阈值（ADC值，0-4095） */
    static constexpr uint16_t DEFAULT_THRESHOLD = 2000;
    
    /**
     * @brief 构造函数
     */
    LineSensor();
    
    /**
     * @brief 析构函数
     */
    ~LineSensor();
    
    /**
     * @brief 初始化传感器（ADC + DMA）
     */
    void init();
    
    /**
     * @brief 更新传感器读数（调用一次读取所有传感器）
     */
    void update();
    
    /**
     * @brief 获取原始 ADC 值
     * @param index: 传感器索引（0=最左, 7=最右）
     * @return ADC 值（0-4095）
     */
    uint16_t getRawValue(uint8_t index) const;
    
    /**
     * @brief 获取所有原始值的指针
     * @return 指向 8 个元素的数组
     */
    const uint16_t* getRawValues() const { return rawValues_; }
    
    /**
     * @brief 检测是否为黑色
     * @param index: 传感器索引
     * @return true=黑色, false=白色
     */
    bool isBlack(uint8_t index) const;
    
    /**
     * @brief 获取黑白状态字节（位图）
     * @return 8位数据，bit0=最左侧传感器
     * @example 0b00011000 = 中间两个检测到黑线
     */
    uint8_t getBlackPattern() const;
    
    /**
     * @brief 计算线条位置（加权平均法）
     * @return 位置值 (-1000 ~ +1000)
     *         -1000 = 线在最左侧
     *             0 = 线在中间
     *         +1000 = 线在最右侧
     *         INT16_MIN = 未检测到线
     */
    int16_t getPosition() const;
    
    /**
     * @brief 检测是否在线上
     * @return true=至少一个传感器检测到黑线
     */
    bool isOnLine() const;
    
    /**
     * @brief 检测是否是十字路口（全黑）
     * @return true=所有传感器都检测到黑线
     */
    bool isCrossroad() const;
    
    /**
     * @brief 检测是否完全丢线（全白）
     * @return true=所有传感器都检测到白色
     */
    bool isLost() const;
    
    /**
     * @brief 设置黑白判断阈值
     * @param threshold: ADC阈值（0-4095）
     */
    void setThreshold(uint16_t threshold);
    
    /**
     * @brief 自动校准阈值（白色地面）
     * @note 将传感器放在白色地面上调用此函数
     */
    void calibrateWhite();
    
    /**
     * @brief 自动校准阈值（黑色线条）
     * @note 将传感器放在黑色线条上调用此函数
     */
    void calibrateBlack();
    
    /**
     * @brief 完成校准（计算最终阈值）
     */
    void finishCalibration();
    
    /**
     * @brief 获取当前阈值
     */
    uint16_t getThreshold() const { return threshold_; }
    
private:
    uint16_t rawValues_[NUM_SENSORS];     // 原始 ADC 值
    uint16_t threshold_;                  // 黑白判断阈值
    uint16_t whiteValue_;                 // 白色校准值
    uint16_t blackValue_;                 // 黑色校准值
    bool initialized_;                    // 初始化标志
    
    // 位置权重（左侧为负，右侧为正）
    static const int16_t POSITION_WEIGHTS[NUM_SENSORS];
};

#endif /* __LINE_SENSOR_HPP */
