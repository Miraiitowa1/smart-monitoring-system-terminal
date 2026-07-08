#include "bsp_adc.h"
#include "bsp_filter.h"

uint16_t ADCx_Value[3];

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  */
void ADCx_Init(void)
{
	/*开启时钟*/
	ADC_APBxClock_FUN(ADC_CLK, ENABLE);						//开启ADC1的时钟
	GPIO_APBxClock_FUN(GPIO_CLK, ENABLE);					//开启GPIOA的时钟
	DMA_APBxClock_FUN(DMA_CLK, ENABLE);						//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = ADC_PIN1 | ADC_PIN2 | ADC_PIN3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStructure);					
	
	/*规则组通道配置*/
	ADC_RegularChannelConfig(ADCx, ADC_CHANNEL1, 1, ADC_SampleTime_55Cycles5);	//规则组序列1的位置，配置为通道0
	ADC_RegularChannelConfig(ADCx, ADC_CHANNEL2, 2, ADC_SampleTime_55Cycles5);	//规则组序列2的位置，配置为通道1
	ADC_RegularChannelConfig(ADCx, ADC_CHANNEL3, 3, ADC_SampleTime_55Cycles5);	//规则组序列3的位置，配置为通道2
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//外部触发，使用软件触发，不需要外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;							//连续转换，使能，每转换一次规则组序列后立刻开始下一次转换
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式，使能，扫描规则组的序列，扫描数量由ADC_NbrOfChannel确定
	ADC_InitStructure.ADC_NbrOfChannel = 3;										//通道数，为4，扫描规则组的前4个通道
	
	ADC_Init(ADCx, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1
	
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;											//定义结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;				//外设基地址，给定形参AddrA
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，对应16为的ADC数据寄存器
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，始终以ADC数据寄存器为源
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCx_Value;					//存储器基地址，给定存放AD转换结果的全局数组AD_Value
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，与源数据宽度对应
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向，选择由外设到存储器，ADC数据寄存器转到数组
	DMA_InitStructure.DMA_BufferSize = 3;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，与ADC的连续转换一致
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
	
	/*DMA和ADC使能*/
	DMA_Cmd(DMA1_Channel1, ENABLE);							//DMA1的通道1使能
	ADC_DMACmd(ADCx, ENABLE);								//ADC1触发DMA1的信号使能
	ADC_Cmd(ADCx, ENABLE);									//ADC1使能
	
	/*ADC校准*/
	ADC_ResetCalibration(ADCx);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADCx) == SET);
	ADC_StartCalibration(ADCx);
	while (ADC_GetCalibrationStatus(ADCx) == SET);
	
	/*ADC触发*/
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);	//软件触发ADC开始工作，由于ADC处于连续转换模式，故触发一次后ADC就可以一直连续不断地工作
	
}

/**
  * 函    数：ADCx_PPM
  * 参    数：无
  * 返 回 值：Flame/MQ-2/MQ-135传感器的PPM值
  * 说    明：计算并返回检测到的气体浓度（PPM）
  */
float ADCx_PPM(void)
{
	Filter *f;
	f->alaph = 0.2;
	Filter_Init(f);
	
    uint16_t adc = ADCx_Value[1];

    float voltage = adc * 5.0f / 4096.0f;
    float RS = (5.0f - voltage) / (voltage * 0.5f);
    float R0 = 6.64f;

    float ratio = RS / R0;

    // y = a * x^b
    return FilterValue(f, 11.5428f * pow(ratio, -0.6549f));
}

