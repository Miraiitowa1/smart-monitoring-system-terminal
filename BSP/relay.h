#ifndef __RELAY_H
#define __RELAY_H

//引脚定义
#define RELAY_GPIO_CLK		RCC_APB2Periph_GPIOA
#define RELAY_GPIO_PIN		GPIO_Pin_0
#define RELAY_GPIO_PORT		GPIOA

//继电器控制宏定义
#define RELAY_ON			GPIO_SetBits(RELAY_GPIO_PORT, RELAY_GPIO_PIN);
#define RELAY_OFF	        GPIO_ResetBits(RELAY_GPIO_PORT, RELAY_GPIO_PIN);

//函数声明
void Relay_Init(void);

#endif