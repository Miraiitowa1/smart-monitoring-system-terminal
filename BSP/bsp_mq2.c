#include "bsp_adc.h"
#include "bsp_delay.h"
#include "bsp_mq2.h"
#include "math.h"

/**
  * 函    数：MQ2_Init()
  * 参    数：无
  * 返 回 值：无
  * 说    明：配置MQ2的引脚和ADC
  */
void MQ2_Init(void)
{
	RCC_APB2PeriphClockCmd(MQ2_AO_GPIO_CLK, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = MQ2_AO_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 					//模拟输入
	
	GPIO_Init(MQ2_AO_GPIO_PORT, &GPIO_InitStructure);
	
	ADCx_Init();
}

/**
  * 函    数：AO模式 MQ2_ADC_Read()
  * 参    数：无
  * 返 回 值：无
  * 说    明：AO模式下读取MQ2传感器的ADC值，返回转换结果
  */
uint16_t MQ2_ADC_Read(void)
{
	//设置指定ADC的规则组通道，采样时间
	return ADCx_GetValue(ADC_CHANNEL_MQ2_AO, ADC_SampleTime_55Cycles5);
}

/**
  * 函    数：MQ2_GetData()
  * 参    数：无
  * 返 回 值：ADC值 或 数字输入状态(引脚电平)
  * 说    明：AO返回MQ2传感器的ADC值，DO返回数字输入状态
  */
uint16_t MQ2_GetData(void)
{
    uint32_t  tempData = 0;
    for (uint8_t i = 0; i < MQ2_READ_TIMES; i++)
    {
      tempData += MQ2_ADC_Read();
      DelayMs(5);
    }

    tempData /= MQ2_READ_TIMES;
    return tempData;
}

/**
  * 函    数：MQ2_GetData_PPM()
  * 参    数：无
  * 返 回 值：MQ2传感器的PPM值
  * 说    明：计算并返回MQ2传感器检测到的气体浓度（PPM）
  */
float MQ2_GetData_PPM(void)
{
    float  tempData = 0;

    for (uint8_t i = 0; i < MQ2_READ_TIMES; i++)
    {
      tempData += MQ2_ADC_Read();
      DelayMs(5);
    }
    tempData /= MQ2_READ_TIMES;
    
    float Vol = (tempData * 5 / 4096);
    float RS = (5 - Vol) / ( Vol * 0.5);
    float R0=6.64;
    
    float ppm = pow(11.5428 * R0 / RS, 0.6549f);
    return ppm;
}

