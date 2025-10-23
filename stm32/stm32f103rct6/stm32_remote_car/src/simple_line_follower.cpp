/**
 * @file    simple_line_follower.cpp
 * @brief   简易双传感器巡线系统实现
 * @author  AI Assistant
 * @date    2024
 */

#include "simple_line_follower.hpp"
#include "debug.hpp"
#include "stm32f1xx_hal.h"
#include <cmath>

/**
 * @brief 构造函数
 */
SimpleLineFollower::SimpleLineFollower(LineSensor& sensor, 
                                       Motor& fl, Motor& fr, 
                                       Motor& rl, Motor& rr)
    : sensor_(sensor)
    , motor_fl_(fl)
    , motor_fr_(fr)
    , motor_rl_(rl)
    , motor_rr_(rr)
    , line_mode_(LineMode::WHITE_LINE_ON_BLACK)
    , base_speed_(20)
    , speed_tiny_(2)
    , speed_soft_(5)
    , speed_mid_(10)
    , speed_hard_(18)
    , threshold_lost_(15.0f)
    , threshold_sharp_turn_(15.0f)
    , threshold_on_line_(70.0f)
    , left_normalized_(0.0f)
    , right_normalized_(0.0f)
    , status_(Status::STOPPED)
    , last_status_(Status::STRAIGHT)
    , lost_line_start_time_(0)
    , lost_line_duration_(3000)
    , sharp_turn_start_time_(0)
    , sharp_turn_duration_(500)
    , sharp_turn_active_(false)
    , sharp_turn_direction_(Status::STRAIGHT)
    , debug_enabled_(false)
    , update_count_(0)
{
    // 初始化校准数组
    for (int i = 0; i < 8; i++) {
        white_cal_[i] = 0;
        black_cal_[i] = 2000;
    }
}

/**
 * @brief 初始化
 */
void SimpleLineFollower::init() {
    // 获取传感器校准数据
    sensor_.getCalibrationValues(white_cal_, black_cal_);
    
    status_ = Status::STRAIGHT;
    last_status_ = Status::STRAIGHT;
    
    Debug_Printf("[SimpleLineFollower] 初始化完成\r\n");
    Debug_Printf("[SimpleLineFollower] 模式: %s\r\n", 
                 line_mode_ == LineMode::WHITE_LINE_ON_BLACK ? "黑底白线" : "白底黑线");
    Debug_Printf("[SimpleLineFollower] 基础速度: %d\r\n", base_speed_);
    Debug_Printf("[SimpleLineFollower] 左传感器(0) 校准: W=%d B=%d\r\n", 
                 white_cal_[0], black_cal_[0]);
    Debug_Printf("[SimpleLineFollower] 右传感器(7) 校准: W=%d B=%d\r\n", 
                 white_cal_[7], black_cal_[7]);
}

/**
 * @brief 更新巡线控制
 */
void SimpleLineFollower::update() {
    // 读取传感器数据
    uint16_t sensor_data[8];
    sensor_.getData(sensor_data);
    
    // 归一化左右传感器值
    left_normalized_ = normalizeValue(0, sensor_data[0]);
    right_normalized_ = normalizeValue(7, sensor_data[7]);
    
    // 分析状态
    analyzeState();
    
    // 应用电机控制
    applyMotorControl();
    
    // 调试输出（每10次更新输出一次）
    if (debug_enabled_ && (update_count_ % 10 == 0)) {
        debugPrint();
    }
    
    update_count_++;
}

/**
 * @brief 归一化传感器数据为百分比
 */
float SimpleLineFollower::normalizeValue(uint8_t sensor_idx, uint16_t raw_value) {
    uint16_t white_val = white_cal_[sensor_idx];
    uint16_t black_val = black_cal_[sensor_idx];
    
    // 防止除零
    if (black_val <= white_val) {
        return 0.0f;
    }
    
    float range = (float)(black_val - white_val);
    float normalized = ((float)raw_value - (float)white_val) / range;
    
    // 对于黑底白线，传感器在白色上时值较高
    // 我们希望归一化后：白线=100%, 黑色=0%
    if (line_mode_ == LineMode::WHITE_LINE_ON_BLACK) {
        normalized = normalized;  // 直接使用
    } else {
        normalized = 1.0f - normalized;  // 反转（白底黑线）
    }
    
    // 限制范围并转为百分比
    if (normalized < 0.0f) normalized = 0.0f;
    if (normalized > 1.0f) normalized = 1.0f;
    
    return normalized * 100.0f;  // 转为 0-100%
}

/**
 * @brief 分析传感器数据并决定状态
 */
