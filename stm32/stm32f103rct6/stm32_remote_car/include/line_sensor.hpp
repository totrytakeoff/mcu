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

/**
 * @class LineSensor
 * @brief 灰度传感器管理类
 */


class LineSensor {
public:
    LineSensor();

    enum class Mode { WHITHE_LINE, BLACK_LINE, UNKNOWN };

    void setMode(Mode mode);

    void getRawData(uint16_t data[8]);

    void getData(uint16_t data[8]);

    void medianFilter(uint16_t data[8]);

    void lowPassFilter(uint16_t data[8]);

    void setThreshold(uint16_t black_line_threshold = 1550, uint16_t white_line_threshold = 150);

    void calibrateWhite();  // 白值校准
    void calibrateBlack();  // 黑值校准
    void autoCalibrate();   // 自动校准
    
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
    Mode mode_;
    
    // 低通滤波器历史数据（用于存储上一次的滤波结果）
    uint16_t filtered_data_[8] = {0};
    bool filter_initialized_ = false;  // 滤波器是否已初始化
    
    // 低通滤波器系数（α值）
    // α = alpha_numerator_ / 256
    // 默认值: 102/256 ≈ 0.4 (平衡响应速度和滤波效果)
    uint16_t alpha_numerator_ = 102;
    
    // 固定分母（使用256便于位移优化）
    static constexpr uint16_t ALPHA_DENOMINATOR = 256;
};

#endif /* __LINE_SENSOR_HPP */
