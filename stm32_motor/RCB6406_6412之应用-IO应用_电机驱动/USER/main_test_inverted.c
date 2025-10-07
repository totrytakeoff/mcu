/***************************************************************************
 * 极性反转测试 - 高电平脉冲（与前面相反）
 * 
 * 有些电调/驱动板需要：
 * - 默认：LOW (0V)
 * - 脉冲：HIGH (3.3V)
 * - 脉宽：1500us (中位/停止)
 * - 周期：20ms (50Hz)
 * 
 * 如果这个版本能工作，说明需要反转信号极性
 ***************************************************************************/

#include "stm32f10x.h"

void Delay(__IO u32 nCount)
{
    for(; nCount != 0; nCount--);
}

void delay_us_rough(uint32_t us)
{
    uint32_t count = us * 7;
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
    
    // 默认低电平（反转极性）
    GPIO_ResetBits(GPIOC, GPIO_Pin_6);
}

int main(void)
{
    SystemInit();
    GPIO_Config_Test();
    
    Delay(0x200000);
    
    while (1)
    {
        // 发送 50Hz 的中位脉冲（1500us 高电平）
        GPIO_SetBits(GPIOC, GPIO_Pin_6);    // 拉高（脉冲开始）
        delay_us_rough(1500);                 // 保持 1.5ms
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);  // 拉低（脉冲结束）
        delay_us_rough(18500);                // 补足到 20ms
    }
}
