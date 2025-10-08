/***************************************************************************
E49模式配置应用函数库。
****************************************************************************/
#include "GPIO.h"


 /***************  配置E49模式选择用到的I/O口 *******************/
void E49_GPIO_Config(void)	
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE); // 使能PA端口时钟  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA端口
	GPIO_SetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);
  //GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);	 // 置GPIOA6、7为低电平
}



