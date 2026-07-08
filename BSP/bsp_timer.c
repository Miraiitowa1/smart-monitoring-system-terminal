#include "bsp_timer.h"

void GENERAL_TIM_Init(void)
{
	RCC_APB1PeriphClockCmd(GENERAL_TIM_CLK, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = GENERAL_TIM_Period;
	TIM_TimeBaseStructure.TIM_Prescaler = GENERAL_TIM_Prescaler;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
	
	TIM_ClearFlag(TIMx, TIM_IT_Update);
	
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = GENERAL_TIM_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIMx, ENABLE);
}

void GENERAL_TIM_IRQHandler(void)
{
	if(TIM_GetFlagStatus(TIMx, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
	}
}
