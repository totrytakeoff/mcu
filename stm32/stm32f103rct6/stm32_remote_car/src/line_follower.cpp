/**
 * @file    line_follower.cpp
 * @brief   抛物线拟合法巡线控制器实现
 * @author  AI Assistant
 * @date    2024
 */

#include "line_follower.hpp"
#include "debug.hpp"
#include <cmath>

// 定义静态常量
constexpr float LineFollower::SENSOR_POSITIONS[8];

/* ========== 构造函数 ========== */

LineFollower::LineFollower(LineSensor& sensor, DriveTrain& drive)
    : sensor_(sensor), drive_(drive) {}

/* ========== 配置接口 ========== */

void LineFollower::setPID(float kp, float ki, float kd) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    Debug_Printf("[LineFollower] PID设置: Kp=%.3f, Ki=%.3f, Kd=%.3f\r\n", kp, ki, kd);
}

void LineFollower::setSpeed(int16_t base_speed) {
    if (base_speed < 0) base_speed = 0;
    if (base_speed > 100) base_speed = 100;
    base_speed_ = base_speed;
    Debug_Printf("[LineFollower] 基础速度设置: %d%%\r\n", base_speed);
}

void LineFollower::setLineMode(LineMode mode) {
    line_mode_ = mode;
    Debug_Printf("[LineFollower] 线模式设置: %s\r\n",
                 mode == LineMode::WHITE_LINE_ON_BLACK ? "黑底白线" : "白底黑线");
}

void LineFollower::enableDebug(bool enable) {
    debug_enabled_ = enable;
}

void LineFollower::setPIDOutputScale(float scale) {
    if (scale < 0.01f) scale = 0.01f;
    if (scale > 1.0f) scale = 1.0f;
    pid_output_scale_ = scale;
    Debug_Printf("[LineFollower] PID输出缩放设置: 固定%.3f\r\n", scale);
}

void LineFollower::enableDynamicScale(bool enable) {
    dynamic_scale_enabled_ = enable;
    Debug_Printf("[LineFollower] 动态缩放: %s\r\n", enable ? "启用" : "禁用");
}

void LineFollower::setDynamicScaleRange(float small_scale, float large_scale) {
    if (small_scale < 0.01f) small_scale = 0.01f;
    if (small_scale > 1.0f) small_scale = 1.0f;
    if (large_scale < 0.01f) large_scale = 0.01f;
    if (large_scale > 1.0f) large_scale = 1.0f;
    
    small_error_scale_ = small_scale;
    large_error_scale_ = large_scale;
    
    Debug_Printf("[LineFollower] 动态缩放范围: 小偏差%.3f, 大偏差%.3f\r\n", 
                 small_scale, large_scale);
}

void LineFollower::setScaleCurve(ScaleCurve curve) {
    scale_curve_ = curve;
    const char* curve_name[] = {"线性", "二次方", "平方根", "三次方"};
    Debug_Printf("[LineFollower] 缩放曲线: %s\r\n", 
                 curve_name[static_cast<int>(curve)]);
}

void LineFollower::setLostLineSearchParams(int16_t search_speed, int16_t max_turn_speed) {
    if (search_speed < 5) search_speed = 5;
    if (search_speed > 20) search_speed = 20;
    if (max_turn_speed < 0) max_turn_speed = 0;
    if (max_turn_speed > 20) max_turn_speed = 20;
    
    lost_line_search_speed_ = search_speed;
    lost_line_max_turn_speed_ = max_turn_speed;
    
    Debug_Printf("[LineFollower] 丢线搜索: 速度=%d, 最大转向=%d\r\n", 
                 search_speed, max_turn_speed);
}

/* ========== 控制接口 ========== */

void LineFollower::start() {
    running_ = true;
    status_ = Status::ON_LINE;
    resetPID();
    last_update_time_ = HAL_GetTick();
    Debug_Printf("[LineFollower] 启动巡线\r\n");
}

void LineFollower::stop() {
    running_ = false;
    status_ = Status::STOPPED;
    drive_.stop();
    Debug_Printf("[LineFollower] 停止巡线\r\n");
}

