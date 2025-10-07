#ifndef __MOTO_H
#define	__MOTO_H

#include "stm32f10x.h"

/* the macro definition to trigger the MOTO on or off 
 * 1 - off
 - 0 - on
 */
#define ON  0
#define OFF 1


#define MOTO1(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_6)

#define MOTO2(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_6)

#define MOTO3(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_7);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_7)

#define MOTO4(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_7);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_7)

#define MOTO5(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_8);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_8)
					
#define MOTO6(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_8);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_8)
					
#define MOTO7(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_9);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_9)
					
#define MOTO8(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_9);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_9)


void MOTO_GPIO_Config(void);

#endif /* __MOTO_H */
