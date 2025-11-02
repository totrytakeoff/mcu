/**
 * @file    line_follower_pid.cpp
 * @brief   基于PID控制器的巡线类实现
 * @author  AI Assistant
 * @date    2024
 */

#include "line_follower_pid.hpp"
#include "debug.hpp"
#include <stdio.h>
#include <math.h>

/**
 * @brief 构造函数
 */
LineFollowerPID::LineFollowerPID(LineSensor& sensor,
                                 Motor& motor_lf, Motor& motor_lb,
                                 Motor& motor_rf, Motor& motor_rb)
    : sensor_(sensor)
    , motor_lf_(motor_lf)
    , motor_lb_(motor_lb)
    , motor_rf_(motor_rf)
    , motor_rb_(motor_rb)
    , pid_(0.06f, 0.0f, 1.0f)  // 默认PID参数
    , line_mode_(LineMode::WHITE_ON_BLACK)
    , base_speed_(30)
    , threshold_(0)  // 0表示使用传感器校准阈值
    , line_lost_threshold_(1)  // 至少1个传感器检测到线
    , debug_enabled_(false)
    , state_(State::STOPPED)
    , error_(0.0f)
    , last_position_(0.0f)
    , pid_output_(0.0f)
    , left_speed_(0)
    , right_speed_(0)
    , last_update_time_(0)
    // 初始化可调参数（提供更合理的默认值）
    , max_adjustment_ratio_(0.8f)  // 增加到80%，允许更大调整幅度
    , min_speed_ratio_(0.1f)       // 降低到10%，允许更慢的速度
    , max_speed_ratio_(2.0f)       // 增加到200%，允许更快的速度
    , pid_output_ratio_(0.8f)      // 增加到80%，更大的PID输出范围
    // 非线性映射参数（保持保守，便于调试）
    , small_threshold_(0.2f)
    , medium_threshold_(0.5f)
    , large_threshold_(0.8f)
    , small_gain_(0.1f)
    , medium_gain_(0.3f)
    , large_gain_(0.6f)
{
    // 配置PID控制器 - 输出限制将动态设置（在setBaseSpeed中）
    pid_.setSampleTime(0.01f);  // 10ms采样时间（与控制周期一致）
    pid_.setAntiWindup(true);   // 启用积分抗饱和
    pid_.setDerivativeFilter(0.6f);  // 降低D项高频跟随，减少边缘符号抖动

    // 初始化动态PID输出限制
    updatePIDOutputLimits();
}

/**
 * @brief 初始化巡线系统
 */
void LineFollowerPID::init() {
    // 重置PID
    pid_.reset();
    
    // 初始化状态
    state_ = State::STOPPED;
    error_ = 0.0f;
    last_position_ = 0.0f;
    pid_output_ = 0.0f;
    left_speed_ = 0;
    right_speed_ = 0;
    last_update_time_ = HAL_GetTick();
    
    Debug_Printf("[LineFollower] 初始化完成\r\n");
}

/**
 * @brief 启动巡线
 */
void LineFollowerPID::start() {
    state_ = State::RUNNING;
    pid_.reset();  // 重置PID状态
    last_position_ = 0.0f;
    last_update_time_ = HAL_GetTick();
    Debug_Printf("[LineFollower] 启动巡线\r\n");
}

/**
 * @brief 停止巡线
 */
void LineFollowerPID::stop() {
    state_ = State::STOPPED;
    
    // 停止所有电机
    motor_lf_.stop();
    motor_lb_.stop();
    motor_rf_.stop();
    motor_rb_.stop();
    
    left_speed_ = 0;
    right_speed_ = 0;
    
    Debug_Printf("[LineFollower] 停止巡线\r\n");
}

/**
 * @brief 更新巡线控制
 */