void LineFollower::update() {
    if (!running_) {
        return;
    }

    uint32_t now = HAL_GetTick();
    uint32_t dt = now - last_update_time_;
    if (dt == 0) dt = 1;  // 避免除0
    last_update_time_ = now;

    // 1. 读取传感器数据（已经过滤波和补偿）
    uint16_t sensor_data[8];
    sensor_.getData(sensor_data);

    // 2. 检测状态
    status_ = detectStatus(sensor_data);

    // 3. 根据状态执行相应动作
    if (status_ == Status::LOST_LINE) {
        handleLostLine();
        return;
    }

    // 4. 计算线位置（抛物线拟合法）
    position_ = calculateLinePositionParabolic(sensor_data);
    last_valid_position_ = position_;

    // 5. PID计算
    float error = position_;  // 位置偏差即为误差
    float raw_pid_output = computePID(error, dt);
    
    // 6. 应用完全动态PID输出缩放（关键：控制转向强度）
    float current_scale;
    if (dynamic_scale_enabled_) {
        // 动态缩放算法：根据位置偏差连续平滑地调整转向强度
        float abs_error = fabsf(error);
        
        // 归一化误差到 0.0-1.0 范围
        float normalized_error = abs_error / 1000.0f;
        if (normalized_error > 1.0f) normalized_error = 1.0f;
        
        // 根据曲线类型计算缩放系数
        float curve_factor;
        switch (scale_curve_) {
            case ScaleCurve::LINEAR:
                // 线性：均匀响应（推荐）
                // error=0 → scale=0.03, error=1000 → scale=0.15
                curve_factor = normalized_error;
                break;
                
            case ScaleCurve::QUADRATIC:
                // 二次方：小偏差温和，大偏差激进
                // 适合需要精细微调但大偏差快速回正的场景
                curve_factor = normalized_error * normalized_error;
                break;
                
            case ScaleCurve::SQRT:
                // 平方根：小偏差激进，大偏差温和
                // 适合需要快速响应但避免过冲的场景
                curve_factor = sqrtf(normalized_error);
                break;
                
            case ScaleCurve::CUBIC:
                // 三次方：更激进的响应曲线
                // 小偏差非常温和，大偏差非常激进
                curve_factor = normalized_error * normalized_error * normalized_error;
                break;
                
            default:
                curve_factor = normalized_error;
        }
        
        // 计算最终缩放系数
        current_scale = small_error_scale_ + 
                       (large_error_scale_ - small_error_scale_) * curve_factor;
    } else {
        // 固定缩放
        current_scale = pid_output_scale_;
    }
    
    pid_output_ = raw_pid_output * current_scale;

    // 7. 应用到差速系统
    // position > 0 表示线在右侧，需要右转（减小右轮速度）
    // 注意：DriveTrain的turnSpeed，正值为左转，负值为右转
    // 使用实时驱动，绕过梯形加速，保证巡线转向毫秒级响应
    // 四舍五入到最近的整数，避免小偏差被截断为0
    int16_t turn_cmd = (int16_t)((pid_output_ >= 0.0f) ? (pid_output_ + 0.5f) : (pid_output_ - 0.5f));
    drive_.driveImmediate(base_speed_, -turn_cmd);

    // 8. 调试输出
    if (debug_enabled_) {
        // 转换为整数输出（避免浮点格式化问题）
        int16_t pos_int = (int16_t)position_;
        int16_t err_int = (int16_t)error;
        int16_t raw_pid_int = (int16_t)raw_pid_output;
        int16_t pid_int = (int16_t)pid_output_;
        int16_t scale_int = (int16_t)(current_scale * 100);  // 转换为百分比
        
        // 归一化值转换为百分比（0~100）
        int16_t norm_int[8];
        for (int i = 0; i < 8; i++) {
            norm_int[i] = (int16_t)(normalized_values_[i] * 100.0f);
        }
        
        Debug_Printf("Pos:%d Err:%d RawPID:%d Scale:%d PID:%d Spd:%d\r\n",
                     pos_int, err_int, raw_pid_int, scale_int, pid_int, base_speed_);
        Debug_Printf("  RAW: %d %d %d %d %d %d %d %d\r\n",
                     sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3],
                     sensor_data[4], sensor_data[5], sensor_data[6], sensor_data[7]);
        Debug_Printf("  NOR: %d %d %d %d %d %d %d %d\r\n\r\n",
                     norm_int[0], norm_int[1], norm_int[2], norm_int[3],
                     norm_int[4], norm_int[5], norm_int[6], norm_int[7]);
    }
}

/* ========== 核心算法实现 ========== */

