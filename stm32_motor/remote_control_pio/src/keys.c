/**
 * @file    keys.c
 * @brief   按键检测与处理实现 - 支持多键同时按下和消息队列
 * @author  AI Assistant
 * @date    2024
 * 
 * 功能特性:
 * - 支持同时按下多个按键
 * - 非阻塞去抖算法 (连续 3 次扫描确认)
 * - 循环消息队列 (防止丢失按键)
 * - 快速响应 (5ms 扫描间隔)
 * - 按键重复功能 (长按自动重复)
 */

#include <8052.h>
#include "keys.h"
#include "config.h"

// ========== 内部变量 ==========
static unsigned int key_state_raw = 0;      // 原始按键状态
static unsigned int key_state_stable = 0;   // 稳定的按键状态 (去抖后)
static unsigned int key_state_prev = 0;     // 上一次的稳定状态
static unsigned char key_debounce_cnt[10];  // 每个按键的去抖计数器

// 消息队列
static KeyMessage msg_queue[MSG_QUEUE_SIZE];
static unsigned char msg_head = 0;  // 队列头 (读)
static unsigned char msg_tail = 0;  // 队列尾 (写)

// 按键重复计时器
static unsigned int key_repeat_timer[10];
static unsigned char key_repeat_stage[10];  // 0=未触发, 1=延时阶段, 2=重复阶段

/**
 * @brief 延时1微秒 (11.0592MHz)
 */
void Delay_us(unsigned char us)
{
    // 11.0592MHz / 12 = 0.921MHz 指令周期
    // 1us 约等于 1 个指令周期
    while (us--)
    {
        // 空操作
    }
}

/**
 * @brief 延时1毫秒 (11.0592MHz)
 */
static void Delay1ms(void)
{
    unsigned char i, j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 33; j++);
}

/**
 * @brief 延时 n 毫秒
 */
void Delay_ms(unsigned int ms)
{
    while (ms--)
    {
        Delay1ms();
    }
}

/**
 * @brief 读取所有按键的原始状态
 * @return 16位按键位图 (1=按下, 0=松开)
 */
static unsigned int Keys_ReadRaw(void)
{
    unsigned int keys = 0;
    
    // 读取 P0 口的 8 个按键 (低电平有效)
    if (KEY_FORWARD == 0)    keys |= KEY_BIT_FORWARD;
    if (KEY_BACK == 0)       keys |= KEY_BIT_BACK;
    if (KEY_LEFT == 0)       keys |= KEY_BIT_LEFT;
    if (KEY_RIGHT == 0)      keys |= KEY_BIT_RIGHT;
    if (KEY_SPEED_UP == 0)   keys |= KEY_BIT_SPEED_UP;
    if (KEY_SPEED_DOWN == 0) keys |= KEY_BIT_SPEED_DOWN;
    if (KEY_F1 == 0)         keys |= KEY_BIT_F1;
    if (KEY_F2 == 0)         keys |= KEY_BIT_F2;
    
    // 读取 P1 口的 2 个按键
    if (KEY_F3 == 0)         keys |= KEY_BIT_F3;
    if (KEY_F4 == 0)         keys |= KEY_BIT_F4;
    
    return keys;
}

/**
 * @brief 获取按键对应的字符码
 */
static unsigned char Keys_BitToChar(unsigned int key_bit)
{
    switch (key_bit)
    {
        case KEY_BIT_FORWARD:    return KEY_CODE_FORWARD;
        case KEY_BIT_BACK:       return KEY_CODE_BACK;
        case KEY_BIT_LEFT:       return KEY_CODE_LEFT;
        case KEY_BIT_RIGHT:      return KEY_CODE_RIGHT;
        case KEY_BIT_SPEED_UP:   return KEY_CODE_SPEED_UP;
        case KEY_BIT_SPEED_DOWN: return KEY_CODE_SPEED_DOWN;
        case KEY_BIT_F1:         return KEY_CODE_F1;
        case KEY_BIT_F2:         return KEY_CODE_F2;
        case KEY_BIT_F3:         return KEY_CODE_F3;
        case KEY_BIT_F4:         return KEY_CODE_F4;
        default:                 return 0;
    }
}

/**
 * @brief 将消息放入队列
 * @param cmd 命令字符
 * @return 1=成功, 0=队列满
 */
static unsigned char Keys_PushMessage(unsigned char cmd)
{
    unsigned char next_tail = (msg_tail + 1) & MSG_QUEUE_MASK;
    
    // 队列满
    if (next_tail == msg_head)
        return 0;
    
    msg_queue[msg_tail].cmd = cmd;
    msg_queue[msg_tail].flags = 0;
    msg_tail = next_tail;
    
    return 1;
}

