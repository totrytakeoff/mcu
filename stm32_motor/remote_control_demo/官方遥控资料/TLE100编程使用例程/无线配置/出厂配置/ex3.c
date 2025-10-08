#include<reg51.h>  //包含51单片机寄存器定义的头文件
#include "intrins.h"
#define uint unsigned int 
#define uchar unsigned char 

#define true  1 
#define false 0 
	
sbit  WX_M0=P1^2;
sbit  WX_M1=P1^1;
uchar RX_DATA=0;

void delay_nus(unsigned int i);
//串口初始化
void init_9600(void)
{
		TMOD = 0x20;			
		TH1 = 0xFD;		
		TL1 = 0xFD;
		SCON = 0x50;			
		PCON &= 0xef;		
		IE |= 0x90 ; 						 //Enable Serial Interrupt	
		TR1 = 1;
}
//发送一个字节
void send_char(unsigned char txd)
{
		SBUF = txd;
		while(!TI);			
		TI = 0;					
}
void send_one_byte_to_char(unsigned char ch)
{
	if ((ch & 0xf0) > 0x90)
	{
		send_char(((ch>>4)&0x0f) + 'A'-0x0a);
	}
	else
	{
		send_char(((ch>>4)&0x0f) + '0'-0x00);
	}
	if ((ch & 0x0f) > 0x09)
	{
		send_char((ch&0x0f) + 'A'-0x0a);
	}
	else
	{
		send_char((ch&0x0f) + '0'-0x00);
	}
}
void send_two_byte_to_char(unsigned int ch)
{
	//3
	if ((ch & 0xf000) > 0x9000)
	{
		send_char(((ch>>12)&0x000f) + 'A'-0x0a);
	}
	else
	{
		send_char(((ch>>12)&0x000f) + '0'-0x00);
	}
	//2
	if ((ch & 0x0f00) > 0x0900)
	{
		send_char(((ch>>8)&0x000f) + 'A'-0x0a);
	}
	else
	{
		send_char(((ch>>8)&0x000f) + '0'-0x00);
	}
	//1
	if ((ch & 0x00f0) > 0x0090)
	{
		send_char(((ch>>4)&0x000f) + 'A'-0x0a);
	}
	else
	{
		send_char(((ch>>4)&0x000f) + '0'-0x00);
	}
	//0
	if ((ch & 0x000f) > 0x0009)
	{
		send_char((ch&0x000f) + 'A'-0x0a);
	}
	else
	{
		send_char((ch&0x000f) + '0'-0x00);
	}
}
void send_four_byte_to_char(uchar a,uchar b,uchar c,uchar d)
{
		send_char(a+'0'-0x00);
		send_char(b+'0'-0x00);
		send_char(c+'0'-0x00);
		send_char(d+'0'-0x00);
}
//发送字符串
void send_string(char *p)
{
	char *pchar;

	pchar = p;
	while (*pchar != '\0')
	{
		send_char(*pchar++);		
	}
}

//延时100us
 void delay_100us(unsigned int n)  //n=11
{ 
 while(--n);	 
}   
//延时 m 个100us
void delay_nus(unsigned int m)  //m*100us
{ 
  
  while(--m)
	{
		delay_100us(11);  
	}		
        
}   
//延时函数1毫秒
void delay1ms()
{
   unsigned char i,j;	
	 for(i=0;i<3;i++)
	  for(j=0;j<29;j++)
	   ;		 
 }

//延时函数，n毫秒
 void delaynms(unsigned int n)
 {
   unsigned int i;
	for(i=0;i<n;i++)
	   delay1ms();
 }

//主函数
void main(void)
{
	  uint a,b,c,x,y;
	  uchar d;
		EA = 0;
		init_9600();
		
		delaynms(100);
	
		//出厂配置参数
		WX_M0=0; //配置模式
		WX_M1=1;
		delaynms(100);
	
		send_char(0xC0);	
		send_char(0x00);
		send_char(0x00);
		send_char(0x19);
		send_char(0x2E);
		send_char(0x00);
	
		delaynms(100);
		WX_M0=0;//传输模式
		WX_M1=0;
		EA = 1;
		while(1)
		{		
				//send_char(0x99);
				delaynms(1000);
					
		}				
}

//串口接收中断函数
void serial () interrupt 4 using 3
{
		if (RI) //开始接收
		{
				RI = 0 ; //软件RI=0
				RX_DATA=SBUF;
				
		}
		
}	