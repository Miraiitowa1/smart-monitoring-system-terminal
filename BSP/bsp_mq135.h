#ifndef __BSP_MQ135_H
#define __BSP_MQ135_H

//MQ135传感器ADC循环次数
#define MQ135_READ_TIMES 10

//AO模式 MQ135传感器引脚定义
#define MQ135_AO_GPIO_CLK     RCC_APB2Periph_GPIOA
#define MQ135_AO_GPIO_PORT    GPIOA
#define MQ135_AO_GPIO_PIN     GPIO_Pin_3
//MQ135传感器连接到ADC通道0
#define ADC_CHANNEL_MQ135_AO  ADC_Channel_3

//函数声明
void MQ135_Init(void);
uint16_t MQ135_GetData(void);
float MQ135_GetData_PPM(void);

#endif


