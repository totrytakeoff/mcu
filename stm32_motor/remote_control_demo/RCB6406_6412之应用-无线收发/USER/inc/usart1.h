#ifndef __USART1_H
#define	__USART1_H

#include "stm32f10x.h"
#include <stdio.h>

void USART1_Config(void);
void UART1Test(void);
void UART1SendByte(unsigned char SendData);
#endif /* __USART1_H */