void SimpleLineFollower::analyzeState() {
    float left = left_normalized_;
    float right = right_normalized_;
    
    // 1. 丢线/路口检测（两侧都是黑色）
    if (left < threshold_lost_ && right < threshold_lost_) {
        status_ = Status::LOST_LINE;
        return;
    }
    
    // 找回线了，重置丢线计时器
    if (lost_line_start_time_ != 0) {
        lost_line_start_time_ = 0;
        if (debug_enabled_) {
            Debug_Printf("[丢线恢复] ✅ 成功找回线！\r\n");
        }
    }
    
    // 2. 直角转弯检测与锁定（改进版）
    // 检查是否正在执行直角转弯
    if (sharp_turn_active_) {
        uint32_t turn_duration = HAL_GetTick() - sharp_turn_start_time_;
        
        // 如果转弯时间未到，继续保持转弯状态
        if (turn_duration < sharp_turn_duration_) {
            status_ = sharp_turn_direction_;
            if (debug_enabled_ && (update_count_ % 20 == 0)) {
                Debug_Printf("[直角转弯] 执行中... 剩余:%dms\r\n", 
                           (int)(sharp_turn_duration_ - turn_duration));
            }
            return;
        } else {
            // 转弯完成，解除锁定
            sharp_turn_active_ = false;
            if (debug_enabled_) {
                Debug_Printf("[直角转弯] ✅ 完成！恢复正常巡线\r\n");
            }
        }
    }
    
    // 检测新的直角转弯（更宽松的条件，提前检测）
    // 条件：一侧<20%（接近黑色），另一侧>45%（在线上）且差值>35%
    float abs_diff = fabs(left - right);
    
    // 急左转：左侧黑 且 右侧白 且 差值大
    if (left < 20.0f && right >= 45.0f && abs_diff >= 35.0f) {
        // 启动直角转弯模式
        sharp_turn_active_ = true;
        sharp_turn_start_time_ = HAL_GetTick();
        sharp_turn_direction_ = Status::TURN_LEFT_SHARP;
        status_ = Status::TURN_LEFT_SHARP;
        if (debug_enabled_) {
            Debug_Printf("[直角转弯] 🔄 检测到急左转！L=%.1f R=%.1f Diff=%.1f\r\n", 
                       left, right, abs_diff);
            Debug_Printf("[直角转弯] 先停车250ms，再原地转弯\r\n");
        }
        return;
    }
    
    // 急右转：右侧黑 且 左侧白 且 差值大
    if (right < 20.0f && left >= 45.0f && abs_diff >= 35.0f) {
        // 启动直角转弯模式
        sharp_turn_active_ = true;
        sharp_turn_start_time_ = HAL_GetTick();
        sharp_turn_direction_ = Status::TURN_RIGHT_SHARP;
        status_ = Status::TURN_RIGHT_SHARP;
        if (debug_enabled_) {
            Debug_Printf("[直角转弯] 🔄 检测到急右转！L=%.1f R=%.1f Diff=%.1f\r\n", 
                       left, right, abs_diff);
            Debug_Printf("[直角转弯] 先停车250ms，再原地转弯\r\n");
        }
        return;
    }
    
    // 3. 正常转向检测（增加死区，稳定控制）
    // 计算两侧差值，判断偏离方向和程度
    float diff = left - right;  // 正值=左侧高（可能右偏），负值=右侧高（可能左偏）
    float avg = (left + right) / 2.0f;  // 平均值，判断是否在线上
    
    // 如果两侧都比较高（平均值>60%），说明基本在线上
    if (avg >= 60.0f) {
        // 增加死区：差值小于10%时保持直行，避免小波动触发转向
        if (fabs(diff) < 10.0f) {
            status_ = Status::STRAIGHT;
            return;
        }
        // 右侧高（右>左，diff<0）→ 可能左偏
        else if (diff < 0) {
            float abs_diff = -diff;
            if (abs_diff < 15.0f) {
                status_ = Status::TURN_LEFT_TINY;  // 极微左偏
            } else if (abs_diff < 25.0f) {
                status_ = Status::TURN_LEFT_SOFT;  // 轻微左偏
            } else if (abs_diff < 40.0f) {
                status_ = Status::TURN_LEFT_MID;   // 中度左偏
            } else {
                status_ = Status::TURN_LEFT_HARD;  // 大幅左偏
            }
            return;
        }
        // 左侧高（左>右，diff>0）→ 可能右偏
        else {
            float abs_diff = diff;
            if (abs_diff < 15.0f) {
                status_ = Status::TURN_RIGHT_TINY;  // 极微右偏
            } else if (abs_diff < 25.0f) {
                status_ = Status::TURN_RIGHT_SOFT;  // 轻微右偏
            } else if (abs_diff < 40.0f) {
                status_ = Status::TURN_RIGHT_MID;   // 中度右偏
            } else {
                status_ = Status::TURN_RIGHT_HARD;  // 大幅右偏
            }
            return;
        }
    }
    
    // 如果平均值较低，但有一侧明显高，说明偏离较大
    // 左侧在线（白色区域），右侧偏离
    if (left >= threshold_on_line_ && right < threshold_on_line_) {
        if (right >= 50.0f) {
            status_ = Status::TURN_RIGHT_SOFT;
        } else if (right >= 35.0f) {
            status_ = Status::TURN_RIGHT_MID;
        } else {
            status_ = Status::TURN_RIGHT_HARD;
        }
        return;
    }
    
    // 右侧在线（白色区域），左侧偏离
    if (right >= threshold_on_line_ && left < threshold_on_line_) {
        if (left >= 50.0f) {
            status_ = Status::TURN_LEFT_SOFT;
        } else if (left >= 35.0f) {
            status_ = Status::TURN_LEFT_MID;
        } else {
            status_ = Status::TURN_LEFT_HARD;
        }
        return;
    }
    
    // 4. 中间状态（两侧都在40-70%之间）
    if (left >= 40.0f && right >= 40.0f) {
        status_ = Status::STRAIGHT;  // 保持直行
        return;
    }
    
    // 5. 大幅转向（一侧在线但另一侧很黑）
    if (left >= 40.0f && right < threshold_sharp_turn_) {
        status_ = Status::TURN_LEFT_HARD;
        return;
    }
    if (right >= 40.0f && left < threshold_sharp_turn_) {
        status_ = Status::TURN_RIGHT_HARD;
        return;
    }
    
    // 默认保持上次状态
    // status_ 保持不变
}

