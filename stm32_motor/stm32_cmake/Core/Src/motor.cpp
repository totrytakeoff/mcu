#include "../Inc/motor.hpp"

Motor::Motor(TIM_HandleTypeDef* htim, uint32_t channel)
        : htim_(htim), channel_(channel), initialized_(true) {}

void Motor::init(TIM_HandleTypeDef* htim, uint32_t channel) {
    htim_ = htim;
    channel_ = channel;
    initialized_ = true;
    __HAL_TIM_SET_COMPARE(htim_, channel_, 1500);
}



// speed range: -100 to 100
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