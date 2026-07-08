#ifndef _BSP_ADC_H
#define _BSP_ADC_H

#include "stm32f10x.h"
#include "math.h"

#define    ADC_APBxClock_FUN             RCC_APB2PeriphClockCmd
#define    GPIO_APBxClock_FUN            RCC_APB2PeriphClockCmd
#define    DMA_APBxClock_FUN             RCC_AHBPeriphClockCmd

#define    ADC_CLK                       RCC_APB2Periph_ADC1
#define    GPIO_CLK                      RCC_APB2Periph_GPIOA
#define    DMA_CLK                       RCC_AHBPeriph_DMA1
                   
#define    ADC_GPIO_PORT                 GPIOA

#define    ADCx							 ADC1

#define    ADC_PIN1                      GPIO_Pin_1
#define    ADC_CHANNEL1                  ADC_Channel_1
#define    ADC_PIN2                      GPIO_Pin_2
#define    ADC_CHANNEL2                  ADC_Channel_2
#define    ADC_PIN3                      GPIO_Pin_3
#define    ADC_CHANNEL3                  ADC_Channel_3

extern uint16_t ADCx_Value[3];
#define  READ_TIMES				10

void ADCx_Init(void);

float ADCx_PPM(void);
	
#endif



