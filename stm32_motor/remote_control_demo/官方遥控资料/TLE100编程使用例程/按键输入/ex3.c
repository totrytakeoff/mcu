#include<reg51.h>  //����51��Ƭ���Ĵ��������ͷ�ļ�
#include "intrins.h"
#define uint unsigned int 
#define uchar unsigned char 

#define true  1 
#define false 0 
//��������ģʽ�˿ڶ���	
sbit  WX_M0=P1^1;
sbit  WX_M1=P1^2;	

//�����
sbit Left    = P0^0;
sbit Right   = P0^1;
sbit Forward = P0^2;
sbit Back    = P0^3;
//�Ӽ��ټ�
sbit UpSpeed   = P0^4;
sbit DownSpeed = P0^5;
//���ܼ�
sbit F1 = P0^6;
sbit F2 = P0^7;
sbit F3 = P1^3;
sbit F4 = P1^4;
//���뿪��
sbit S1 = P2^0;
sbit S2 = P2^1;
sbit S3 = P2^2;
sbit S4 = P2^3;
sbit S5 = P2^4;
sbit S6 = P2^5;
sbit S7 = P2^6;
sbit S8 = P2^7;
//���ڳ�ʼ��
void init_9600(void)
{
		TMOD = 0x20;			
		TH1 = 0xFD;		
		TL1 = 0xFD;
		SCON = 0x50;			
		PCON &= 0xef;		
		TR1 = 1;				
	
		IE = 0x0;		

}
//����һ���ֽ�
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
void send_string(char *p)
{
	char *pchar;

	pchar = p;
	while (*pchar != '\0')
	{
		send_char(*pchar++);		
	}
}
//��ʱ��0��ʼ��
void T0_INIT(void)
{
		TMOD &= 0xF0; /* Timer 0 mode 1 with software gate */ 
		TMOD |= 0x01; /* GATE0=0; C/T0#=0; M10=0; M00=1; */ 
		TH0 = 0x00; /* init values */ 
		TL0 = 0x00; 
		ET0=1; /* enable timer0 interrupt */ 
		//EA=1; /* enable interrupts */ 
		//TR0=1; /* timer0 run */
}
//�ⲿ�ж�1��ʼ��
void INT1_INIT(void)
{
	
	IT1=1;//�������жϲ���
	EX1=1;//�����ⲿ1�ж�
	//EA=1;
}
 void delay_nus(unsigned int i)  
{ 
  i=i/10;
  while(--i);
}   

void delay_nms(unsigned int n)  
{ 
  n=n+1;
  while(--n)  
  delay_nus(900);         
}   
//��ʱ����1����
void delay1ms()
{
   unsigned char i,j;	
	 for(i=0;i<10;i++)
	  for(j=0;j<33;j++)
	   ;		 
 }

//��ʱ������n����
 void delaynms(unsigned int n)
 {
		unsigned int i;
		for(i=0;i<n;i++)
	  delay1ms();
 }

//������
void main(void)
{
		init_9600();
		WX_M0=0;//����ģʽ
		WX_M1=0;
		while(1)
		{	
				//			
				if(Left==0)
				{
						send_string("Left");
				}
				if(Right==0)
				{
						send_string("Right");
				}
				if(Forward==0)
				{
						send_string("Forward");
				}
				if(Back==0)
				{
						send_string("Back");
				}
				if(UpSpeed==0)
				{
						send_string("UpSpeed");
				}
				if(DownSpeed==0)
				{
						send_string("DownSpeed");
				}
				if(F1==0)
				{
						send_string("F1");
				}
				if(F2==0)
				{
						send_string("F2");
				}
				if(F3==0)
				{
						send_string("F3");
				}
				if(F4==0)
				{
						send_string("F4");
				}
				delaynms(200);
		}
		
		
}

//�ⲿ�ж�1�жϷ�����
void it_INT1(void) interrupt 2 
{ 
	IE1 = 0; 

}
//��ʱ��0�жϷ�����
void it_timer0(void) interrupt 1 
{ 
	TF0 = 0; 

}