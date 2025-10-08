/**
 * @file    motor.cpp
 * @brief   Motor control class implementation
 * @author  Migrated from stm32_cmake project
 * @date    2024
 * 
 * Implementation of Motor class for DC motor control via PWM
 */

#include "../include/motor.hpp"

Motor::Motor(TIM_HandleTypeDef* htim, uint32_t channel)
        : htim_(htim), channel_(channel), initialized_(true) {}

void Motor::init(TIM_HandleTypeDef* htim, uint32_t channel) {
    htim_ = htim;
    channel_ = channel;
    initialized_ = true;
    __HAL_TIM_SET_COMPARE(htim_, channel_, 1500);
}

/**
 * @brief Set motor speed
 * @param speed Speed range: -100 to 100
 * 
 * PWM pulse calculation:
 * - Neutral (stop): 1500us
 * - Forward max (+100): 1750us (1500 + 100 * 5/2)
 * - Reverse max (-100): 1250us (1500 - 100 * 5/2)
 */
void Motor::setSpeed(int speed) {
    if (!initialized_) {
        return;
    }
    __HAL_TIM_SET_COMPARE(htim_, channel_, 1500 + speed * 5 / 2);
    speed_ = speed;
}

void Motor::maxSpeed() {
    if (!initialized_) {
        return;
    }
    __HAL_TIM_SET_COMPARE(htim_, channel_, 1750);
    speed_ = 100;
}

void Motor::reverse() {
    if (!initialized_) {
        return;
    }
    __HAL_TIM_SET_COMPARE(htim_, channel_, 1500 - speed_ * 5 / 2);
    speed_ = -speed_;
}

void Motor::stop() {
    if (!initialized_) {
        return;
    }
    __HAL_TIM_SET_COMPARE(htim_, channel_, 1500);
    speed_ = 0;
}
