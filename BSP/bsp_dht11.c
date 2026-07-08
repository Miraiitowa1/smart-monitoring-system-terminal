#include "bsp_dht11.h"
#include "bsp_delay.h"

/**
  * 函    数：DHT11_GPIO_Config
  * 参    数：无
  * 返 回 值：无
  * 说    明: DHT11 GPIO配置
**/
static void DHT11_GPIO_Config(void)
{		
    DHT11_Dout_SCK_APBxClock_FUN(DHT11_Dout_GPIO_CLK, ENABLE);	

	GPIO_InitTypeDef GPIO_InitStructure; 
 															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

  	GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure);		  	
}


/**
  * 函    数：DHT11_Mode_IPU
  * 参    数：无
  * 返 回 值：无
  * 说    明：DHT11输入模式配置
**/
static void DHT11_Mode_IPU(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 

	GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure);	 
}


/**
  * 函    数：DHT11_Mode_Out_PP
  * 参    数：无
  * 返 回 值：无
  * 说    明：DHT11输出模式配置
**/
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_Dout_GPIO_PIN;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  	GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStructure);	 	 	
}


/**
  * 函    数：DHT11_ReadByte
  * 参    数：无
  * 返 回 值：读取到的字节数据
  * 说    明：从DHT11读取一个字节数据
**/
static uint8_t DHT11_ReadByte(void)
{
	uint8_t Data=0;
	
	for(uint8_t i = 0; i < 8; i++)    
	{	 
		/*每bit以50us低电平标置开始，轮询直到从机发出的50us低电平结束*/  
		while(DHT11_Dout_IN == Bit_RESET);

		/*DHT11 以26~28us的高电平表示“0”，以70us高电平表示“1”，
		 *通过检测 x us后的电平即可区别这两个状 ，x 即下面的延时 
		 */
		 DelayUs(40); //延时x us 这个延时需要大于数据0持续的时间即可	   	  

		if(DHT11_Dout_IN == Bit_SET)/* x us后仍为高电平表示数据“1” */
		{
			/* 等待数据1的高电平结束 */
			while(DHT11_Dout_IN == Bit_SET);

			Data |= (uint8_t)(0x01 << (7 - i));  //把第7-i位置1，MSB先行 
		}
		else	 // x us后为低电平表示数据“0”
		{			   
			Data &= (uint8_t)~(0x01 << (7 - i)); //把第7-i位置0，MSB先行
		}
	}
	return Data;
}
/**
  * 函    数：DHT11_Read_TempAndHumidity
  * 参    数：DHT11_Data - 存储温湿度数据的结构体指针
  * 返 回 值：无
  * 说    明：从DHT11读取温湿度数据
**/
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{  
	DHT11_Mode_Out_PP();											//配置为输出模式，准备发送开始信号
	DHT11_Dout_0;													//拉低数据线，发送开始信号
	DelayMs(20);													//保持数据线低电平20ms，DHT11会在此期间检测到开始信号

	DHT11_Dout_1; 													//拉高数据线，结束开始信号
	DelayUs(13);   												    //等待13us，DHT11会在此时拉低数据线，表示准备开始发送数据
	DHT11_Mode_IPU();												//配置为输入模式，准备接收数据
  
	if(DHT11_Dout_IN == Bit_RESET)     							    //如果数据线被拉低，表示DHT11已经响应，准备开始发送数据
	{
		while(DHT11_Dout_IN == Bit_RESET);						    //等待数据线变高，表示DHT11开始发送数据
		while(DHT11_Dout_IN == Bit_SET);							//等待数据线变低，表示DHT11开始发送第一个数据位	

		DHT11_Data->humi_int = DHT11_ReadByte();					//读取湿度整数部分	
		DHT11_Data->humi_deci = DHT11_ReadByte();					//读取湿度小数部分	

		DHT11_Data->temp_int = DHT11_ReadByte();					//读取温度整数部分
		DHT11_Data->temp_deci = DHT11_ReadByte();					//读取温度小数部分		

		DHT11_Data->check_sum = DHT11_ReadByte();					//读取校验和

		DHT11_Mode_Out_PP();										//配置为输出模式，准备发送结束信号
		DHT11_Dout_1;												//拉高数据线，结束通信

		//校验数据有效性，校验和应该等于湿度整数部分、湿度小数部分、温度整数部分和温度小数部分的和
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
			return SUCCESS;
		else 
			return ERROR;
	}
	else
		return ERROR;
}

	  
/**
  * 函    数：DHT11_Init
  * 参    数：无
  * 返 回 值：无
  * 说    明：DHT11初始化
**/
void DHT11_Init(void)
{
	DHT11_GPIO_Config ();											//配置GPIO引脚
	DHT11_Dout_1;              								 		//初始化时将数据线拉高，准备等待开始信号
}
