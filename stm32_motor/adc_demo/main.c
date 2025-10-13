/***************************************************************************

厂商名称：

    深圳前海英飞扬科技发展有限公司 （简称：英飞扬科技）
   （Shenzhen Qianhai Infeeon Technology Development Co., Ltd.）
		
厂商简介：

    英飞扬科技是一家从事机器人领域微型计算机系统和集成应用模块产品开发与服务
的技术型企业，帮助用户机器人系统可靠、简便、高效的运转。
   （Infeeon Technology is a technology engaged in the development and serv-
ice of microcomputer system and integrated application module in robot field 
Technical enterprises, to help users robot system reliable, simple and effi-
cient operation.）

联系方式：

		13823123830，15180190599
		support_infeeon@126.com
		sales_infeeon@126.com
		深圳市福田区华强北街道华强北路38号群星广场A座33楼3331
		3462348702（qq） 13823123830（微信）

淘宝渠道：

		https://shop497043480.taobao.com（正在建设）

****************************************************************************/


/***************************************************************************

程序说明：

    本程序适用控制板RCB6406/RCB6412,本程序为ADC 6通道采集程序。
		
****************************************************************************/ 

#include "stm32f10x.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

// ADC1转换的电压值通过MDA方式传到SRAM
extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];

// 局部变量，用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal[NOFCHANEL];        

// 软件延时
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
} 

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{		
	// 配置串口
	USART_Config();
	
	// ADC 初始化
	ADCx_Init();
	
	printf("\r\n ----这是一个ADC多通道采集实验----\r\n");
	
	while (1)
	{	
    
			ADC_ConvertedValueLocal[0] =(float) ADC_ConvertedValue[0]/4096*3.3;//对应ADC3口值
			ADC_ConvertedValueLocal[1] =(float) ADC_ConvertedValue[1]/4096*3.3;//对应ADC4口值
			ADC_ConvertedValueLocal[2] =(float) ADC_ConvertedValue[2]/4096*3.3;//对应ADC5口值
			ADC_ConvertedValueLocal[3] =(float) ADC_ConvertedValue[3]/4096*3.3;//对应ADC6口值
			ADC_ConvertedValueLocal[4] =(float) ADC_ConvertedValue[4]/4096*3.3;//对应ADC7口值
			ADC_ConvertedValueLocal[5] =(float) ADC_ConvertedValue[5]/4096*3.3;//对应ADC8口值
		
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

