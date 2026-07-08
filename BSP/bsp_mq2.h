#ifndef __BSP_MQ2_H
#define __BSP_MQ2_H

#include "stm32f10x.h"

//MQ2传感器ADC循环次数
#define MQ2_READ_TIMES 10

//AO模式 MQ2传感器引脚定义
#define MQ2_AO_GPIO_CLK     RCC_APB2Periph_GPIOA
#define MQ2_AO_GPIO_PORT    GPIOA
#define MQ2_AO_GPIO_PIN     GPIO_Pin_1
//MQ2传感器连接到ADC通道0
#define ADC_CHANNEL_MQ2_AO  ADC_Channel_1

//函数声明
void MQ2_Init(void);
uint16_t MQ2_GetData(void);
float MQ2_GetData_PPM(void);

#endif