/**
 * @brief 根据状态设置电机速度
 */
void SimpleLineFollower::applyMotorControl() {
    int left_speed = base_speed_;
    int right_speed = base_speed_;
    
    switch (status_) {
        case Status::STRAIGHT:
            // 直行：左右同速
            left_speed = base_speed_;
            right_speed = base_speed_;
            last_status_ = Status::STRAIGHT;
            break;
            
        // === 左转4级梯度 ===
        case Status::TURN_LEFT_TINY:
            // 极微左偏：左侧加速（1级）
            left_speed = base_speed_ + speed_tiny_;
            right_speed = base_speed_;
            last_status_ = Status::TURN_LEFT_TINY;
            break;
            
        case Status::TURN_LEFT_SOFT:
            // 轻微左偏：左侧加速（2级）
            left_speed = base_speed_ + speed_soft_;
            right_speed = base_speed_;
            last_status_ = Status::TURN_LEFT_SOFT;
            break;
            
        case Status::TURN_LEFT_MID:
            // 中度左偏：左侧加速（3级）
            left_speed = base_speed_ + speed_mid_;
            right_speed = base_speed_;
            last_status_ = Status::TURN_LEFT_MID;
            break;
            
        case Status::TURN_LEFT_HARD:
            // 大幅左偏：左侧加速（4级）
            left_speed = base_speed_ + speed_hard_;
            right_speed = base_speed_;
            last_status_ = Status::TURN_LEFT_HARD;
            break;
            
        case Status::TURN_LEFT_SHARP:
            // 急左转：先刹车停稳，再原地左转
            {
                uint32_t turn_time = HAL_GetTick() - sharp_turn_start_time_;
                if (turn_time < 250) {
                    // 前250ms：急刹车完全停止
                    left_speed = 0;
                    right_speed = 0;
                    if (debug_enabled_ && turn_time < 50) {
                        Debug_Printf("[直角转弯] 急刹车停止中...\r\n");
                    }
                } else {
                    // 250ms后：原地左转（右轮倒车，左轮前进）
                    left_speed = base_speed_ * 8 / 10;   // 80%速度转弯
                    right_speed = -(base_speed_ * 8 / 10);
                }
                last_status_ = Status::TURN_LEFT_SHARP;
            }
            break;
            
        // === 右转4级梯度 ===
        case Status::TURN_RIGHT_TINY:
            // 极微右偏：右侧加速（1级）
            left_speed = base_speed_;
            right_speed = base_speed_ + speed_tiny_;
            last_status_ = Status::TURN_RIGHT_TINY;
            break;
            
        case Status::TURN_RIGHT_SOFT:
            // 轻微右偏：右侧加速（2级）
            left_speed = base_speed_;
            right_speed = base_speed_ + speed_soft_;
            last_status_ = Status::TURN_RIGHT_SOFT;
            break;
            
        case Status::TURN_RIGHT_MID:
            // 中度右偏：右侧加速（3级）
            left_speed = base_speed_;
            right_speed = base_speed_ + speed_mid_;
            last_status_ = Status::TURN_RIGHT_MID;
            break;
            
        case Status::TURN_RIGHT_HARD:
            // 大幅右偏：右侧加速（4级）
            left_speed = base_speed_;
            right_speed = base_speed_ + speed_hard_;
            last_status_ = Status::TURN_RIGHT_HARD;
            break;
            
        case Status::TURN_RIGHT_SHARP:
            // 急右转：先刹车停稳，再原地右转
            {
                uint32_t turn_time = HAL_GetTick() - sharp_turn_start_time_;
                if (turn_time < 250) {
                    // 前250ms：急刹车完全停止
                    left_speed = 0;
                    right_speed = 0;
                    if (debug_enabled_ && turn_time < 50) {
                        Debug_Printf("[直角转弯] 急刹车停止中...\r\n");
                    }
                } else {
                    // 250ms后：原地右转（左轮倒车，右轮前进）
                    left_speed = -(base_speed_ * 8 / 10);
                    right_speed = base_speed_ * 8 / 10;   // 80%速度转弯
                }
                last_status_ = Status::TURN_RIGHT_SHARP;
            }
            break;
            
        case Status::LOST_LINE:
            // 丢线处理
            handleLostLine();
            return;  // handleLostLine 会设置电机，直接返回
            
        case Status::STOPPED:
            left_speed = 0;
            right_speed = 0;
            break;
    }
    
    // 限制速度范围 [-100, 100]
    if (left_speed > 100) left_speed = 100;
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < -100) right_speed = -100;
    
    // 设置四个电机（左右两侧方向相反，整体取反）
    // 左侧电机：反向
    motor_fl_.setSpeed(-left_speed);
    motor_rl_.setSpeed(-left_speed);
    // 右侧电机：正向
    motor_fr_.setSpeed(right_speed);
    motor_rr_.setSpeed(right_speed);
}