/**
 * @brief 抛物线拟合法计算线位置（核心算法）
 * 
 * 算法原理：
 * 1. 使用校准数据将原始ADC值归一化为0.0~1.0的比例值
 * 2. 根据线模式处理数据（白线需要反转）
 * 3. 找到峰值（最大值点）及其索引
 * 4. 使用峰值及其左右相邻点拟合抛物线
 * 5. 计算抛物线顶点位置，得到亚传感器级别的精确位置
 * 
 * 归一化公式：
 * normalized = (raw - black) / (white - black)
 * 其中：black = 黑色校准值，white = 白色校准值
 * 
 * 三点抛物线拟合公式：
 * 对于等间距的三点 (x0,y0), (x1,y1), (x2,y2)，其中 x0=-1, x1=0, x2=1
 * 抛物线 y = ax² + bx + c
 * 顶点位置 x_vertex = (y0 - y2) / (2*(y0 - 2*y1 + y2))
 */
float LineFollower::calculateLinePositionParabolic(uint16_t sensor_data[8]) {
    // 1. 获取校准数据
    uint16_t white_cal[8];
    uint16_t black_cal[8];
    sensor_.getCalibrationValues(white_cal, black_cal);
    
    // 2. 数据预处理：归一化 + 根据线模式处理
    float values[8];
    
    for (int i = 0; i < 8; i++) {
        // 归一化到 0.0~1.0
        // 注意：你的传感器是"黑高白低"（黑色读数高，白色读数低）
        float range = (float)(black_cal[i] - white_cal[i]);  // 黑 - 白（正值）
        float normalized;
        
        if (range > 1.0f) {
            // 正常校准范围
            // 归一化：黑色→1.0，白色→0.0
            normalized = (float)(sensor_data[i] - white_cal[i]) / range;
            // 限幅到 [0, 1]
            if (normalized < 0.0f) normalized = 0.0f;
            if (normalized > 1.0f) normalized = 1.0f;
        } else {
            // 校准数据无效，退化为原始值归一化（假设黑高白低）
            normalized = 1.0f - (float)sensor_data[i] / ADC_MAX;
        }
        
        // 保存归一化值（用于调试输出）
        normalized_values_[i] = normalized;
        
        if (line_mode_ == LineMode::WHITE_LINE_ON_BLACK) {
            // 黑底白线：反转比例值（让白线变成峰值）
            // normalized: 黑色=1.0, 白色=0.0
            // values: 黑色=0.0, 白线=1.0（峰值）
            values[i] = 1.0f - normalized;
        } else {
            // 白底黑线：直接使用归一化值
            values[i] = normalized;
        }
    }

    // 3. 找到峰值及其索引（处理多峰值情况）
    // 找到最大值
    float peak_value = values[0];
    for (int i = 1; i < 8; i++) {
        if (values[i] > peak_value) {
            peak_value = values[i];
        }
    }
    
    // 找到所有等于最大值的传感器
    int first_peak = -1;
    int last_peak = -1;
    int peak_count = 0;
    for (int i = 0; i < 8; i++) {
        if (fabsf(values[i] - peak_value) < 0.01f) {  // 考虑浮点误差
            if (first_peak == -1) first_peak = i;
            last_peak = i;
            peak_count++;
        }
    }
    
    // 如果有多个峰值（宽白线），直接返回峰值区域的几何中心位置
    if (peak_count >= 3) {
        // 宽白线：直接计算first_peak和last_peak对应位置的平均
        float center_position = (SENSOR_POSITIONS[first_peak] + SENSOR_POSITIONS[last_peak]) / 2.0f;
        return center_position;
    }
    
    // 单峰或双峰：使用抛物线拟合
    int peak_idx = (first_peak + last_peak) / 2;

    // 4. 边界处理：使用虚拟邻点进行拟合，避免直接跳到 ±1000
    if (peak_idx == 0) {
        // 边缘左侧：构造虚拟左邻点 y(-1) = 2*y0 - y1，使用 (-1,0,1)
        float y_m1 = 2.0f * values[0] - values[1];
        float y0e  = values[0];
        float y1e  = values[1];

        float denom_e = 2.0f * (y_m1 - 2.0f * y0e + y1e);
        float offset_e;
        if (fabsf(denom_e) < 0.001f) {
            float weighted_sum = y_m1 * (-1.0f) + y0e * 0.0f + y1e * 1.0f;
            float total_weight = y_m1 + y0e + y1e;
            if (total_weight < 0.001f) {
                return SENSOR_POSITIONS[0];
            }
            offset_e = weighted_sum / total_weight; // [-1,1]
        } else {
            offset_e = (y_m1 - y1e) / denom_e; // [-1,1]
        }
        if (offset_e > 1.0f) offset_e = 1.0f;
        if (offset_e < -1.0f) offset_e = -1.0f;
        float final_position_e = SENSOR_POSITIONS[0] + offset_e * SENSOR_SPACING;
        if (final_position_e > 1000.0f) final_position_e = 1000.0f;
        if (final_position_e < -1000.0f) final_position_e = -1000.0f;
        return final_position_e;
    }
    if (peak_idx == 7) {
        // 边缘右侧：构造虚拟右邻点 y(8) = 2*y7 - y6，使用 (-1,0,1) 映射到 (6,7,8)
        float y0e = values[6];
        float y1e = values[7];
        float y2e = 2.0f * values[7] - values[6];

        float denom_e = 2.0f * (y0e - 2.0f * y1e + y2e);
        float offset_e;
        if (fabsf(denom_e) < 0.001f) {
            float weighted_sum = y0e * (-1.0f) + y1e * 0.0f + y2e * 1.0f;
            float total_weight = y0e + y1e + y2e;
            if (total_weight < 0.001f) {
                return SENSOR_POSITIONS[7];
            }
            offset_e = weighted_sum / total_weight; // [-1,1]
        } else {
            offset_e = (y0e - y2e) / denom_e; // [-1,1]
        }
        if (offset_e > 1.0f) offset_e = 1.0f;
        if (offset_e < -1.0f) offset_e = -1.0f;
        float final_position_e = SENSOR_POSITIONS[7] + offset_e * SENSOR_SPACING;
        if (final_position_e > 1000.0f) final_position_e = 1000.0f;
        if (final_position_e < -1000.0f) final_position_e = -1000.0f;
        return final_position_e;
    }

    // 5. 三点抛物线拟合（使用归一化后的值）
    // 取峰值点及其左右相邻点
    float y0 = values[peak_idx - 1];  // 左点
    float y1 = values[peak_idx];      // 峰值点（中点）
    float y2 = values[peak_idx + 1];  // 右点

    // 计算抛物线参数
    // 分母 = 2*(y0 - 2*y1 + y2)
    float denominator = 2.0f * (y0 - 2.0f * y1 + y2);

    // 检查分母是否接近0（三点共线情况）
    if (fabsf(denominator) < 0.001f) {
        // 退化为加权平均
        float weighted_sum = y0 * (-1.0f) + y1 * 0.0f + y2 * 1.0f;
        float total_weight = y0 + y1 + y2;
        
        if (total_weight < 0.001f) {
            return SENSOR_POSITIONS[peak_idx];
        }
        
        float offset = weighted_sum / total_weight;
        return SENSOR_POSITIONS[peak_idx] + offset * SENSOR_SPACING;
    }

    // 6. 计算顶点偏移量（相对于peak_idx）
    // offset范围：-1 到 +1（表示在相邻传感器之间的相对位置）
    float offset = (y0 - y2) / denominator;

    // 限制偏移范围
    if (offset > 1.0f) offset = 1.0f;
    if (offset < -1.0f) offset = -1.0f;

    // 7. 计算最终位置
    float final_position = SENSOR_POSITIONS[peak_idx] + offset * SENSOR_SPACING;

    // 8. 限幅
    if (final_position > 1000.0f) final_position = 1000.0f;
    if (final_position < -1000.0f) final_position = -1000.0f;

    return final_position;
}

