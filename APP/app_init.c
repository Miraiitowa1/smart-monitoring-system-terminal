#include "app_init.h"
	
DHT11_Data_TypeDef *DHT11_Data;

char oled_Temp[16], oled_TempThr[16];
char oled_Humi[16], oled_HumiThr[16];
char Flame_ppm[32], Mq2_ppm[32], Mq135_ppm[32];

uint8_t key_value = 0;
uint16_t Temp_Thr = 30;
uint16_t Humi_Thr = 60;
uint64_t Flame_Thr = 6000;
uint64_t Mq2_Thr = 6000;
uint64_t Mq135_Thr = 6000;

uint8_t TIMEOUT = 10;

typedef enum{
	MAIN_MENU, 
	TEMP_SET, 
	HUMI_SET,
	FLAME_SET,
	MQ2_SET,
	MQ135_SET,
}DisplayState;

DisplayState currentState = MAIN_MENU;

char PUBLIS_BUF[256];
const char devPubTopic[] = "$sys/XPz90SBcDh/test/thing/property/post";
const char *devSubTopic[] = {"$sys/XPz90SBcDh/test/thing/property/set"};
unsigned char *dataPtr = NULL;
uint16_t TimeCount = 0;
uint8_t Alarm_flag = 0;
	
void OLED_Show()
{
	 OLED_ShowString(24, 0, "Smart Kitchen", OLED_6X8);

	 if(DHT11_Read_TempAndHumidity(DHT11_Data) == 1)
	 {
		 sprintf(oled_Temp, "Temp:%d.%d", DHT11_Data->temp_int, DHT11_Data->temp_deci);
		 OLED_Printf(0, 10, OLED_6X8, oled_Temp);
		 sprintf(oled_Humi,"Humi:%d%%", DHT11_Data->humi_int);
		 OLED_Printf(0, 20, OLED_6X8, oled_Humi);			
	 }
	 
	 sprintf(Flame_ppm, "Flame_ppm:%.2f", ADCx_PPM());
	 OLED_Printf(0, 30, OLED_6X8, Flame_ppm);	
	 sprintf(Mq2_ppm, "Mq2_ppm:%.2f", ADCx_PPM());
	 OLED_Printf(0, 40, OLED_6X8, Mq2_ppm);
	 sprintf(Mq135_ppm, "Mq135_ppm:%.2f", ADCx_PPM());
	 OLED_Printf(0, 50, OLED_6X8, Mq135_ppm);
	 
	 OLED_Update();
}


void OLED_Show1()
{
	OLED_Printf(30, 0, OLED_8X16, "温度阈值");
	if(key_value == 2)
	{
		if(Temp_Thr < 100)
		{
			Temp_Thr++;
		}
	}
	else if(key_value == 3)
	{
		if(Temp_Thr > 0)
		{
			Temp_Thr--;
		}			
	}
	sprintf(oled_TempThr, "Temp:%d", Temp_Thr);
	OLED_Printf(30, 4, OLED_8X16, oled_TempThr);

	OLED_Update();
}

void OLED_Show2()
{
	OLED_Printf(30, 0, OLED_8X16, "湿度阈值");
	if(key_value == 2)
	{
		if(Humi_Thr < 100)
		{
			Humi_Thr++;
		}
	}
	else if(key_value == 3)
	{
		if(Humi_Thr > 0)
		{
			Humi_Thr--;
		}			
	}
	sprintf(oled_HumiThr, "Humi:%d", Humi_Thr);
	OLED_Printf(30, 4, OLED_8X16,oled_HumiThr);

	OLED_Update();
}

void OLED_Show3()
{
	OLED_Printf(30, 0, OLED_8X16, "火焰阈值");
	if(key_value == 2)
	{
		if(Flame_Thr < 10000)
		{
			Flame_Thr++;
		}
	}
	else if(key_value == 3)
	{
		if(Flame_Thr > 0)
		{
			Flame_Thr--;
		}			
	}
	sprintf(Flame_ppm, "Flame_ppm:%.2f", ADCx_PPM());
	OLED_Printf(30, 4, OLED_8X16, Flame_ppm);

	OLED_Update();
}

