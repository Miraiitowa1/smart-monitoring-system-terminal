#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#include "stm32f10x.h"

#define            TIMx                   		 TIM2
#define            GENERAL_TIM_CLK               RCC_APB1Periph_TIM2
#define            GENERAL_TIM_Period            1000
#define            GENERAL_TIM_Prescaler         72 - 1
#define            GENERAL_TIM_IRQ               TIM2_IRQn
#define            GENERAL_TIM_IRQHandler        TIM2_IRQHandler


//函数声明
void GENERAL_TIM_Init(void);

#endif
