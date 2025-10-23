/**
 * @file    uart.c
 * @brief   UART 串口通信实现
 * @author  AI Assistant
 * @date    2024
 */

#include <8052.h>
#include "uart.h"
#include "config.h"

/**
 * @brief 初始化 UART (9600 波特率)
 * @note  11.0592MHz 晶振，Timer1 模式2
 */
void UART_Init(void)
{
    // Timer1 配置 (波特率发生器)
    TMOD &= 0x0F;     // 清除 Timer1 配置位
    TMOD |= 0x20;     // Timer1 模式2 (8位自动重装)
    
    // 计算重装值: 9600 @ 11.0592MHz
    // 公式: TH1 = 256 - (FOSC / (32 * 12 * 波特率))
    // TH1 = 256 - (11059200 / (32 * 12 * 9600)) = 256 - 3 = 253 = 0xFD
    TH1 = 0xFD;       // 波特率 9600
    TL1 = 0xFD;
    
    // 串口配置
    SCON = 0x50;      // 模式1 (8位 UART, 可变波特率)
                      // SM0=0, SM1=1, REN=1 (允许接收)
    
    PCON &= 0x7F;     // SMOD=0 (不倍频)
    
    TR1 = 1;          // 启动 Timer1
    
    // 禁用串口中断 (使用轮询模式)
    ES = 0;
    EA = 0;
}

/**
 * @brief 发送单个字符
 * @param ch 要发送的字符
 */
void UART_SendChar(unsigned char ch)
{
    SBUF = ch;        // 写入发送缓冲区
    while (!TI);      // 等待发送完成 (TI = 发送中断标志)
    TI = 0;           // 清除发送完成标志
}

/**
 * @brief 发送字符串
 * @param str 要发送的字符串 (以 '\0' 结尾)
 */
void UART_SendString(unsigned char *str)
{
    while (*str != '\0')
    {
        UART_SendChar(*str);
        str++;
    }
}

/**
 * @brief 发送安全数据包
 * @param cmd 命令字符
 * 
 * 数据包格式: [$][ID_HIGH][ID_LOW][CMD][CHECKSUM][*]
 * 示例: $A5F3F57*
 * 校验和 = ID_HIGH ^ ID_LOW ^ CMD
 */
void UART_SendSecureCommand(unsigned char cmd)
{
#if ENABLE_SECURITY
    unsigned char checksum;
    unsigned char id_high, id_low;
    
    // 提取设备ID的高低字节
    id_high = (DEVICE_ID >> 8) & 0xFF;
    id_low = DEVICE_ID & 0xFF;
    
    // 1. 发送起始标志
    UART_SendChar(PACKET_START);
    
    // 2. 发送设备ID（高字节）
    UART_SendChar(id_high);
    
    // 3. 发送设备ID（低字节）
    UART_SendChar(id_low);
    
    // 4. 发送命令
    UART_SendChar(cmd);
    
    // 5. 计算并发送校验和
    checksum = id_high ^ id_low ^ cmd;
    UART_SendChar(checksum);
    
    // 6. 发送结束标志
    UART_SendChar(PACKET_END);
#else
    // 不安全模式：直接发送命令
    UART_SendChar(cmd);
#endif
}
