#ifndef __FLAME_H
#define __FLAME_H

#include "stm32f10x.h"

//FLAME传感器ADC循环次数
#define FLAME_READ_TIMES 10

//AO模式 FLAME传感器引脚定义
#define FLAME_AO_GPIO_CLK     			RCC_APB2Periph_GPIOA
#define FLAME_AO_GPIO_PORT    			GPIOA
#define FLAME_AO_GPIO_PIN     			GPIO_Pin_2
//FLAME传感器连接到ADC通道0
#define ADC_CHANNEL_FLAME_AO  			ADC_Channel_2

//函数声明
void FLAME_Init(void);
uint16_t FLAME_FireData(void);

#endif


