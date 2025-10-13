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

�Ա�������

		https://shop497043480.taobao.com�����ڽ��裩

****************************************************************************/


/***************************************************************************

����˵����

    ���������ÿ��ư�RCB6406/RCB6412,������ΪADC 6ͨ���ɼ�����
		
****************************************************************************/ 

#include "stm32f10x.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

// ADC1ת���ĵ�ѹֵͨ��MDA��ʽ����SRAM
extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];

// �ֲ����������ڱ���ת�������ĵ�ѹֵ 	 
float ADC_ConvertedValueLocal[NOFCHANEL];        

// �����ʱ
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
} 

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{		
	// ���ô���
	USART_Config();
	
	// ADC ��ʼ��
	ADCx_Init();
	
	printf("\r\n ----����һ��ADC��ͨ���ɼ�ʵ��----\r\n");
	
	while (1)
	{	
    
			ADC_ConvertedValueLocal[0] =(float) ADC_ConvertedValue[0]/4096*3.3;//��ӦADC3��ֵ
			ADC_ConvertedValueLocal[1] =(float) ADC_ConvertedValue[1]/4096*3.3;//��ӦADC4��ֵ
			ADC_ConvertedValueLocal[2] =(float) ADC_ConvertedValue[2]/4096*3.3;//��ӦADC5��ֵ
			ADC_ConvertedValueLocal[3] =(float) ADC_ConvertedValue[3]/4096*3.3;//��ӦADC6��ֵ
			ADC_ConvertedValueLocal[4] =(float) ADC_ConvertedValue[4]/4096*3.3;//��ӦADC7��ֵ
			ADC_ConvertedValueLocal[5] =(float) ADC_ConvertedValue[5]/4096*3.3;//��ӦADC8��ֵ
		
			printf("\r\n CH0 value = %f V \r\n",ADC_ConvertedValueLocal[0]);
			printf("\r\n CH1 value = %f V \r\n",ADC_ConvertedValueLocal[1]);
			printf("\r\n CH2 value = %f V \r\n",ADC_ConvertedValueLocal[2]);
			printf("\r\n CH3 value = %f V \r\n",ADC_ConvertedValueLocal[3]);
			printf("\r\n CH4 value = %f V \r\n",ADC_ConvertedValueLocal[4]);
			printf("\r\n CH5 value = %f V \r\n",ADC_ConvertedValueLocal[5]);
		
			printf("\r\n\r\n");
			Delay(0xffffee);		 
	}
}
/*********************************************END OF FILE**********************/

