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
配置USART1。
****************************************************************************/

#include "usart1.h"
#include <stdarg.h>


void USART1_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;

		/* 使能 USART1 时钟*/
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); 

		/* USART1 使用IO端口配置 */    
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 			// 复用推挽输出
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);    
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// 浮空输入
		GPIO_Init(GPIOA, &GPIO_InitStructure);   							// 初始化GPIOA
	  
		/* USART1 工作模式配置 */
		USART_InitStructure.USART_BaudRate = 9600;					// 波特率设置：115200
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;	// 数据位数设置：8位
		USART_InitStructure.USART_StopBits = USART_StopBits_1; 			// 停止位设置：1位
		USART_InitStructure.USART_Parity = USART_Parity_No ;  			// 是否奇偶校验：无
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// 硬件流控制模式设置：没有使能
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;// 接收与发送都使能
		USART_Init(USART1, &USART_InitStructure);  // 初始化USART1
		USART_Cmd(USART1, ENABLE);// USART1使能
}

 /*发送一个字节数据*/
 void UART1SendByte(unsigned char SendData)
{	   
    USART_SendData(USART1,SendData);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	    
}  

/*接收一个字节数据*/
unsigned char UART1GetByte(unsigned char* GetData)
{   	   
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
    {  
				return 0;	// 没有收到数据 
		}
    *GetData = USART_ReceiveData(USART1); 
    return 1;			// 收到数据
}
/*接收一个数据，马上返回接收到的这个数据*/
void UART1Test(void)
{
    unsigned char i = 0;

    while(1)
    {    
				while(UART1GetByte(&i))
        {
						USART_SendData(USART1,i);
        }      
    }     
}



