/**
 * STM32F103 示例代码
 * 
 * 此代码展示如何在STM32端配置串口与ESP32-C3通信
 * 使用HAL库
 */

#include "main.h"
#include "usart.h"
#include <string.h>

// 接收缓冲区
#define RX_BUFFER_SIZE 256
uint8_t rxBuffer[RX_BUFFER_SIZE];
uint8_t rxData;
volatile uint16_t rxIndex = 0;
volatile uint8_t rxComplete = 0;

/**
 * USART1初始化（连接ESP32-C3）
 * PA9: TX -> ESP32 RX (GPIO20)
 * PA10: RX -> ESP32 TX (GPIO21)
 */
void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 启动中断接收
    HAL_UART_Receive_IT(&huart1, &rxData, 1);
}

/**
 * 串口接收完成回调
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 将接收到的数据存入缓冲区
        if (rxIndex < RX_BUFFER_SIZE)
        {
            rxBuffer[rxIndex++] = rxData;
            
            // 检测结束符（可根据实际协议修改）
            // 例如：检测换行符或特定字节
            if (rxData == '\n' || rxData == '\r')
            {
                rxComplete = 1;
            }
        }
        else
        {
            // 缓冲区满，重置
            rxIndex = 0;
        }
        
        // 继续接收下一个字节
        HAL_UART_Receive_IT(&huart1, &rxData, 1);
    }
}

/**
 * 通过蓝牙发送数据到手机
 */
void BLE_SendData(uint8_t *data, uint16_t length)
{
    HAL_UART_Transmit(&huart1, data, length, HAL_MAX_DELAY);
}

/**
 * 处理从蓝牙接收到的数据
 */
void BLE_ProcessReceivedData(void)
{
    if (rxComplete)
    {
        // 处理接收到的数据
        // 例如：解析命令、控制电机等
        
        // 示例：回显数据
        BLE_SendData(rxBuffer, rxIndex);
        
        // 示例：解析简单命令
        if (rxIndex > 0)
        {
            switch (rxBuffer[0])
            {
                case 'F': // 前进
                    // Motor_Forward();
                    BLE_SendData((uint8_t*)"Forward\n", 8);
                    break;
                    
                case 'B': // 后退
                    // Motor_Backward();
                    BLE_SendData((uint8_t*)"Backward\n", 9);
                    break;
                    
                case 'L': // 左转
                    // Motor_TurnLeft();
                    BLE_SendData((uint8_t*)"Left\n", 5);
                    break;
                    
                case 'R': // 右转
                    // Motor_TurnRight();
                    BLE_SendData((uint8_t*)"Right\n", 6);
                    break;
                    
                case 'S': // 停止
                    // Motor_Stop();
                    BLE_SendData((uint8_t*)"Stop\n", 5);
                    break;
                    
                default:
                    BLE_SendData((uint8_t*)"Unknown\n", 8);
                    break;
            }
        }
        
        // 重置缓冲区
        rxIndex = 0;
        rxComplete = 0;
        memset(rxBuffer, 0, RX_BUFFER_SIZE);
    }
}

/**
 * 主函数示例
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    // 初始化串口
    MX_USART1_UART_Init();
    
    // 发送启动消息
    BLE_SendData((uint8_t*)"STM32 Ready\n", 12);
    
    while (1)
    {
        // 处理蓝牙数据
        BLE_ProcessReceivedData();
        
        // 其他任务
        // ...
        
        HAL_Delay(10);
    }
}

/**
 * DMA方式接收（可选，性能更好）
 */
void BLE_InitDMA(void)
{
    // 使用DMA循环接收
    HAL_UART_Receive_DMA(&huart1, rxBuffer, RX_BUFFER_SIZE);
}

/**
 * 发送传感器数据示例
 */
void BLE_SendSensorData(void)
{
    char buffer[64];
    
    // 假设读取传感器数据
    uint16_t temperature = 25;  // 温度
    uint16_t humidity = 60;     // 湿度
    
    // 格式化数据
    sprintf(buffer, "T:%d,H:%d\n", temperature, humidity);
    
    // 发送到蓝牙
    BLE_SendData((uint8_t*)buffer, strlen(buffer));
}
