#ifndef __APP_INIT
#define __APP_INIT

#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "bsp_delay.h"
#include "bsp_key.h"
#include "bsp_oled.h"
#include "bsp_dht11.h"
#include "bsp_led.h"
#include "bsp_adc.h"

#include "app_alarm.h"

#include "esp8266.h"
#include "onenet.h"

extern char PUBLIS_BUF[256];
extern const char devPubTopic[];
extern const char *devSubTopic[];
extern unsigned char *dataPtr;
extern uint16_t TimeCount;
extern uint8_t Alarm_flag;

void App_Init(void);


#endif