/**
 * @brief 初始化按键系统
 */
void Keys_Init(void)
{
    unsigned char i;
    
    // 清零所有状态
    key_state_raw = 0;
    key_state_stable = 0;
    key_state_prev = 0;
    
    for (i = 0; i < 10; i++)
    {
        key_debounce_cnt[i] = 0;
        key_repeat_timer[i] = 0;
        key_repeat_stage[i] = 0;
    }
    
    // 清空消息队列
    msg_head = 0;
    msg_tail = 0;
}

/**
 * @brief 按键扫描任务 (需在主循环中频繁调用)
 * @note  建议每 5ms 调用一次
 */
void Keys_Task(void)
{
    unsigned char i;
    unsigned int key_bit;
    unsigned int key_changed;
    unsigned int key_pressed;
    unsigned char key_index;
    
    // 1. 读取原始按键状态
    key_state_raw = Keys_ReadRaw();
    
    // 2. 去抖处理 (对每个按键独立去抖)
    for (i = 0; i < 10; i++)
    {
        key_bit = (1 << i);
        key_index = i;
        
        // 检测当前按键状态
        if (key_state_raw & key_bit)
        {
            // 按键按下
            if (key_debounce_cnt[key_index] < KEY_DEBOUNCE_COUNT)
            {
                key_debounce_cnt[key_index]++;
                
                // 达到去抖次数，确认按下
                if (key_debounce_cnt[key_index] >= KEY_DEBOUNCE_COUNT)
                {
                    key_state_stable |= key_bit;
                }
            }
        }
        else
        {
            // 按键松开
            if (key_debounce_cnt[key_index] > 0)
            {
                key_debounce_cnt[key_index]--;
                
                // 计数归零，确认松开
                if (key_debounce_cnt[key_index] == 0)
                {
                    key_state_stable &= ~key_bit;
                    
                    // 重置重复状态
                    key_repeat_stage[key_index] = 0;
                    key_repeat_timer[key_index] = 0;
                }
            }
        }
    }
    
    // 3. 检测按键变化 (边沿检测)
    key_changed = key_state_stable ^ key_state_prev;
    key_pressed = key_changed & key_state_stable;  // 新按下的按键
    
    // 4. 生成按键消息 (按下事件)
    for (i = 0; i < 10; i++)
    {
        key_bit = (1 << i);
        
        if (key_pressed & key_bit)
        {
            // 新按下的按键，立即发送消息
            Keys_PushMessage(Keys_BitToChar(key_bit));
            
            // 启动重复计时器
            key_repeat_stage[i] = 1;  // 延时阶段
            key_repeat_timer[i] = KEY_REPEAT_DELAY / KEY_SCAN_INTERVAL;
        }
    }
    
    // 5. 处理按键重复 (长按自动重复)
    for (i = 0; i < 10; i++)
    {
        key_bit = (1 << i);
        
        // 只处理持续按下的按键
        if ((key_state_stable & key_bit) && key_repeat_stage[i] > 0)
        {
            if (key_repeat_timer[i] > 0)
            {
                key_repeat_timer[i]--;
            }
            else
            {
                // 定时器到期
                if (key_repeat_stage[i] == 1)
                {
                    // 延时阶段结束，进入重复阶段
                    key_repeat_stage[i] = 2;
                    key_repeat_timer[i] = KEY_REPEAT_RATE / KEY_SCAN_INTERVAL;
                }
                else
                {
                    // 重复阶段，持续发送消息
                    Keys_PushMessage(Keys_BitToChar(key_bit));
                    key_repeat_timer[i] = KEY_REPEAT_RATE / KEY_SCAN_INTERVAL;
                }
            }
        }
    }
    
    // 6. 更新前一次状态
    key_state_prev = key_state_stable;
}

/**
 * @brief 从消息队列中获取一条消息
 */
unsigned char Keys_GetMessage(KeyMessage *msg)
{
    // 队列空
    if (msg_head == msg_tail)
        return 0;
    
    // 取出消息
    msg->cmd = msg_queue[msg_head].cmd;
    msg->flags = msg_queue[msg_head].flags;
    
    // 移动队列头
    msg_head = (msg_head + 1) & MSG_QUEUE_MASK;
    
    return 1;
}

/**
 * @brief 检查消息队列是否为空
 */
unsigned char Keys_IsQueueEmpty(void)
{
    return (msg_head == msg_tail);
}

/**
 * @brief 获取当前按下的按键位图
 */
unsigned int Keys_GetState(void)
{
    return key_state_stable;
}