void LineFollowerPID::update() {
    if (state_ == State::STOPPED) {
        return;
    }
    
    // 计算时间间隔
    uint32_t current_time = HAL_GetTick();
    float dt = (current_time - last_update_time_) / 1000.0f;
    last_update_time_ = current_time;
    
    // 防御性：限制dt范围，避免偶发大dt导致D项异常和过大调整
    if (dt <= 0.0f) dt = 0.01f;       // 10ms
    if (dt > 0.1f)  dt = 0.1f;        // 100ms

    // 传感器预自适应：在采样前根据上次位置调整滤波和采样次数以提升响应
    {
        float prev_ratio = fabsf(last_position_) / 1000.0f; // [0,1]
        if (prev_ratio > 1.0f) prev_ratio = 1.0f;
        float alpha = 0.6f + 0.25f * prev_ratio; // 0.6~0.85，避免过快导致噪声放大
        sensor_.setFilterAlpha(alpha);
        // 固定3次中值采样：折中稳定与响应，避免运行中切换采样次数
        sensor_.setMedianSamples(3);
    }

    // 从传感器获取线位置和数据（使用传感器的独立阈值）
    uint16_t sensor_data[8];
    bool binary_data[8];
    float line_position = sensor_.getLinePositionWithData(sensor_data, binary_data,
                                                          line_mode_, 0);  // 0表示使用独立阈值
    // 缓存传感器数据以便显示使用（避免OLED重复采样）
    for (int i = 0; i < 8; i++) {
        last_sensor_data_[i] = sensor_data[i];
        last_binary_data_[i] = binary_data[i];
    }
    
    // 自动方向判定（早期一次性）：用二值化左右计数与原始位置符号比对
    if (!orientation_confirmed_ && !isnan(line_position)) {
        int left_on = 0, right_on = 0;
        for (int i = 0; i < 8; i++) {
            if (binary_data[i]) {
                if (i < 4) left_on++; else right_on++;
            }
        }
        float pos_raw = line_position; // 未翻转前的原始符号
        if (fabsf(pos_raw) > 150.0f && (left_on + right_on) >= 1 && (left_on + right_on) <= 7) {
            int expected_sign = (left_on > right_on) ? -1 : (right_on > left_on ? +1 : 0);
            int actual_sign = (pos_raw > 0.0f) ? +1 : (pos_raw < 0.0f ? -1 : 0);
            orientation_frames_++;
            if (expected_sign != 0 && actual_sign != 0 && expected_sign != actual_sign) {
                orientation_mismatch_++;
            }
            // 采样5帧后判断：多数不一致则翻转
            if (orientation_frames_ >= 5) {
                if (orientation_mismatch_ >= 3) {
                    invert_position_ = !invert_position_;
                    Debug_Printf("[LineFollower] 自动方向校正: invert_position=%d\r\n", invert_position_);
                }
                orientation_confirmed_ = true;
            }
        }
    }

    if (invert_position_) {
        line_position = -line_position;
    }

    // 检查是否丢线（使用isnan判断）或位置异常
    bool position_invalid = isnan(line_position) ||
                           fabs(line_position) > 1000.0f;

    // 辅助丢线判断：基于探头计数（全白/全黑已在传感器层返回NaN，这里作为双保险）
    bool lost_by_count = false;
    int count_on = 0;
    for (int i = 0; i < 8; i++) if (last_binary_data_[i]) count_on++;
    if (count_on < line_lost_threshold_ || count_on == 8) lost_by_count = true;

    if (position_invalid || lost_by_count) {
        state_ = State::LINE_LOST;

        // 丢线处理：使用上次位置继续，但如果上次位置也异常则归零
        if (fabs(last_position_) > 1000.0f) {
            last_position_ = 0.0f;  // 重置异常位置
        }

        line_position = last_position_;

        // 降低速度（保持一定巡线能力，同时避免误判激进转向）
        left_speed_ = static_cast<int>(base_speed_ * 0.6f);
        right_speed_ = static_cast<int>(base_speed_ * 0.6f);

        if (debug_enabled_) {
            Debug_Printf("[LineFollower] 丢线! 使用上次位置: %d\r\n", (int)(last_position_ * 1000.0f));
        }
    } else {
        state_ = State::RUNNING;

        // 只保存有效位置
        last_position_ = line_position;
        
        // 误差 = 目标位置(0) - 当前位置
        error_ = 0.0f - line_position;
        
        // PID计算速度调整（输出范围已根据baseSpeed动态限制）
        pid_output_ = pid_.compute(0.0f, line_position, dt);

        // 自适应传感器滤波：误差越大，滤波越弱（提升响应速度）
        // 运行中不再重复调整采样次数，仅保持采样前的α设置

        // 简化的非线性映射：减少复杂层次，提高响应性
        float abs_output = fabs(pid_output_);
        float max_output = base_speed_ * pid_output_ratio_;  // 最大PID输出范围

        // 直接使用PID输出，避免多层衰减
        float adjusted_output = pid_output_;

        // 限制在最大范围内，避免过度调整
        if (abs_output > max_output) {
            adjusted_output = (pid_output_ > 0) ? max_output : -max_output;
        }

        // 基于百分比的速度计算
        float speed_adjustment_ratio = adjusted_output / max_output;  // 归一化到[-1, 1]
        // 防抖门限+滞回（使用方向滞回决定符号，避免历史输出符号误导）
        const float dead_low = 0.06f;   // 低阈值：小于此认为0
        const float dead_high = 0.10f;  // 高阈值：大于此才认为非0
        float mag = fabsf(speed_adjustment_ratio);
        if (mag < dead_low) {
            speed_adjustment_ratio = 0.0f;
        } else if (mag < dead_high) {
            speed_adjustment_ratio = (speed_adjustment_ratio >= 0.0f ? 1.0f : -1.0f) * dead_high;
        }

        // 自适应差速幅度与最小速度：偏差越大，允许更大转向，内侧速度最低可降至0
        float error_ratio = fabsf(line_position) / 1000.0f;  // [0,1]
        if (error_ratio > 1.0f) error_ratio = 1.0f;
        const float HARD_MAX_ADJ = 1.0f;   // 大偏差时最大允许调整幅度（100%）
        const float HARD_MIN_RATIO = 0.0f; // 大偏差时内侧车轮最低比例（0%）
        float dynamic_max_adj = max_adjustment_ratio_ + (HARD_MAX_ADJ - max_adjustment_ratio_) * error_ratio;
        float dynamic_min_ratio = min_speed_ratio_ * (1.0f - error_ratio) + HARD_MIN_RATIO * error_ratio; // 线性到0
        if (dynamic_min_ratio < 0.0f) dynamic_min_ratio = 0.0f;

        float target_adjustment = speed_adjustment_ratio * dynamic_max_adj;

        // 方向滞回：仅用位置符号决定内外侧，且设置切换滞回区间
        // 更新 last_inner_left_：|position|>dir_hyst_high_ 才切换；|position|<dir_hyst_low_ 保持
        float pos_abs = fabsf(line_position);
        if (pos_abs > dir_hyst_high_) {
            last_inner_left_ = (line_position < 0.0f); // 负值=线在左 → 左为内侧
        } // 否则保持原值

        // 抖动抑制：调整系数限斜率（同向快、反向慢）
        float max_delta_same = 8.0f * dt;  // 同向快速上升（~0.125/帧）
        float max_delta_flip = 2.0f * dt;  // 反向缓慢变化（~0.025/帧）
        float delta = target_adjustment - last_adjustment_factor_;
        if ((delta > 0.0f && last_adjustment_factor_ >= 0.0f) || (delta < 0.0f && last_adjustment_factor_ <= 0.0f)) {
            if (delta > max_delta_same) delta = max_delta_same;
            if (delta < -max_delta_same) delta = -max_delta_same;
        } else {
            if (delta > max_delta_flip) delta = max_delta_flip;
            if (delta < -max_delta_flip) delta = -max_delta_flip;
        }
        float adjustment_factor = last_adjustment_factor_ + delta;
        // 轻微一阶低通平滑
        adjustment_factor = 0.6f * adjustment_factor + 0.4f * last_adjustment_factor_;
        last_adjustment_factor_ = adjustment_factor;

        // 差速控制（左右电机硬件方向相反）：
        // 为实现“线在左时向左纠偏”，应当“左轮更慢、右轮更快”
        // 注意：applySpeed() 内部对左侧速度取反，右侧保持正向
        float left_speed_f  = base_speed_ * (1.0f - adjustment_factor);
        float right_speed_f = base_speed_ * (1.0f + adjustment_factor);

        // 双向速度保护：统一最小/最大速度，避免限幅破坏差速方向
        float min_speed = base_speed_ * min_speed_ratio_;
        float max_speed = base_speed_ * max_speed_ratio_;
        
        if (left_speed_f < min_speed)  left_speed_f = min_speed;
        if (left_speed_f > max_speed)  left_speed_f = max_speed;
        if (right_speed_f < min_speed) right_speed_f = min_speed;
        if (right_speed_f > max_speed) right_speed_f = max_speed;

        left_speed_  = static_cast<int>(left_speed_f);
        right_speed_ = static_cast<int>(right_speed_f);
    }
    
    // 应用速度到电机
    applySpeed(left_speed_, right_speed_);
    
    // 调试输出（减少频率以提升性能）
    if (debug_enabled_) {
        static uint32_t last_debug_time = 0;
        uint32_t current_time = HAL_GetTick();

        // 每100ms输出一次调试信息，而不是每次控制循环都输出
        if (current_time - last_debug_time >= 100) {
            last_debug_time = current_time;
            printDebugInfo(last_sensor_data_, last_binary_data_);
        }
    }
}

