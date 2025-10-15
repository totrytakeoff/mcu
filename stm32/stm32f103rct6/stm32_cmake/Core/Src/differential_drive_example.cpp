/**
 * @file differential_drive_example.cpp
 * @brief 差速转向控制系统使用示例
 * 
 * 这个文件展示了如何使用DriveTrain类实现各种差速转向控制功能
 */

#include "drive_train.hpp"
#include "motor.hpp"
#include <iostream>

// 假设的电机初始化函数
extern TIM_HandleTypeDef htim1; // 假设使用定时器1
extern TIM_HandleTypeDef htim2; // 假设使用定时器2
extern TIM_HandleTypeDef htim3; // 假设使用定时器3
extern TIM_HandleTypeDef htim4; // 假设使用定时器4

// 示例：基本差速转向控制
void basicDifferentialDriveExample() {
    // 创建电机对象
    Motor leftFront(&htim1, TIM_CHANNEL_1);
    Motor leftBack(&htim2, TIM_CHANNEL_1);
    Motor rightFront(&htim3, TIM_CHANNEL_1);
    Motor rightBack(&htim4, TIM_CHANNEL_1);
    
    // 创建动力系统对象
    DriveTrain driveTrain(leftFront, leftBack, rightFront, rightBack);
    
    // 初始化
    driveTrain.init(leftFront, leftBack, rightFront, rightBack);
    
    // 示例1：直线前进
    std::cout << "直线前进..." << std::endl;
    driveTrain.drive(50, 0);  // 50%速度前进
    HAL_Delay(2000);         // 2秒
    
    // 示例2：右转
    std::cout << "右转..." << std::endl;
    driveTrain.drive(30, 30); // 30%速度前进，30%速度右转
    HAL_Delay(2000);
    
    // 示例3：左转
    std::cout << "左转..." << std::endl;
    driveTrain.drive(30, -30); // 30%速度前进，30%速度左转
    HAL_Delay(2000);
    
    // 示例4：原地右转
    std::cout << "原地右转..." << std::endl;
    driveTrain.turn(50);     // 50%速度原地右转
    HAL_Delay(2000);
    
    // 示例5：原地左转
    std::cout << "原地左转..." << std::endl;
    driveTrain.turn(-50);    // 50%速度原地左转
    HAL_Delay(2000);
    
    // 停止
    driveTrain.stop();
    std::cout << "停止" << std::endl;
}

// 示例：平滑转向控制
void smoothDriveExample() {
    // 创建电机对象和动力系统对象（同上）
    Motor leftFront(&htim1, TIM_CHANNEL_1);
    Motor leftBack(&htim2, TIM_CHANNEL_1);
    Motor rightFront(&htim3, TIM_CHANNEL_1);
    Motor rightBack(&htim4, TIM_CHANNEL_1);
    
    DriveTrain driveTrain(leftFront, leftBack, rightFront, rightBack);
    driveTrain.init(leftFront, leftBack, rightFront, rightBack);
    
    // 平滑加速前进
    std::cout << "平滑加速前进..." << std::endl;
    for(int i = 0; i <= 50; i += 5) {
        driveTrain.smoothDrive(i, 0, 0.2f);  // 平滑因子0.2
        HAL_Delay(100);
    }
    
    // 平滑减速停止
    std::cout << "平滑减速停止..." << std::endl;
    for(int i = 50; i >= 0; i -= 5) {
        driveTrain.smoothDrive(i, 0, 0.2f);
        HAL_Delay(100);
    }
    
    driveTrain.stop();
}