/**
 * @brief 检测巡线状态
 * 
 * 改进的丢线检测：
 * 1. 检测最大值和最小值的差值
 * 2. 如果差值太小，说明所有传感器读数相近（丢线）
 */
LineFollower::Status LineFollower::detectStatus(uint16_t sensor_data[8]) {
    // 1. 找到最大值和最小值
    uint16_t max_val = sensor_data[0];
    uint16_t min_val = sensor_data[0];
    
    for (int i = 1; i < 8; i++) {
        if (sensor_data[i] > max_val) max_val = sensor_data[i];
        if (sensor_data[i] < min_val) min_val = sensor_data[i];
    }

    // 2. 计算差值（对比度）
    uint16_t contrast = max_val - min_val;

    // 3. 判断状态
    // 如果对比度很小，说明所有传感器读数相近（丢线）
    // 根据你的数据：黑底约1600，白底约300，差值约1300
    // 丢线时差值应该<500
    const uint16_t LOST_LINE_CONTRAST_THRESHOLD = 400;
    
    if (contrast < LOST_LINE_CONTRAST_THRESHOLD) {
        return Status::LOST_LINE;
    }

    // 4. 检测十字路口（所有传感器都检测到线）
    // 这里可以根据具体需求添加十字路口检测逻辑
    // ...

    return Status::ON_LINE;
}