/**
 * @brief 应用速度到电机
 */
void LineFollowerPID::applySpeed(int left_speed, int right_speed) {
    // 左侧电机（前后同步）- 需要反向补偿机械安装方向
    motor_lf_.setSpeed(-left_speed);
    motor_lb_.setSpeed(-left_speed);

    // 右侧电机（前后同步）- 正方向为前进
    motor_rf_.setSpeed(right_speed);
    motor_rb_.setSpeed(right_speed);
}

void LineFollowerPID::getLastSensorData(uint16_t out[8]) const {
    for (int i = 0; i < 8; i++) out[i] = last_sensor_data_[i];
}

void LineFollowerPID::getLastBinaryData(bool out[8]) const {
    for (int i = 0; i < 8; i++) out[i] = last_binary_data_[i];
}

/**
 * @brief 速度限幅
 */
int LineFollowerPID::clampSpeed(int speed) {
    if (speed > 100) {
        return 100;
    } else if (speed < -100) {
        return -100;
    }
    return speed;
}

/**
 * @brief 设置PID参数
 */
void LineFollowerPID::setPID(float kp, float ki, float kd) {
    pid_.setTunings(kp, ki, kd);
    Debug_Printf("[LineFollower] PID参数: Kp=%.3f, Ki=%.3f, Kd=%.3f\r\n", kp, ki, kd);
}

