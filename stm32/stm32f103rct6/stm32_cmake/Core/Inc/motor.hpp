#include <cstdint>
#include "main.hpp"
// Define the STM32 device type for HAL library
#define STM32F103xE

class Motor {
public:
    Motor() = default;
    Motor(TIM_HandleTypeDef* htim, uint32_t channel);

    void init(TIM_HandleTypeDef* htim, uint32_t channel);

    void setSpeed(int speed);  // speed range: -100 to 100

    void stop();

    void reverse();

    void maxSpeed();

private:
    TIM_HandleTypeDef* htim_;
    uint32_t channel_;
    bool initialized_ = false;
    int speed_ = 0;
};
