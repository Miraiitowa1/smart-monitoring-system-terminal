#include "bsp_delay.h"

//计数
static uint32_t UsCount = 9;										//1us = 9个时钟周期
static uint32_t MsCount = 9000;										//1ms = 1000us

/**
  * 函    数：Delay_Init
  * 参    数：无
  * 返 回 值：无
  * 说    明：SysTick初始化
**/
void Delay_Init(void)
{
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;		//72MHz时钟源进行8分频，得到9MHz的时钟频率	
}

/**
  * 函    数：SysTick_Delay
  * 参    数：Ticks -- 延时计数
  * 返 回 值：无
  * 说    明：
**/
static void SysTick_Delay(uint32_t Ticks)
{	
	if(Ticks > 0xFFFFFF)
		Ticks = 0xFFFFFF;
	if(Ticks == 0)
		Ticks = 1;
	
	SysTick->LOAD = Ticks - 1;								    //装载重装计数值
	SysTick->VAL = 0;											//清空当前计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;					//使能SysTick定时器
	
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));		//等待计数器归零
	
	SysTick->CTRL = 0;											//关闭SysTick定时器
	SysTick->VAL = 0;											//清空计数器
}

/**
  * 函    数：DelayUs
  * 参    数：Us
  * 返 回 值：无
  * 说    明：
**/
void DelayUs(unsigned short Us)
{
	SysTick_Delay(Us * UsCount);
}

/**
  * 函    数：Delayms
  * 参    数：Ms
  * 返 回 值：无
  * 说    明：
**/
void DelayMs(unsigned short Ms)
{
	SysTick_Delay(Ms * MsCount);
}

