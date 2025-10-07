/***************************************************************************
 * 50Hz PWM 测试 - 完全匹配 Arduino 的信号时序
 * 
 * 信号规格：
 * - 默认：HIGH (3.3V)
 * - 脉冲：LOW (0V)
 * - 脉宽：1500us (中位/停止)
 * - 周期：20ms (50Hz)
 * 
 * 用示波器应该看到：周期 20ms，低电平持续 1.5ms
 ***************************************************************************/

#include "stm32f10x.h"

void Delay(__IO u32 nCount)
{
    for(; nCount != 0; nCount--);
}

// 粗略的微秒延时（基于 72MHz，需要实测校准）
void delay_us_rough(uint32_t us)
{
    // 72MHz 下，1us 约 72 个时钟周期
    // 考虑函数调用开销，粗略估计每次循环约 10 个周期
    uint32_t count = us * 7;  // 7 次循环 ≈ 1us
    while (count--);
}

void GPIO_Config_Test(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    // 默认高电平（与 Arduino 一致）
    GPIO_SetBits(GPIOC, GPIO_Pin_6);
}

int main(void)
{
    SystemInit();
    GPIO_Config_Test();
    
    Delay(0x200000);  // 上电稳定
    
    while (1)
    {
        // 发送 50Hz 的中位脉冲（1500us 低电平）
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);  // 拉低
        delay_us_rough(1500);                 // 保持 1.5ms
        GPIO_SetBits(GPIOC, GPIO_Pin_6);    // 拉高
        delay_us_rough(18500);                // 补足到 20ms
    }
}
