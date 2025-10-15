/**
 * @file    motor.hpp
 * @brief   Motor control class definition
 * @author  Migrated from stm32_cmake project
 * @date    2024
 * 
 * Motor control class for controlling DC motors via PWM signals
 * Speed range: -100 (full reverse) to 100 (full forward)
 */

#ifndef MOTOR_HPP
#define MOTOR_HPP

#include <cstdint>
#include "stm32f1xx_hal.h"

class Motor {
public:
    Motor() = default;
    Motor(TIM_HandleTypeDef* htim, uint32_t channel);

    /**
     * @brief Initialize motor with timer and channel
     * @param htim Pointer to TIM handle
     * @param channel TIM channel (TIM_CHANNEL_1, TIM_CHANNEL_2, etc.)
     */
    void init(TIM_HandleTypeDef* htim, uint32_t channel);

    /**
     * @brief Set motor speed
     * @param speed Speed value from -100 to 100
     *              Negative values for reverse, positive for forward
     */
    void setSpeed(int speed);

    /**
     * @brief Stop the motor (neutral position)
     */
    void stop();

    /**
     * @brief Reverse current motor direction
     */
    void reverse();

    /**
     * @brief Set motor to maximum speed
     */
    void maxSpeed();

private:
    TIM_HandleTypeDef* htim_;
    uint32_t channel_;
    bool initialized_ = false;
    int speed_ = 0;
};

#endif // MOTOR_HPP
