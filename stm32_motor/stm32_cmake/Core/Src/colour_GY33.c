/*
 * colour_GY33.c
 *
 *  Created on: Oct 5, 2024
 *      Author: 17212
 */

#include "usart.h"
#include "colour_GY33.h"

static uint8_t  colour_rx_temp=0, colour_rx_buf[20]={0};//接收中断缓冲区，接收缓冲区
static uint16_t colour_rx_count=0;//中断接收字节
static serial_output_cfg_e serial_output;//传感器输出数据类型记录
static UART_HandleTypeDef *colour_huart;//传感器挂载串行端口记录
uint16_t colour_RGBC[4]={0}, colour_LCC[3]={0},colour_RGB[3]={0};//输出数据


/*初始化颜色传感器：串口端口，串口波特率设置，串口输出数据类型设置，LED亮度设置*/
void colour_GY33_init(UART_HandleTypeDef *huart, serial_baud_e serial_baud, serial_output_cfg_e serial_output_cfg, LED_lum_e LED_lum)
{
	uint8_t uart_write_data_raw[3];

	colour_huart = huart;
	uart_write_data_raw[0] = 0xA5;
	uart_write_data_raw[1] = serial_baud;
	uart_write_data_raw[2] = uart_write_data_raw[0] + uart_write_data_raw[1];
	HAL_UART_Transmit(huart, uart_write_data_raw, 3, 0xffff);
	if(serial_baud == serial_baud_9600)
	{
		if((*huart).Init.BaudRate != 9600)
		{
			(*huart).Init.BaudRate = 9600;
			if (HAL_UART_Init(huart) != HAL_OK) Error_Handler();
		}
	}
	else //115200
	{
		if((*huart).Init.BaudRate != 115200)
		{
			(*huart).Init.BaudRate = 115200;
			if (HAL_UART_Init(huart) != HAL_OK) Error_Handler();
		}
	}

	serial_output = serial_output_cfg;
	uart_write_data_raw[0] = 0xA5;
	uart_write_data_raw[1] = 0x80 | serial_output_cfg;
	uart_write_data_raw[2] = uart_write_data_raw[0] + uart_write_data_raw[1];
	HAL_UART_Transmit(huart, uart_write_data_raw, 3, 0xffff);

	uart_write_data_raw[0] = 0xA5;
	uart_write_data_raw[1] = 0x60 | LED_lum;
	uart_write_data_raw[2] = uart_write_data_raw[0] + uart_write_data_raw[1];
	HAL_UART_Transmit(huart, uart_write_data_raw, 3, 0xffff);

	HAL_UART_Receive_IT(huart, &colour_rx_temp, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t i;//for循环使用
	uint16_t sum=0;//本地计算校验和
	static serial_output_cfg_e data_format;
	if(huart->Instance == colour_huart->Instance)
	{
		colour_rx_buf[colour_rx_count] = colour_rx_temp;
		colour_rx_count++;
		if(colour_rx_count == 2) if(colour_rx_buf[0] != 0x5A || colour_rx_buf[1] != 0x5A) colour_rx_count = 0;
		if(colour_rx_count == 3)
		{

			if(colour_rx_buf[2] == 0x15) data_format = output_RGBC;//RGBC数据格式
			if(colour_rx_buf[2] == 0x25) data_format = output_LCC;//LCC数据格式
			if(colour_rx_buf[2] == 0x45) data_format = output_RGB;//RGBC数据格式
		}
		if(data_format == output_RGBC)
		{
			if(colour_rx_count >= 13)
			{
				for(i=0;i<12;i++) sum += colour_rx_buf[i];
				if((sum & 0xff) == colour_rx_buf[12])//校验成功
				{
					colour_RGBC[0] = (uint16_t)colour_rx_buf[4]<<8 | colour_rx_buf[5];
					colour_RGBC[1] = (uint16_t)colour_rx_buf[6]<<8 | colour_rx_buf[7];
					colour_RGBC[2] = (uint16_t)colour_rx_buf[8]<<8 | colour_rx_buf[9];
					colour_RGBC[3] = (uint16_t)colour_rx_buf[10]<<8 | colour_rx_buf[11];
				}
				colour_rx_count = 0;
			}
		}
		else if(data_format == output_LCC)
		{
			if(colour_rx_count >= 11)
			{
				for(i=0;i<10;i++) sum += colour_rx_buf[i];
				if((sum & 0xff) == colour_rx_buf[10])//校验成功
				{
					colour_LCC[0] = (uint16_t)colour_rx_buf[4]<<8 | colour_rx_buf[5];
					colour_LCC[1] = (uint16_t)colour_rx_buf[6]<<8 | colour_rx_buf[7];
					colour_LCC[2] = (uint16_t)colour_rx_buf[8]<<8 | colour_rx_buf[9];
				}
				colour_rx_count = 0;
			}
		}
		else if(data_format == output_RGB)
		{
			if(colour_rx_count >= 8)
			{
				for(i=0;i<7;i++) sum += colour_rx_buf[i];
				if((sum & 0xff) == colour_rx_buf[7])//校验成功
				{
					colour_RGB[0] = colour_rx_buf[4];
					colour_RGB[1] = colour_rx_buf[5];
					colour_RGB[2] = colour_rx_buf[6];
				}
				colour_rx_count = 0;
			}
		}
		HAL_UART_Receive_IT(colour_huart, &colour_rx_temp, 1);

	}
}
