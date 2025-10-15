/**
 * @file    keys.h
 * @brief   按键检测与处理 - 支持多键同时按下和消息队列
 * @author  AI Assistant
 * @date    2024
 */

#ifndef __KEYS_H__
#define __KEYS_H__

// ========== 按键位定义 (位图) ==========
#define KEY_BIT_FORWARD     0x01  // Bit 0
#define KEY_BIT_BACK        0x02  // Bit 1
#define KEY_BIT_LEFT        0x04  // Bit 2
#define KEY_BIT_RIGHT       0x08  // Bit 3
#define KEY_BIT_SPEED_UP    0x10  // Bit 4
#define KEY_BIT_SPEED_DOWN  0x20  // Bit 5
#define KEY_BIT_F1          0x40  // Bit 6
#define KEY_BIT_F2          0x80  // Bit 7
// F3, F4 在第二个字节
#define KEY_BIT_F3          0x0100  // Bit 8
#define KEY_BIT_F4          0x0200  // Bit 9

// ========== 按键码定义 (ASCII) ==========
#define KEY_CODE_FORWARD    'F'
#define KEY_CODE_BACK       'B'
#define KEY_CODE_LEFT       'L'
#define KEY_CODE_RIGHT      'R'
#define KEY_CODE_SPEED_UP   'U'
#define KEY_CODE_SPEED_DOWN 'D'
#define KEY_CODE_F1         'W'
#define KEY_CODE_F2         'X'
#define KEY_CODE_F3         'Y'
#define KEY_CODE_F4         'Z'

// ========== 消息队列配置 ==========
#define MSG_QUEUE_SIZE      16    // 队列大小 (必须是 2 的幂)
#define MSG_QUEUE_MASK      (MSG_QUEUE_SIZE - 1)

// ========== 消息类型 ==========
typedef struct {
    unsigned char cmd;     // 命令字符 ('F', 'B', 'L', 'R' 等)
    unsigned char flags;   // 标志位 (预留)
} KeyMessage;

/**
 * @brief 初始化按键系统
 */
void Keys_Init(void);

/**
 * @brief 按键扫描任务 (需在主循环中频繁调用)
 * @note  每 5ms 调用一次，检测按键状态并生成消息
 */
void Keys_Task(void);

/**
 * @brief 从消息队列中获取一条消息
 * @param msg 消息指针
 * @return 1=成功, 0=队列空
 */
unsigned char Keys_GetMessage(KeyMessage *msg);

/**
 * @brief 检查消息队列是否为空
 * @return 1=空, 0=非空
 */
unsigned char Keys_IsQueueEmpty(void);

/**
 * @brief 获取当前按下的按键位图
 * @return 16位按键状态 (1=按下, 0=松开)
 */
unsigned int Keys_GetState(void);

/**
 * @brief 延时函数 (毫秒)
 * @param ms 延时毫秒数
 */
void Delay_ms(unsigned int ms);

/**
 * @brief 微秒延时 (用于软件串口等)
 * @param us 延时微秒数
 */
void Delay_us(unsigned char us);

#endif // __KEYS_H__
