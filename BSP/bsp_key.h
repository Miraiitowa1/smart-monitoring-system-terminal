#ifndef BSP_KEY_H
#define BSP_KEY_H

#include "stm32f10x.h"

//引脚定义
#define KEY_PORT                    GPIOB 
#define KEY_PORT_CLK                RCC_APB2Periph_GPIOB

#define KEY1_PORT_PIN               GPIO_Pin_12
#define KEY2_PORT_PIN               GPIO_Pin_13
//#define KEY3_PORT_PIN               GPIO_Pin_7

//KEY控制宏定义
#define KEY1                        GPIO_ReadInputDataBit(KEY_PORT,KEY1_PORT_PIN) 
#define KEY2                        GPIO_ReadInputDataBit(KEY_PORT,KEY2_PORT_PIN) 
//#define KEY3                        GPIO_ReadInputDataBit(KEY_PORT,KEY3_PORT_PIN) 

//函数声明
void Key_Init(void);
uint8_t Key_Scan(uint8_t mode);

#endif

