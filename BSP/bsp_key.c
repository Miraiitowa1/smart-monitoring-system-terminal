#include "bsp_key.h"
#include "bsp_delay.h"

/**
  * 函    数：Key_Init
  * 参    数：无
  * 返 回 值：无
  * 说    明：按键初始化
**/
void Key_Init(void) 
{ 
	RCC_APB2PeriphClockCmd(KEY_PORT_CLK, ENABLE);
	
 	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin  = KEY1_PORT_PIN | KEY2_PORT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
 	GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}


/**
  * 函    数：Key_Scan
  * 参    数：mode - 扫描模式
  * 返 回 值：按键值
  * 说    明：按键扫描
**/
uint8_t Key_Scan(uint8_t mode)
{	 
	static uint8_t key_up = 1;

	if(key_up && (KEY1 == 0 || KEY2 == 0))						//当按键被按下，并且之前没有被按下过
	{
		DelayMs(5);
		key_up = 0;
		if(KEY1 == 0)
			return 1;
		else if(KEY2==0)
			return 2;

	}
	else if(KEY1 == 1 && KEY2 == 1)				  				//当按键被松开时，设置key_up为1
		key_up = 1; 	
    
 	return 0;
}
