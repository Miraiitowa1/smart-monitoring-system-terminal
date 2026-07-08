#ifndef _APP_ALARM_H
#define _APP_ALARM_H

#include "stm32f10x.h"
#include "bsp_led.h"

//引脚定义
#define ALARM_GPIO_PORT			        GPIOA
#define ALARM_GPIO_CLK			        RCC_APB2Periph_GPIOA

#define BUZZER_GPIO_PIN			        GPIO_Pin_11

//报警控制宏定义
#define ALARM_ON                        GPIO_SetBits(ALARM_GPIO_PORT, BUZZER_GPIO_PIN), LED_ON;
#define ALARM_OFF                       GPIO_ResetBits(ALARM_GPIO_PORT, BUZZER_GPIO_PIN), LED_OFF; 

//函数声明
void Alarm_Init(void);

#endif
