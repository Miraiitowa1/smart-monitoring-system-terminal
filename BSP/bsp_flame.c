#include "bsp_adc.h"
#include "bsp_delay.h"
#include "bsp_flame.h"
#include "math.h"

/**
  * 函    数：FLAME_Init()
  * 参    数：无
  * 返 回 值：无
  * 说    明：配置FLAME的引脚和ADC
  */
void FLAME_Init(void)
{	
	RCC_APB2PeriphClockCmd(FLAME_AO_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;	
	GPIO_InitStructure.GPIO_Pin = FLAME_AO_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 							//模拟输入
	
	GPIO_Init(FLAME_AO_GPIO_PORT, &GPIO_InitStructure);
	
	ADCx_Init();
}

/**
  * 函    数：AO模式 FLAME_ADC_Read()
  * 参    数：无
  * 返 回 值：无
  * 说    明：AO模式下读取FLAME传感器的ADC值，返回转换结果
  */
uint16_t FLAME_ADC_Read(void)
{
	//设置指定ADC的规则组通道，采样时间
	return ADCx_GetValue(ADC_CHANNEL_FLAME_AO, ADC_SampleTime_55Cycles5);
}

uint16_t FLAME_FireData(void)
{
	uint32_t  tempData = 0;
	for (uint8_t i = 0; i < FLAME_READ_TIMES; i++)
	{
		tempData += FLAME_ADC_Read();
		DelayMs(5);
	}

	tempData /= FLAME_READ_TIMES;
	return tempData;
}

