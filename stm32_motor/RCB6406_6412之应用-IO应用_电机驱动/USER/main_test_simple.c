/***************************************************************************
 * 最简单的 GPIO 测试 - 用于验证 PC6 引脚是否能正常输出
 * 
 * 现象：PC6 引脚应该以 1Hz 闪烁（1秒高电平，1秒低电平）
 * 测量：用万用表测量 PC6，应该看到 0V ↔ 3.3V 变化
 ***************************************************************************/

#include "stm32f10x.h"

void Delay(__IO u32 nCount)
{
    for(; nCount != 0; nCount--);
}

void GPIO_Config_Test(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能 GPIOC 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    // 配置 PC6 为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    // 默认输出高电平
    GPIO_SetBits(GPIOC, GPIO_Pin_6);
}

int main(void)
{
    SystemInit();
    GPIO_Config_Test();
    
    while (1)
    {
        // 拉高 PC6 → 3.3V
        GPIO_SetBits(GPIOC, GPIO_Pin_6);
        Delay(0x500000);  // 约 1 秒
        
        // 拉低 PC6 → 0V
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);
        Delay(0x500000);  // 约 1 秒
    }
}
