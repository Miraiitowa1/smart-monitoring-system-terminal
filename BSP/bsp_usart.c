#include "bsp_usart.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


/**
  * 函    数：UART1_Init ----- PA9_TX--ESP8266_RX  PA10_RX--ESP8266_TX
  * 参    数：baud -- 波特率
  * 返 回 值：无
  * 说    明：UART1初始化，用于ESP8266通信
**/
void USART1_Init(unsigned int baud)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	
	//PA9_TX--ESP8266_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//PA10_RX--ESP8266_TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = baud;												//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;			//无硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;							//收发模式
	USART_InitStructure.USART_Parity = USART_Parity_No;										//无奇偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;									//1位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;								//8位数据位
	
	USART_Init(USART1, &USART_InitStructure);												//初始化USART1
	
	USART_Cmd(USART1, ENABLE);																//使能USART1
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);											//使能接收中断
	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;										//UART1中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;											//使能中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;								//抢占优先级0	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;										//子优先级2

	NVIC_Init(&NVIC_InitStructure);
}

/**
  * 函    数：UARTt3_Init ----- PB10_TX-- SER_RX  PB11_RX--SER_TX
  * 参    数：baud -- 波特率
  * 返 回 值：无
  * 说    明：调试串口UARTT3初始化，用于串口调试
**/
void USART3_Init(unsigned int baud)
{	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	
	//PB10	TXD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//PB11	RXD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;			
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						  	
	USART_InitStructure.USART_Parity = USART_Parity_No;									
	USART_InitStructure.USART_StopBits = USART_StopBits_1;								
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	
	
	USART_Init(USART3, &USART_InitStructure);
	
	USART_Cmd(USART3, ENABLE);														
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);									
	
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

	NVIC_Init(&NVIC_InitStructure);

}

/**
  * 函    数：UART_Init
  * 参    数：无
  * 返 回 值：无
  * 说    明：初始化所有串口 设置每个串口的波特率
**/
void Usart_Init(void)
{
	 USART1_Init(115200);						//esp8266通信
	 USART3_Init(115200);						//调试串口

	return;
}

/**
  * 函    数：Usart_SendString
  * 参    数：USARTx - 发送数据的串口
  *			   pData - 发送的数据字符串
  *			   len - 发送数据的长度
  * 返 回 值：无
  * 说    明：判断发送寄存器TC和发送移位寄存器标志位TXE
**/
void Usart_SendString(USART_TypeDef *USARTx, unsigned char *pData, unsigned short len)
{
	for(uint32_t i = 0; i < len;i ++)
	{
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);			//等待TXE==SET，TDR为空
		USART_SendData(USARTx, pData[i]);										//发送数据
	}

	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);				//TC==SET，TDR和移位寄存器都为空
}


/**
  * 函    数：Usart_Printf
  * 参    数：UARTx - 发送数据的串口
  *			   fmt - 格式化字符串，类似于printf的格式
  * 返 回 值：无
  * 说    明：格式化字符串并发送
**/
void Usart_Printf(USART_TypeDef *USARTx, char *fmt,...)
 {

 	unsigned char UARTx_PrintfBuf[296];										//格式化字符串缓冲区
 	va_list ap;																//指向可变参数的指针
 	unsigned char *pData = UARTx_PrintfBuf;									//指向格式化字符串缓冲区的指针
	
 	va_start(ap, fmt);													 	//初始化指向可变参数的指针，ap指向第一个可变参数，fmt是最后一个固定参数
 	vsnprintf((char *)UARTx_PrintfBuf, sizeof(UARTx_PrintfBuf), 		 
 	fmt, ap);											  					//格式化字符串并存储在UARTPrintfBuf中，sizeof(UARTPrintfBuf)确保不会发生缓冲区溢出
 	va_end(ap);															 	//格式化字符串已经存储在UARTPrintfBuf中，pStr指向UARTPrintfBuf的起始位置
	
 	while(*pData != 0)
 	{
 		USART_SendData(USARTx, *pData++);									//发送一个字节
 		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);			//等待发送完成
 	}

 }

///**
//  * 函    数：UART1_IRQHandler
//  * 参    数：无
//  * 返 回 值：无
//  * 说    明：UART1中断服务函数，处理UART1的接收中断
//**/
//void USART1_IRQHandler(void)
//{

//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 						//接收中断
//	{
//		USART_ClearFlag(USART1, USART_FLAG_RXNE);								//清除接收中断标志位
//	}

//}

/**
  * 函    数：USART2_IRQHandler
  * 参    数：无
  * 返 回 值：无
  * 说    明：UART2中断服务函数，处理UART2的接收中断
**/
void USART3_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 						//接收中断
	{
		USART_ClearFlag(USART2, USART_FLAG_RXNE);								//清除接收中断标志位
	}

}
