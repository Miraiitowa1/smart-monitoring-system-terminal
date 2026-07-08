#ifndef __DHT11_H
#define	__DHT11_H

#include "stm32f10x.h"

typedef struct
{
	uint8_t  humi_int;		//湿度整数部分
	uint8_t  humi_deci;	 	//湿度小数部分
	uint8_t  temp_int;	 	//温度整数部分
	uint8_t  temp_deci;	 	//温度小数部分
	uint8_t  check_sum;	 	//校验和
		                 
} DHT11_Data_TypeDef;

//引脚定义
#define      DHT11_Dout_SCK_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define      DHT11_Dout_GPIO_CLK                       RCC_APB2Periph_GPIOB

#define      DHT11_Dout_GPIO_PORT                      GPIOB
#define      DHT11_Dout_GPIO_PIN                       GPIO_Pin_5

//控制宏定义
#define      DHT11_Dout_0	                           GPIO_ResetBits(DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN) 
#define      DHT11_Dout_1	                           GPIO_SetBits(DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN) 

#define      DHT11_Dout_IN	                           GPIO_ReadInputDataBit(DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN)

//函数声明
static void DHT11_GPIO_Config(void);
static void DHT11_Mode_IPU(void);
static void DHT11_Mode_Out_PP(void);
static uint8_t DHT11_ReadByte(void);

void DHT11_Init(void);
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data);

#endif







