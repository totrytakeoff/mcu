/**
 * @file    motion_profile.hpp
 * @brief   梯形速度轮廓（直线加减速与反向刹车）
 */

#ifndef MOTION_PROFILE_HPP
#define MOTION_PROFILE_HPP

#include "stm32f1xx_hal.h"
#include <algorithm>

class MotionProfile {
public:
    MotionProfile()
        : target_(0)
        , current_(0)
        , acceleration_(5)
        , deceleration_(8)
        , reverseDeceleration_(12)
        , lastUpdateMs_(0)
        , updateIntervalMs_(20) {}

    void setTarget(int target)
    {
        if (target > 100) target = 100;
        if (target < -100) target = -100;
        target_ = target;
    }

    int getTarget() const { return target_; }
    int getCurrent() const { return current_; }

    void setParams(int acceleration, int deceleration, int reverseDeceleration)
    {
        acceleration_ = std::max(1, acceleration);
        deceleration_ = std::max(1, deceleration);
        reverseDeceleration_ = std::max(1, reverseDeceleration);
    }

    void setUpdateInterval(uint32_t intervalMs)
    {
        if (intervalMs < 10u) intervalMs = 10u;
        updateIntervalMs_ = intervalMs;
    }

    void reset()
    {
        target_ = 0;
        current_ = 0;
    }

    // 按时间片更新当前速度，返回更新后的 current
    int update(uint32_t nowMs)
    {
        if (nowMs - lastUpdateMs_ < updateIntervalMs_) {
            return current_;
        }
        lastUpdateMs_ = nowMs;

        if (current_ == target_) {
            return current_;
        }

        const bool reversing = (current_ > 0 && target_ < 0) || (current_ < 0 && target_ > 0);
        if (reversing) {
            if (current_ > 0) {
                current_ = std::max(0, current_ - reverseDeceleration_);
            } else {
                current_ = std::min(0, current_ + reverseDeceleration_);
            }
            return current_;
        }

        if (abs(current_) < abs(target_)) {
            // 加速阶段
            if (target_ > current_) {
                current_ = std::min(target_, current_ + acceleration_);
            } else {
                current_ = std::max(target_, current_ - acceleration_);
            }
        } else {
            // 减速阶段
            if (target_ > current_) {
                current_ = std::min(target_, current_ + deceleration_);
            } else {
                current_ = std::max(target_, current_ - deceleration_);
            }
        }
        return current_;
    }

private:
    int target_;
    int current_;
    int acceleration_;
    int deceleration_;
    int reverseDeceleration_;
    uint32_t lastUpdateMs_;
    uint32_t updateIntervalMs_;
};

#endif // MOTION_PROFILE_HPP

