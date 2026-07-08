#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"
#include "stm32f10x_usart.h"

#define USART_DEBUG		USART1		

//函数声明
static void USART1_Init(unsigned int baud);                //调试
static void USART3_Init(unsigned int baud);                //ESP8266通信

void Usart_Init(void);
void Usart_SendString(USART_TypeDef *UARTx, unsigned char *str, unsigned short len);
void Usart_Printf(USART_TypeDef *USARTx, char *fmt,...);


#endif
