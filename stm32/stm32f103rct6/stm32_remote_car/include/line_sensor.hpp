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

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "eeprom.hpp"







/**
 * @brief 传感器校准数据结构体
 * 
 * 保存黑白线的阈值到EEPROM
 * 总共占用 4 + 8*2 + 8*2 + 1 = 37字节
 */
struct __attribute__((packed)) SensorCalibration {
    uint32_t magic_number;          ///< 魔术数字 0xCAFEBABE（用于验证数据有效性）
    uint16_t white_values[8];       ///< 白色校准值（每个传感器）
    uint16_t black_values[8];       ///< 黑色校准值（每个传感器）
    // CRC会自动添加在writeStructCRC时
};

/**
 * @class LineSensor
 * @brief 灰度传感器管理类
 */


class LineSensor {
public:
    LineSensor();

    void getRawData(uint16_t data[8]);

    void getData(uint16_t data[8]);

    void medianFilter(uint16_t data[8]);

    void lowPassFilter(uint16_t data[8]);

    void setThreshold(uint16_t black_line_threshold = 1550, uint16_t white_line_threshold = 150);

    // ========== 校准相关接口 ==========
    
    /**
     * @brief 白色校准
     * @note 将传感器放在白色区域上，调用此函数采集白色值
     */
    void calibrateWhite();
    
    /**
     * @brief 黑色校准
     * @note 将传感器放在黑色线上，调用此函数采集黑色值
     */
    void calibrateBlack();
    
    /**
     * @brief 手动分步校准（推荐）
     * @param button 校准按钮引用
     * @note 分三步进行：
     *       1. 按下按钮 → 白色校准
     *       2. 按下按钮 → 黑色校准
     *       3. 按下按钮 → 计算阈值
     */
    void autoCalibrate(class Button& button);
    
    /**
     * @brief 从EEPROM加载校准数据
     * @param eeprom EEPROM对象引用
     * @return true 加载成功
     * @return false 加载失败（使用默认值）
     */
    bool loadCalibration(EEPROM& eeprom);
    
    /**
     * @brief 保存校准数据到EEPROM
     * @param eeprom EEPROM对象引用
     * @return true 保存成功
     * @return false 保存失败
     */
    bool saveCalibration(EEPROM& eeprom);
    
    /**
     * @brief 获取校准数据
     * @param calib 校准数据结构体
     */
    void getCalibration(SensorCalibration& calib) const;
    
    /**
     * @brief 应用校准数据
     * @param calib 校准数据结构体
     */
    void applyCalibration(const SensorCalibration& calib);
    
    // ========== 传感器补偿接口 ==========
    
    /**
     * @brief 设置传感器偏移补偿值
     * @param offsets 8个传感器的偏移值数组（正值表示增加，负值表示减少）
     * @note 用于补偿硬件差异，例如：某个传感器读数偏低120，可设置offsets[i]=120
     * @example
     *   int16_t offsets[8] = {0, 120, 0, 0, 0, 0, 0, 0};  // 2号传感器补偿+120
     *   sensor.setSensorOffsets(offsets);
     */
    void setSensorOffsets(const int16_t offsets[8]);
    
    /**
     * @brief 清除传感器偏移补偿（恢复为0）
     */
    void clearSensorOffsets();
    
    /**
     * @brief 获取当前的传感器偏移补偿值
     * @param offsets 输出数组（8个元素）
     */
    void getSensorOffsets(int16_t offsets[8]) const;
    
    /**
     * @brief 获取校准后的白色/黑色原始值（用于算法处理）
     * @param white_vals 输出白色校准值数组（8个元素）
     * @param black_vals 输出黑色校准值数组（8个元素）
     */
    void getCalibrationValues(uint16_t white_vals[8], uint16_t black_vals[8]) const;
    
    // ========== 滤波器控制接口 ==========
    
    /**
     * @brief 设置低通滤波系数（浮点数方式）
     * @param alpha 滤波系数 (0.0 - 1.0)
     *              0.0 = 最平滑（慢响应）
     *              1.0 = 无滤波（快响应）
     *              推荐值: 0.3 - 0.5
     */
    void setFilterAlpha(float alpha);
    
    /**
     * @brief 设置低通滤波系数（整数方式）
     * @param alpha_numerator α的分子 (0 - 256)
     *                        77  = α ≈ 0.3 (平滑)
     *                        102 = α ≈ 0.4 (推荐)
     *                        128 = α ≈ 0.5 (适中)
     *                        179 = α ≈ 0.7 (快速)
     */
    void setFilterAlphaRaw(uint16_t alpha_numerator);
    
    /**
     * @brief 获取当前滤波系数
     * @return 当前α值（浮点数）
     */
    float getFilterAlpha() const;
    
    /**
     * @brief 重置滤波器（清除历史数据）
     */
    void resetFilter();
    
    /**
     * @brief 检查滤波器是否已初始化
     * @return true-已初始化，false-未初始化
     */
    bool isFilterInitialized() const;
    
    /**
     * @brief 根据速度自动调整滤波系数
     * @param speed_mps 小车速度（米/秒）
     *                  < 0.3 m/s: 使用强滤波 (α=0.3)
     *                  0.3-0.6:   使用中等滤波 (α=0.4)
     *                  > 0.6:     使用弱滤波 (α=0.7)
     */
    void setFilterBySpeed(float speed_mps);

private:
    uint16_t black_line_threszhold_ = 1550;
    uint16_t white_line_threszhold_ = 150;
    
    // 校准数据
    uint16_t white_calibration_[8] = {0};  ///< 白色校准值
    uint16_t black_calibration_[8] = {0};  ///< 黑色校准值
    
    // 传感器偏移补偿
    int16_t sensor_offsets_[8] = {0};      ///< 传感器偏移补偿值
    
    // 低通滤波器历史数据（用于存储上一次的滤波结果）
    uint16_t filtered_data_[8] = {0};
    bool filter_initialized_ = false;  // 滤波器是否已初始化
    
    // 低通滤波器系数（α值）
    // α = alpha_numerator_ / 256
    // 默认值: 102/256 ≈ 0.4 (平衡响应速度和滤波效果)
    uint16_t alpha_numerator_ = 102;
    
    // 固定分母（使用256便于位移优化）
    static constexpr uint16_t ALPHA_DENOMINATOR = 256;
    
    // EEPROM地址常量
    static constexpr uint8_t CALIBRATION_EEPROM_ADDR = 0x40;  ///< 校准数据存储地址
    static constexpr uint32_t CALIBRATION_MAGIC = 0xCAFEBABE; ///< 校准数据魔术数字
};

#endif /* __LINE_SENSOR_HPP */