/**
 * @brief 设置基础速度（自动调整PID参数）
 */
void LineFollowerPID::setBaseSpeed(int speed) {
    if (speed >= 0 && speed <= 100) {
        base_speed_ = speed;

        // 动态调整PID输出限制
        updatePIDOutputLimits();

        Debug_Printf("[LineFollower] 基础速度: %d (PID限制: ±%.1f)\r\n",
                     speed, base_speed_ * 0.6f);
    }
}

/**
 * @brief 设置线模式
 */
void LineFollowerPID::setLineMode(LineMode mode) {
    line_mode_ = mode;
    const char* mode_str = (mode == LineMode::WHITE_ON_BLACK) ? "黑底白线" : "白底黑线";
    Debug_Printf("[LineFollower] 线模式: %s\r\n", mode_str);
}

/**
 * @brief 设置阈值
 */
void LineFollowerPID::setThreshold(uint16_t threshold) {
    threshold_ = threshold;
    if (threshold == 0) {
        Debug_Printf("[LineFollower] 阈值: 使用传感器校准值\r\n");
    } else {
        Debug_Printf("[LineFollower] 阈值: %d\r\n", threshold);
    }
}

/**
 * @brief 设置丢线阈值
 */
void LineFollowerPID::setLineLostThreshold(int min_sensors) {
    if (min_sensors >= 0 && min_sensors <= 8) {
        line_lost_threshold_ = min_sensors;
        Debug_Printf("[LineFollower] 丢线阈值: %d个传感器\r\n", min_sensors);
    }
}

/**
 * @brief 启用调试输出
 */
void LineFollowerPID::enableDebug(bool enable) {
    debug_enabled_ = enable;
    Debug_Printf("[LineFollower] 调试输出: %s\r\n", enable ? "启用" : "禁用");
}

/**
 * @brief 重置PID
 */
void LineFollowerPID::resetPID() {
    pid_.reset();
    last_position_ = 0.0f;
    Debug_Printf("[LineFollower] PID已重置\r\n");
}

/**
 * @brief 打印调试信息
 */
