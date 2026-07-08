#ifndef BSP_DELAY_H
#define BSP_DELAY_H

#include "stm32f10x.h"

//函数声明
void Delay_Init(void);                              //初始化
void DelayUs(unsigned short Us);                    //微秒级延时
void DelayMs(unsigned short Ms);                    //毫秒级延时

#endif











