void OLED_Show4()
{
	OLED_Printf(30, 0, OLED_8X16, "MQ2阈值");
	if(key_value == 2)
	{
		if(Mq2_Thr < 100)
		{
			Mq2_Thr++;
		}
	}
	else if(key_value == 3)
	{
		if(Mq2_Thr > 0)
		{
			Mq2_Thr--;
		}			
	}
	sprintf(Flame_ppm, "mq2_ppm:%.2f", ADCx_PPM());
	OLED_Printf(30, 4, OLED_8X16,Mq2_ppm);

	OLED_Update();
}

void OLED_Show5()
{
	OLED_Printf(30, 0, OLED_8X16, "MQ135阈值");
	if(key_value == 2)
	{
		if(Mq135_Thr < 100)
		{
			Mq135_Thr++;
		}
	}
	else if(key_value == 3)
	{
		if(Mq135_Thr > 0)
		{
			Mq135_Thr--;
		}			
	}
	sprintf(Flame_ppm, "mq135:%.2f", ADCx_PPM());
	OLED_Printf(30, 4, OLED_8X16,Mq135_ppm);

	OLED_Update();
}

void OLED_Switch()
{
	 key_value = Key_Scan(0);
	 if(key_value == 1)
	 {
		currentState = (DisplayState)((currentState + 1) % 6);	
		OLED_Clear();
	 }

	 switch(currentState)
	 {
		case MAIN_MENU:
			OLED_Show();
			break;
		case TEMP_SET:
			OLED_Show1();
			break;
		case HUMI_SET:	
			OLED_Show2();
			break;
		case FLAME_SET:
			OLED_Show3();
			break;
		case MQ2_SET:
			OLED_Show4();
			break;
		case MQ135_SET:	
			OLED_Show5();
			break;		
	 }
}


void Alarm_Statue(void)
{
	if(Alarm_flag == 1)
	{
		if(DHT11_Data->temp_int > Temp_Thr || DHT11_Data->humi_int > Humi_Thr)
		{
			ALARM_ON;
		}
		else
		{
			ALARM_OFF;
		}
	}
	else
	{
		ALARM_OFF;		
	}
	
}

void JsonValue(void)
{	
	memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
		
	sprintf(PUBLIS_BUF,"{\"id\":\"123\",\"params\":{\"Buzzer\":{\"value\":%s},\"Temp\":{\"value\":%d},\"LED\":{\"value\":%s},\"Humi\":{\"value\":%d}}}",
	"false",DHT11_Data->temp_int,"true",DHT11_Data->humi_int);	
}

void App_Init()
{
	Delay_Init();
	OLED_Init();
	DHT11_Init();
	Key_Init();
	LED_Init();
	
	Alarm_Init();	
	ESP8266_Init();

	OLED_Printf(20, 3, OLED_8X16, "网络连接中...");	
	
	
	while(OneNet_DevLink() || TIMEOUT--)												//连接onenet，失败等待500ms进行连接
	{
		DelayMs(500);
	}
	
	OLED_Clear();		
	OLED_Printf(30, 3, OLED_8X16, "连接成功");
	DelayMs(3000);
	OLED_Clear();	
	
	OneNet_Subscribe(devSubTopic,1);									//订阅主题
	
	while(1)	
	{
		OLED_Switch();
		Alarm_Statue();
	
		if(++TimeCount >= 100)										//Oled_Switch切换
		{
			JsonValue();
			OneNet_Publish(devPubTopic, PUBLIS_BUF);
			ESP8266_Clear();
			TimeCount = 0;
		}					
		dataPtr = ESP8266_GetIPD(2);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);		
	}
}
