/**
 * @file    bluetooth_control_example.cpp
 * @brief   蓝牙遥控差速转向完整示例（仅供参考）
 * @author  AI Assistant
 * @date    2024
 * 
 * 硬件要求：
 * - HC-05/HC-06 蓝牙模块连接到 USART1
 * - TX -> PA10 (USART1_RX)
 * - RX -> PA9  (USART1_TX)
 * - VCC -> 3.3V/5V
 * - GND -> GND
 * 
 * 蓝牙协议：
 * - 波特率: 9600
 * - 数据格式: 每包4字节
 *   [0xAA][直行速度][转向速度][0x55]
 * - 速度范围: 0-200 (100为中点)
 */

#include "stm32f1xx_hal.h"
#include "common.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"  // 需要自己创建串口配置
#include "motor.hpp"
#include "drive_train.hpp"

// 蓝牙接收缓冲区
#define BT_RX_BUFFER_SIZE 4
uint8_t btRxBuffer[BT_RX_BUFFER_SIZE];
volatile bool newDataReceived = false;

// 全局机器人实例（中断中访问）
DriveTrain* g_robot = nullptr;

/**
 * @brief 数值映射函数
 */
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief 限制数值范围
 */
int constrain(int value, int min_val, int max_val)
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

/**
 * @brief 解析并执行蓝牙命令
 */
void processBTCommand(uint8_t* buffer)
{
    // 验证数据包格式
    if (buffer[0] != 0xAA || buffer[3] != 0x55) {
        return;  // 无效数据包，忽略
    }
    
    // 提取速度值 (0-200, 100为中点)
    int straightRaw = buffer[1];
    int turnRaw = buffer[2];
    
    // 转换到 -100 ~ +100 范围
    int straightSpeed = constrain(straightRaw - 100, -100, 100);
    int turnSpeed = constrain(turnRaw - 100, -100, 100);
    
    // 特殊命令: 都为0表示紧急停止
    if (straightRaw == 0 && turnRaw == 0) {
        if (g_robot) {
            g_robot->stop();
        }
        return;
    }
    
    // 发送命令给机器人
    if (g_robot) {
        g_robot->drive(straightSpeed, turnSpeed);
    }
}

/**
 * @brief 串口接收完成回调
 * @note 接收到完整数据包时调用
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        newDataReceived = true;
        
        // 立即处理命令
        processBTCommand(btRxBuffer);
        
        // 重新启动接收等待下一包
        HAL_UART_Receive_IT(&huart1, btRxBuffer, BT_RX_BUFFER_SIZE);
    }
}

/**
 * @brief 串口错误回调
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        // 清除错误标志并重新启动接收
        __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | 
                              UART_CLEAR_PEF | UART_CLEAR_FEF);
        HAL_UART_Receive_IT(&huart1, btRxBuffer, BT_RX_BUFFER_SIZE);
    }
}

/**
 * @brief 安全看门狗 - 长时间无命令时自动停车
 * @note 建议每100ms调用一次
 */
uint32_t lastCommandTime = 0;
const uint32_t WATCHDOG_TIMEOUT_MS = 500;  // 500ms超时

void safetyWatchdog(DriveTrain& robot)
{
    if (newDataReceived) {
        lastCommandTime = HAL_GetTick();
        newDataReceived = false;
    } else {
        // 检查超时
        if (HAL_GetTick() - lastCommandTime > WATCHDOG_TIMEOUT_MS) {
            robot.stop();  // 500ms无命令则自动停车
        }
    }
}

/**
 * @brief 主函数（蓝牙遥控版本）
 */
extern "C" int main(void)
{
    /* HAL 初始化 */
    HAL_Init();
    SystemClock_Config();
    
    /* 外设初始化 */
    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();  // 初始化蓝牙串口
    
    /* 启动所有电机通道的PWM */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    
    /* 初始化电机 */
    Motor leftFront(&htim3, TIM_CHANNEL_1);
    Motor leftBack(&htim3, TIM_CHANNEL_3);
    Motor rightFront(&htim3, TIM_CHANNEL_2);
    Motor rightBack(&htim3, TIM_CHANNEL_4);
    
    /* 创建小车控制对象 */
    DriveTrain robot(leftFront, leftBack, rightFront, rightBack);
    g_robot = &robot;  // 设置全局指针供中断访问
    
    /* 启动串口中断接收 */
    HAL_UART_Receive_IT(&huart1, btRxBuffer, BT_RX_BUFFER_SIZE);
    
    /* 发送启动消息 */
    const char* startMsg = "STM32 Robot Ready!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)startMsg, strlen(startMsg), 100);
    
    /* 初始化看门狗计时器 */
    lastCommandTime = HAL_GetTick();
    
    /* 主循环 */
    while (1)
    {
        // 安全看门狗 - 失联自动停车
        safetyWatchdog(robot);
        
        // 可选：添加状态LED闪烁
        // HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        
        // 小延时防止CPU过载
        HAL_Delay(10);  // 100Hz更新率
    }
}

/**
 * ============================================================================
 * Python测试脚本（通过USB转串口测试）
 * ============================================================================
 * 
 * import serial
 * import time
 * 
 * ser = serial.Serial('COM3', 9600)  # 根据实际端口修改
 * 
 * def send_command(straight, turn):
 *     """发送控制命令 (两个参数都是 -100 到 100)"""
 *     s = int(straight + 100)  # 转换为 0-200
 *     t = int(turn + 100)
 *     packet = bytes([0xAA, s, t, 0x55])
 *     ser.write(packet)
 * 
 * # 测试序列
 * send_command(50, 0)    # 前进
 * time.sleep(2)
 * send_command(50, 30)   # 前进+左转
 * time.sleep(2)
 * send_command(0, 0)     # 停止
 * 
 * ============================================================================
 */