/**
 * @brief 处理丢线情况（持续3秒纠正寻线）
 */
void SimpleLineFollower::handleLostLine() {
    // 记录丢线开始时间
    if (lost_line_start_time_ == 0) {
        lost_line_start_time_ = HAL_GetTick();
    }
    
    // 检查丢线持续时间
    uint32_t lost_duration = HAL_GetTick() - lost_line_start_time_;
    
    // 如果丢线超过设定时间（默认3秒），停止
    if (lost_duration > lost_line_duration_) {
        Debug_Printf("[丢线] 超时！停止搜索\r\n");
        motor_fl_.stop();
        motor_rl_.stop();
        motor_fr_.stop();
        motor_rr_.stop();
        return;
    }
    
    // 根据上次状态决定纠正方向，使用中等速度持续搜索
    int left_speed = 0;
    int right_speed = 0;
    int search_speed = base_speed_ * 6 / 10;  // 60% 基础速度作为搜索速度（中等）
    
    // 如果上次是左转状态，继续向左搜索
    if (last_status_ == Status::TURN_LEFT_TINY ||
        last_status_ == Status::TURN_LEFT_SOFT || 
        last_status_ == Status::TURN_LEFT_MID || 
        last_status_ == Status::TURN_LEFT_HARD ||
        last_status_ == Status::TURN_LEFT_SHARP) {
        // 左侧快速，右侧慢速，继续左转搜索
        left_speed = search_speed;
        right_speed = search_speed / 3;
        if (debug_enabled_ && (update_count_ % 50 == 0)) {
            Debug_Printf("[丢线恢复] 左转搜索 剩余:%dms\r\n", (int)(lost_line_duration_ - lost_duration));
        }
    }
    // 如果上次是右转状态，继续向右搜索
    else if (last_status_ == Status::TURN_RIGHT_TINY ||
             last_status_ == Status::TURN_RIGHT_SOFT || 
             last_status_ == Status::TURN_RIGHT_MID || 
             last_status_ == Status::TURN_RIGHT_HARD ||
             last_status_ == Status::TURN_RIGHT_SHARP) {
        // 右侧快速，左侧慢速，继续右转搜索
        left_speed = search_speed / 3;
        right_speed = search_speed;
        if (debug_enabled_ && (update_count_ % 50 == 0)) {
            Debug_Printf("[丢线恢复] 右转搜索 剩余:%dms\r\n", (int)(lost_line_duration_ - lost_duration));
        }
    }
    // 如果上次是直行，前进搜索
    else {
        left_speed = search_speed;
        right_speed = search_speed;
        if (debug_enabled_ && (update_count_ % 50 == 0)) {
            Debug_Printf("[丢线恢复] 直行搜索 剩余:%dms\r\n", (int)(lost_line_duration_ - lost_duration));
        }
    }
    
    // 设置电机（左右两侧方向相反，整体取反）
    motor_fl_.setSpeed(-left_speed);
    motor_rl_.setSpeed(-left_speed);
    motor_fr_.setSpeed(right_speed);
    motor_rr_.setSpeed(right_speed);
}

