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
		
****************************************************************************/



/***************************************************************************
通过串口调试软件，向板子发送数据，板子接收到数据
后，立即回传给电脑。 波特率设置：115200
****************************************************************************/

#include "stm32f10x.h"
#include "usart1.h"
#include "GPIO.h"


//1us延时
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
	     
    SystemInit();			// 配置系统时钟为 72M 
   
		USART1_Config(); 	// USART1 配置 		
		E49_GPIO_Config();//配置E49传输模式
		GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);	 // 置GPIOA6、7为低电平
		while (1)
		{	 
				UART1Test();
			//UART1SendByte(0x99);
			
		}
}




