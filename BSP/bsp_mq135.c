#include "stm32f10x.h"
#include "bsp_adc.h"
#include "bsp_delay.h"
#include "bsp_mq135.h"
#include "math.h"

/**
  * 函    数：MQ135_Init()
  * 参    数：无
  * 返 回 值：无
  * 说    明：配置MQ135的引脚和ADC
  */
void MQ135_Init(void)
{
	//AO模式初始化
	RCC_APB2PeriphClockCmd(MQ135_AO_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = MQ135_AO_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 						//模拟输入
	
	GPIO_Init(MQ135_AO_GPIO_PORT, &GPIO_InitStructure);
	
	ADCx_Init();
}

/**
  * 函    数：AO模式 MQ135_ADC_Read()
  * 参    数：无
  * 返 回 值：无
  * 说    明：AO模式下读取MQ135传感器的ADC值，返回转换结果
  */
uint16_t MQ135_ADC_Read(void)
{
	//设置指定ADC的规则组通道，采样时间
	return ADCx_GetValue(ADC_CHANNEL_MQ135_AO, ADC_SampleTime_55Cycles5);
}

/**
  * 函    数：MQ135_GetData()
  * 参    数：无
  * 返 回 值：ADC值 或 数字输入状态(引脚电平)
  * 说    明：AO返回MQ135传感器的ADC值，DO返回数字输入状态
  */
uint16_t MQ135_GetData(void)
{
	uint32_t  tempData = 0;
	for (uint8_t i = 0; i < MQ135_READ_TIMES; i++)
	{
	  tempData += MQ135_ADC_Read();
	  DelayMs(5);
	}

	tempData /= MQ135_READ_TIMES;
	
	return tempData;
}

/**
  * 函    数：MQ135_GetData_PPM()
  * 参    数：无
  * 返 回 值：MQ135传感器的PPM值
  * 说    明：计算并返回MQ135传感器检测到的气体浓度（PPM）
  */
float MQ135_GetData_PPM(void)
{
	float  tempData = 0;

	for (uint8_t i = 0; i < MQ135_READ_TIMES; i++)
	{
	  tempData += MQ135_ADC_Read();
	  DelayMs(5);
	}
	tempData /= MQ135_READ_TIMES;
	
	float Vol = (tempData * 5 / 4096);
	float RS = (5 - Vol) / ( Vol * 0.5);
	float R0 = 6.64;
	
	float ppm = pow(11.5428 * R0 / RS, 0.6549f);
	
	return ppm;
}

