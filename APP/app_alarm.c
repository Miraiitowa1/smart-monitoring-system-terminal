#include "app_alarm.h"

/**
  * 函    数：Alarm_Init
  * 参    数：无
  * 返 回 值：无
  * 说    明：报警初始化
**/
void Alarm_Init(void)
{
	RCC_APB2PeriphClockCmd(ALARM_GPIO_CLK, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin= BUZZER_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(ALARM_GPIO_PORT, &GPIO_InitStructure);

	ALARM_OFF; 
}
