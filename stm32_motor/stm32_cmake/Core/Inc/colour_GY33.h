/*
 * colour_GY33.h
 *
 *  Created on: Oct 5, 2024
 *      Author: 17212
 */

#ifndef SRC_COLOUR_GY33_H_
#define SRC_COLOUR_GY33_H_

typedef enum
{
	serial_baud_9600 = 0xAE,
	serial_baud_115200 = 0xAF
}serial_baud_e;

typedef enum
{
	no_output = 0,
	output_RGB = 1,
	output_LCC = 2,
	output_RGB_and_LCC = 3,
	output_RGBC = 4,
	output_RGBC_and_RGB = 5,
	output_RGBC_and_LCC = 6,
	output_all = 7
}serial_output_cfg_e;

typedef enum
{
	LED_lum_10 = 0,
	LED_lum_9,
	LED_lum_8,
	LED_lum_7,
	LED_lum_6,
	LED_lum_5,
	LED_lum_4,
	LED_lum_3,
	LED_lum_2,
	LED_lum_1,
	LED_lum_0
}LED_lum_e;

typedef enum
{
	colour_red = 1<<0,
	colour_yellow = 1<<1,
	colour_pink = 1<<2,
	colour_white = 1<<3,
	colour_black = 1<<4,
	colour_green = 1<<5,
	colour_dark_blue = 1<<6,
	colour_blue = 1<<7,
}colour_e;

extern uint16_t colour_RGBC[4], colour_LCC[3],colour_RGB[3];//输出数据缓冲区

void colour_GY33_init(UART_HandleTypeDef *huart, serial_baud_e serial_baud, serial_output_cfg_e serial_output_cfg, LED_lum_e LED_lum);//初始化颜色传感器
int get_colour_LCC(uint16_t *lum, uint16_t *ctmp, uint8_t *colour);//获取亮度、色温、简单颜色数据，操作正确返回1否则返回0
#endif /* SRC_COLOUR_GY33_H_ */
