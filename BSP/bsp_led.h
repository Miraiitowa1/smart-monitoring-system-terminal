#ifndef BSP_LED_H
#define BSP_LED_H

#include "stm32f10x.h"

//引脚定义
#define LED_GPIO_PORT               GPIOC
#define LED_GPIO_PIN                GPIO_Pin_13
#define LED_GPIO_CLK                RCC_APB2Periph_GPIOC

//LED控制宏定义
#define LED_ON                      GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN)
#define LED_OFF                     GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN)

//函数声明
void LED_Init(void);

#endif