// 示例：弧线运动控制
void arcDriveExample() {
    // 创建电机对象和动力系统对象（同上）
    Motor leftFront(&htim1, TIM_CHANNEL_1);
    Motor leftBack(&htim2, TIM_CHANNEL_1);
    Motor rightFront(&htim3, TIM_CHANNEL_1);
    Motor rightBack(&htim4, TIM_CHANNEL_1);
    
    DriveTrain driveTrain(leftFront, leftBack, rightFront, rightBack);
    driveTrain.init(leftFront, leftBack, rightFront, rightBack);
    
    // 示例1：大弧线右转
    std::cout << "大弧线右转..." << std::endl;
    driveTrain.arcDrive(40, 500);  // 40%速度，转向半径500
    HAL_Delay(3000);
    
    // 示例2：小弧线左转
    std::cout << "小弧线左转..." << std::endl;
    driveTrain.arcDrive(30, -200); // 30%速度，转向半径-200
    HAL_Delay(3000);
    
    // 示例3：原地转向
    std::cout << "原地转向..." << std::endl;
    driveTrain.arcDrive(0, 0);    // 速度0，转向半径0 = 原地转向
    HAL_Delay(2000);
    
    driveTrain.stop();
}

// 示例：速度限制控制
void speedLimitExample() {
    // 创建电机对象和动力系统对象（同上）
    Motor leftFront(&htim1, TIM_CHANNEL_1);
    Motor leftBack(&htim2, TIM_CHANNEL_1);
    Motor rightFront(&htim3, TIM_CHANNEL_1);
    Motor rightBack(&htim4, TIM_CHANNEL_1);
    
    DriveTrain driveTrain(leftFront, leftBack, rightFront, rightBack);
    driveTrain.init(leftFront, leftBack, rightFront, rightBack);
    
    // 设置最大速度为50%
    std::cout << "设置最大速度为50%..." << std::endl;
    driveTrain.setMaxSpeed(50);
    
    // 尝试以80%速度前进（会被限制为50%）
    std::cout << "尝试以80%速度前进..." << std::endl;
    driveTrain.drive(80, 0);
    HAL_Delay(2000);
    
    // 尝试以60%速度右转（会被限制为50%）
    std::cout << "尝试以60%速度右转..." << std::endl;
    driveTrain.drive(40, 40);
    HAL_Delay(2000);
    
    driveTrain.stop();
}

// 示例：蓝牙遥控控制
void bluetoothRemoteControlExample() {
    // 创建电机对象和动力系统对象（同上）
    Motor leftFront(&htim1, TIM_CHANNEL_1);
    Motor leftBack(&htim2, TIM_CHANNEL_1);
    Motor rightFront(&htim3, TIM_CHANNEL_1);
    Motor rightBack(&htim4, TIM_CHANNEL_1);
    
    DriveTrain driveTrain(leftFront, leftBack, rightFront, rightBack);
    driveTrain.init(leftFront, leftBack, rightFront, rightBack);
    
    // 模拟蓝牙接收到的指令
    // 假设蓝牙协议：前进行程，转向速度
    int straightCmd = 0;
    int turnCmd = 0;
    
    while(1) {
        // 这里应该是从蓝牙接收数据的代码
        // 为了示例，我们模拟一些指令
        
        // 示例：前进
        straightCmd = 60;
        turnCmd = 0;
        driveTrain.drive(straightCmd, turnCmd);
        HAL_Delay(1000);
        
        // 示例：右转
        straightCmd = 40;
        turnCmd = 30;
        driveTrain.drive(straightCmd, turnCmd);
        HAL_Delay(1000);
        
        // 示例：左转
        straightCmd = 40;
        turnCmd = -30;
        driveTrain.drive(straightCmd, turnCmd);
        HAL_Delay(1000);
        
        // 示例：后退
        straightCmd = -40;
        turnCmd = 0;
        driveTrain.drive(straightCmd, turnCmd);
        HAL_Delay(1000);
        
        // 示例：停止
        straightCmd = 0;
        turnCmd = 0;
        driveTrain.drive(straightCmd, turnCmd);
        HAL_Delay(1000);
    }
}

// 主测试函数
void testDifferentialDrive() {
    std::cout << "=== 差速转向控制系统测试 ===" << std::endl;
    
    // 运行基本测试
    basicDifferentialDriveExample();
    
    // 运行平滑转向测试
    smoothDriveExample();
    
    // 运行弧线运动测试
    arcDriveExample();
    
    // 运行速度限制测试
    speedLimitExample();
    
    std::cout << "=== 测试完成 ===" << std::endl;
}
