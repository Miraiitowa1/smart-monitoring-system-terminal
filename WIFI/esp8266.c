#include "stm32f10x.h"
#include "esp8266.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include <string.h>
#include <stdio.h>


#define ESP8266_WIFI_INFO		"AT+CWJAP=\"afeiya\",\"987654321\"\r\n"			//WIFI 账户 密码

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"

unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

/**
  * 函    数：ESP8266_Clear
  * 参    数：无
  * 返 回 值：无
  * 说    明：无
**/
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

/**
  * 函    数：ESP8266_WaitRecive
  * 参    数：无
  * 返 回 值：REV_OK-接收完成		
			REV_WAIT-接收超时未完成
  * 说    明：循环调用检测是否接收完成
**/
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志

}

/**
  * 函    数：ESP8266_SendCmd
  * 参    数：Rcmd：命令
			res：需要检查的返回指令
  * 返 回 值：0-成功	1-失败
  * 说    明：
**/
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(USART1, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		DelayMs(10);
	}
	
	return 1;

}

/**
  * 函    数：ESP8266_SendData
  * 参    数：data：数据
			len：长度
  * 返 回 值：无
  * 说    明：
**/
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		Usart_SendString(USART1, data, len);		//发送设备连接请求数据
	}

}

/**
  * 函    数：ESP8266_GetIPD
  * 参    数：等待的时间(乘以10ms)
  * 返 回 值：平台返回的原始数据
  * 说    明：不同网络设备返回的格式不同，需要去调试
			如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
**/
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{	
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//搜索“IPD”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				Usart_Printf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		DelayMs(5);
        timeOut--;		//延时等待
	} while(timeOut>0);
	
	return NULL;														//超时还未找到，返回空指针

}

/**
  * 函    数：ESP8266_Init
  * 参    数：无
  * 返 回 值：无
  * 说    明：
**/
void ESP8266_Init(void)
{
		
		ESP8266_Clear();
	
		Usart_Printf(USART_DEBUG, "1. AT\r\n");
		while(ESP8266_SendCmd("AT\r\n", "OK"))
		DelayMs(500);
	
		Usart_Printf(USART_DEBUG, "2. CWMODE\r\n");
		while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		DelayMs(500);
	
		Usart_Printf(USART_DEBUG, "3. AT+CWDHCP\r\n");
		while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		DelayMs(500);
	
		Usart_Printf(USART_DEBUG, "4. CWJAP\r\n");
		while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		DelayMs(500);
	
		Usart_Printf(USART_DEBUG, "5. CIPSTART\r\n");
		while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		DelayMs(500);
	
		Usart_Printf(USART_DEBUG, "6. ESP8266 Init OK\r\n");

}

/**
  * 函    数：USART1_IRQHandler
  * 参    数：无
  * 返 回 值：无
  * 说    明：
**/
void USART1_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	{
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; 
		esp8266_buf[esp8266_cnt++] = USART2->DR;
				
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

}
