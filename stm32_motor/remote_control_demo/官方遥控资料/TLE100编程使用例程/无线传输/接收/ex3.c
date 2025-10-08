#include<reg51.h>  //����51��Ƭ���Ĵ��������ͷ�ļ�
#include "intrins.h"
#define uint unsigned int 
#define uchar unsigned char 

#define true  1 
#define false 0 
	
sbit  WX_M0=P1^2;
sbit  WX_M1=P1^1;

sbit trig=P2^0;//�����˿�
sbit echo=P3^3;//�źŽ��ն˿�
sbit tes = P1^0;//���Զ˿�
sbit zhou_1 = P0^0;//������ƶ˿�
sbit zhou_2 = P0^1;//������ƶ˿�
sbit zhou_3 = P0^2;//������ƶ˿�
sbit zhou_4 = P0^3;//������ƶ˿�
sbit zhou_5 = P0^4;//������ƶ˿�
sbit zhou_6 = P0^5;//������ƶ˿�
sbit zhou_7 = P0^6;//������ƶ˿�
sbit zhou_8 = P0^7;//������ƶ˿�

//���ٵ��
sbit MOTO1_A = P2^0;//������ƶ˿�
sbit MOTO1_B = P2^1;//������ƶ˿�
sbit MOTO2_A = P2^2;//������ƶ˿�
sbit MOTO2_B = P2^3;//������ƶ˿�
sbit MOTO3_A = P2^4;//������ƶ˿�
sbit MOTO3_B = P2^5;//������ƶ˿�
sbit MOTO4_A = P2^6;//������ƶ˿�
sbit MOTO4_B = P2^7;//������ƶ˿�

uchar FX_MOTO1_A=0;
uchar FX_MOTO1_B=0;
uchar FX_MOTO2_A=0;
uchar FX_MOTO2_B=0;
uchar FX_MOTO3_A=0;
uchar FX_MOTO3_B=0;
uchar FX_MOTO4_A=0;
uchar FX_MOTO4_B=0;

uchar flag=0;
uchar RX_DATA=0;


uint M_t_long=0;
uint t_long1=0;
uchar time_h,time_l;//ʱ��ֵ
uchar distance_h,distance_l;//����ֵ
uint time,distance;//ʱ����ֵ��������ֵ
uint d;
uchar d0,d1,d2,d3;
void delay_nus(unsigned int i);
//���ڳ�ʼ��
void init_9600(void)
{
			SCON= 0x40;              //���ڷ�ʽ1
		PCON=0;                  //SMOD=0
		REN=1;                   //�������
		TMOD= 0x20;              //��ʱ��1��ʱ��ʽ2
		TH1= 0xfd;               //11.0592M 9600������
		TL1= 0xfd;
		IE |= 0x90 ; 						 //Enable Serial Interrupt
		TR1= 1;                  //������ʱ��
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
		TH0 = 0xb8; /* init values */ 
		TL0 = 0x00; 
		ET0=1; /* enable timer0 interrupt */ 
		//EA=1; /* enable interrupts */ 
		TR0=1; /* timer0 run */
}
//��ʱ��1��ʼ��
void T1_INIT(void)
{
		TMOD &= 0x0F; /* Timer 0 mode 1 with software gate */ 
		TMOD |= 0x10; /* GATE0=0; C/T0#=0; M10=0; M00=1; */ 
		TH1 = 0xFC; /* init values */ 
		TL1 = 0x66; 
		ET1=1; /* enable timer0 interrupt */ 
		//EA=1; /* enable interrupts */ 
		TR1=1; /* timer0 run */
}
//�ⲿ�ж�1��ʼ��
void INT1_INIT(void)
{
	
	IT1=1;//�������жϲ���
	EX1=1;//�����ⲿ1�ж�
	//EA=1;
}
 void delay_100us(unsigned int n)  //n=11
{ 
 while(--n);	 
}   

void delay_nus(unsigned int m)  //m*100us
{ 
  
  while(--m)
	{
		delay_100us(11);  
	}		
        
}   
//��ʱ����1����
void delay1ms()
{
   unsigned char i,j;	
	 for(i=0;i<3;i++)
	  for(j=0;j<29;j++)
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
	  uint a,b,c,x,y;
	  uchar d;
		EA = 0;
		init_9600();
		
	
		WX_M0=0;//����ģʽ
		WX_M1=0;
		EA = 1;
		delaynms(10);
		send_string("ceshi jieshou!");
		while(1)
		{		
			  if(flag==1)
				{
						flag=0;
						send_char(RX_DATA);
					EA = 1;
						//send_char(0x88);
				}
		//delaynms(10);
		}
		
		
}

//���ڽ����жϺ���
void serial () interrupt 4 using 3
{
		if (RI) //��ʼ����
		{
				EA = 0;
			flag=1;
				RI = 0 ; //���RI=0
				RX_DATA=SBUF;
				
		}
		
}	

