/***************************************************************************
E49ģʽ����Ӧ�ú����⡣
****************************************************************************/
#include "GPIO.h"


 /***************  ����E49ģʽѡ���õ���I/O�� *******************/
void E49_GPIO_Config(void)	
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE); // ʹ��PA�˿�ʱ��  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA�˿�
	GPIO_SetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);
  //GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);	 // ��GPIOA6��7Ϊ�͵�ƽ
}



