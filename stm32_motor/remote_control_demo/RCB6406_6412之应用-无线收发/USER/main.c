/***************************************************************************

�������ƣ�

    ����ǰ��Ӣ����Ƽ���չ���޹�˾ ����ƣ�Ӣ����Ƽ���
   ��Shenzhen Qianhai Infeeon Technology Development Co., Ltd.��
		
���̼�飺

    Ӣ����Ƽ���һ�Ҵ��»���������΢�ͼ����ϵͳ�ͼ���Ӧ��ģ���Ʒ���������
�ļ�������ҵ�������û�������ϵͳ�ɿ�����㡢��Ч����ת��
   ��Infeeon Technology is a technology engaged in the development and serv-
ice of microcomputer system and integrated application module in robot field 
Technical enterprises, to help users robot system reliable, simple and effi-
cient operation.��

��ϵ��ʽ��

		13823123830��15180190599
		support_infeeon@126.com
		sales_infeeon@126.com
		�����и�������ǿ���ֵ���ǿ��·38��Ⱥ�ǹ㳡A��33¥3331
		3462348702��qq�� 13823123830��΢�ţ�
		
****************************************************************************/



/***************************************************************************
ͨ�����ڵ������������ӷ������ݣ����ӽ��յ�����
�������ش������ԡ� ���������ã�115200
****************************************************************************/

#include "stm32f10x.h"
#include "usart1.h"
#include "GPIO.h"


//1us��ʱ
void delay_us(int i)
{
		int j,m=10;
		for(j=0;j<i;j++)
		{
				for(int n=0;n<m;n++)
					;
		}
}
int main(void)
{  
	     
    SystemInit();			// ����ϵͳʱ��Ϊ 72M 
   
		USART1_Config(); 	// USART1 ���� 		
		E49_GPIO_Config();//����E49����ģʽ
		GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);	 // ��GPIOA6��7Ϊ�͵�ƽ
		while (1)
		{	 
				UART1Test();
			//UART1SendByte(0x99);
			
		}
}