void LineFollowerPID::printDebugInfo(const uint16_t sensor_data[8], const bool binary_data[8]) {
    // 格式: Pos:xxx.x Err:xxx.x PID:xx.x L:xx R:xx | S:xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | B:████
    
    // 位置、误差、PID输出、速度（将浮点数转换为整数，乘以1000表示3位小数）
    Debug_Printf("Pos:%d Err:%d PID:%d L:%d R:%d | ",
                 static_cast<int>(last_position_ * 1000.0f),  // 乘以1000转换为整数
                 static_cast<int>(error_ * 1000.0f),
                 static_cast<int>(pid_output_ * 1000.0f),
                 left_speed_,
                 right_speed_);
    
    // 传感器数据
    Debug_Printf("S:");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%4d ", sensor_data[i]);
    }
    
    // 二值化数据（使用ASCII字符表示）
    Debug_Printf("| B:");
    for (int i = 0; i < 8; i++) {
        Debug_Printf("%c", binary_data[i] ? 'B' : 'W');
    }
    
    // PID各项（可选，用于深度调试）
    // Debug_Printf("| P:%.1f I:%.1f D:%.1f",
    //              pid_.getProportional(),
    //              pid_.getIntegral(),
    //              pid_.getDerivative());
    
    Debug_Printf("\r\n");
}

/**
 * @brief 动态更新PID输出限制（基于基础速度）
 */
void LineFollowerPID::updatePIDOutputLimits() {
    // PID输出限制使用可调参数
    float limit = base_speed_ * pid_output_ratio_;
    pid_.setOutputLimits(-limit, limit);
}

/**
 * @brief 双向速度保护函数（使用可调参数）
 */
void LineFollowerPID::constrainSpeeds() {
    // 计算速度范围（使用可调参数）
    int min_speed = static_cast<int>(base_speed_ * min_speed_ratio_);
    int max_speed = static_cast<int>(base_speed_ * max_speed_ratio_);

    // 约束速度范围
    if (left_speed_ < min_speed) left_speed_ = min_speed;
    if (left_speed_ > max_speed) left_speed_ = max_speed;
    if (right_speed_ < min_speed) right_speed_ = min_speed;
    if (right_speed_ > max_speed) right_speed_ = max_speed;
}

/**
 * @brief 设置控制参数（可调）
 */
void LineFollowerPID::setControlParameters(float max_adjustment_ratio,
                                           float min_speed_ratio,
                                           float max_speed_ratio,
                                           float pid_output_ratio) {
    max_adjustment_ratio_ = max_adjustment_ratio;
    min_speed_ratio_ = min_speed_ratio;
    max_speed_ratio_ = max_speed_ratio;
    pid_output_ratio_ = pid_output_ratio;

    // 重新计算PID输出限制
    updatePIDOutputLimits();

    Debug_Printf("[LineFollower] 控制参数更新: 调整幅度=%.0f%%, 速度范围=%.0f%%-%.0f%%, PID限制=%.0f%%\r\n",
                 max_adjustment_ratio * 100, min_speed_ratio * 100, max_speed_ratio * 100, pid_output_ratio * 100);
}

/**
 * @brief 设置非线性映射参数（可调）
 */
void LineFollowerPID::setNonLinearParameters(float small_threshold,
                                              float medium_threshold,
                                              float large_threshold,
                                              float small_gain,
                                              float medium_gain,
                                              float large_gain) {
    small_threshold_ = small_threshold;
    medium_threshold_ = medium_threshold;
    large_threshold_ = large_threshold;
    small_gain_ = small_gain;
    medium_gain_ = medium_gain;
    large_gain_ = large_gain;

    Debug_Printf("[LineFollower] 非线性参数更新: 阈值=%.2f/%.2f/%.2f, 增益=%.2f/%.2f/%.2f\r\n",
                 small_threshold, medium_threshold, large_threshold, small_gain, medium_gain, large_gain);
}

/**
 * @brief 获取当前控制参数
 */
void LineFollowerPID::getControlParameters(float& max_adjustment, float& min_speed,
                                          float& max_speed, float& pid_output) const {
    max_adjustment = max_adjustment_ratio_;
    min_speed = min_speed_ratio_;
    max_speed = max_speed_ratio_;
    pid_output = pid_output_ratio_;
}