/**
 * @brief 设置巡线模式
 */
void SimpleLineFollower::setLineMode(LineMode mode) {
    line_mode_ = mode;
    Debug_Printf("[SimpleLineFollower] 模式切换: %s\r\n", 
                 mode == LineMode::WHITE_LINE_ON_BLACK ? "黑底白线" : "白底黑线");
}

/**
 * @brief 设置基础速度
 */
void SimpleLineFollower::setBaseSpeed(int speed) {
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;
    base_speed_ = speed;
    Debug_Printf("[SimpleLineFollower] 基础速度: %d\r\n", base_speed_);
}

/**
 * @brief 停止所有电机
 */
void SimpleLineFollower::stop() {
    status_ = Status::STOPPED;
    motor_fl_.stop();
    motor_fr_.stop();
    motor_rl_.stop();
    motor_rr_.stop();
}

/**
 * @brief 设置速度梯度参数（4级）
 */
void SimpleLineFollower::setSpeedGradient(int tiny, int soft, int mid, int hard) {
    speed_tiny_ = tiny;
    speed_soft_ = soft;
    speed_mid_ = mid;
    speed_hard_ = hard;
    Debug_Printf("[SimpleLineFollower] 速度梯度(4级): 微=%d 轻=%d 中=%d 重=%d\r\n", 
                 speed_tiny_, speed_soft_, speed_mid_, speed_hard_);
}

/**
 * @brief 设置阈值参数
 */
void SimpleLineFollower::setThresholds(float lost_threshold, 
                                       float sharp_turn_threshold, 
                                       float on_line_threshold) {
    threshold_lost_ = lost_threshold;
    threshold_sharp_turn_ = sharp_turn_threshold;
    threshold_on_line_ = on_line_threshold;
    Debug_Printf("[SimpleLineFollower] 阈值设置: 丢线=%.1f%% 急转=%.1f%% 在线=%.1f%%\r\n", 
                 threshold_lost_, threshold_sharp_turn_, threshold_on_line_);
}

/**
 * @brief 设置直角转弯参数
 */
void SimpleLineFollower::setSharpTurnDuration(uint32_t duration) {
    sharp_turn_duration_ = duration;
    Debug_Printf("[SimpleLineFollower] 直角转弯锁定时间: %dms\r\n", sharp_turn_duration_);
}

/**
 * @brief 调试输出
 */
void SimpleLineFollower::debugPrint() {
    const char* status_str = "UNKNOWN";
    switch (status_) {
        case Status::STRAIGHT:          status_str = "==直行=="; break;
        case Status::TURN_LEFT_TINY:    status_str = "←微左1"; break;
        case Status::TURN_LEFT_SOFT:    status_str = "←轻左2"; break;
        case Status::TURN_LEFT_MID:     status_str = "←中左3"; break;
        case Status::TURN_LEFT_HARD:    status_str = "←重左4"; break;
        case Status::TURN_LEFT_SHARP:   status_str = "←←急左"; break;
        case Status::TURN_RIGHT_TINY:   status_str = "微右1→"; break;
        case Status::TURN_RIGHT_SOFT:   status_str = "轻右2→"; break;
        case Status::TURN_RIGHT_MID:    status_str = "中右3→"; break;
        case Status::TURN_RIGHT_HARD:   status_str = "重右4→"; break;
        case Status::TURN_RIGHT_SHARP:  status_str = "急右→→"; break;
        case Status::LOST_LINE:         status_str = "❌丢线"; break;
        case Status::STOPPED:           status_str = "■停止"; break;
    }
    
    float diff = left_normalized_ - right_normalized_;
    float avg = (left_normalized_ + right_normalized_) / 2.0f;
    
    Debug_Printf("[%s] L:%.1f R:%.1f Diff:%.1f Avg:%.1f\r\n", 
                 status_str, 
                 left_normalized_, 
                 right_normalized_,
                 diff,
                 avg);
}