/**
 * @brief PID控制计算
 * 
 * 实现增量式PID控制，包含：
 * - P项：比例控制，响应当前误差
 * - I项：积分控制，消除稳态误差（带抗饱和）
 * - D项：微分控制，抑制震荡（带低通滤波）
 */
float LineFollower::computePID(float error, uint32_t dt) {
    // 1. 比例项（P）
    float p_term = kp_ * error;

    // 2. 积分项（I）
    integral_ += error * dt;
    
    // 积分限幅（抗饱和）
    const float INTEGRAL_MAX = 100000.0f;
    if (integral_ > INTEGRAL_MAX) integral_ = INTEGRAL_MAX;
    if (integral_ < -INTEGRAL_MAX) integral_ = -INTEGRAL_MAX;
    
    float i_term = ki_ * integral_ / 1000.0f;

    // 3. 微分项（D）
    float derivative = 0;
    if (dt > 0) {
        derivative = (error - last_error_) * 1000.0f / dt;
    }
    
    // 对微分项进行低通滤波（减少噪声影响）
    const float DERIVATIVE_FILTER_ALPHA = 0.3f;
    derivative_filtered_ = DERIVATIVE_FILTER_ALPHA * derivative + 
                          (1.0f - DERIVATIVE_FILTER_ALPHA) * derivative_filtered_;
    
    float d_term = kd_ * derivative_filtered_;
    
    last_error_ = error;

    // 4. 总输出
    float output = p_term + i_term + d_term;

    // 5. 输出限幅
    if (output > 100.0f) output = 100.0f;
    if (output < -100.0f) output = -100.0f;

    return output;
}

/**
 * @brief 丢线处理（超温和版：几乎不转向）
 * 
 * 策略改进：
 * 1. 优先直行慢速前进，给传感器时间重新找到线
 * 2. 只在大偏差时才轻微转向
 * 3. 避免过度反应导致越跑越偏
 */
void LineFollower::handleLostLine() {
    return ;
    const float MEDIUM_POSITION_THRESHOLD = 400.0f;
    const float LARGE_POSITION_THRESHOLD = 700.0f;
    
    // 根据上次位置的大小，动态计算搜索转向强度
    float abs_last_pos = fabsf(last_valid_position_);
    int16_t search_turn_speed;
    
    if (abs_last_pos < MEDIUM_POSITION_THRESHOLD) {
        // 中小偏差：直行，完全不转向
        // 大部分情况下，丢线是暂时的，直行前进就能重新找到
        search_turn_speed = 0;
    } else if (abs_last_pos < LARGE_POSITION_THRESHOLD) {
        // 中大偏差：轻微转向（使用最大转向的50%）
        search_turn_speed = lost_line_max_turn_speed_ / 2;
    } else {
        // 大偏差：使用最大转向速度
        search_turn_speed = lost_line_max_turn_speed_;
    }
    
    // 根据方向决定左转还是右转
    if (last_valid_position_ > MEDIUM_POSITION_THRESHOLD) {
        // 上次线在右侧，轻微右转搜索
        drive_.setTargetSpeed(lost_line_search_speed_, -search_turn_speed);
    } else if (last_valid_position_ < -MEDIUM_POSITION_THRESHOLD) {
        // 上次线在左侧，轻微左转搜索
        drive_.setTargetSpeed(lost_line_search_speed_, search_turn_speed);
    } else {
        // 中小偏差，直行搜索
        drive_.setTargetSpeed(lost_line_search_speed_, 0);
    }
    
    drive_.update();
    
    if (debug_enabled_) {
        int16_t last_pos_int = (int16_t)last_valid_position_;
        Debug_Printf("[丢线] 上次:%d 搜索转向:%d 速度:%d\r\n", 
                     last_pos_int, search_turn_speed, lost_line_search_speed_);
    }
}

/**
 * @brief 重置PID状态
 */
void LineFollower::resetPID() {
    integral_ = 0.0f;
    last_error_ = 0.0f;
    derivative_filtered_ = 0.0f;
    pid_output_ = 0.0f;
}
