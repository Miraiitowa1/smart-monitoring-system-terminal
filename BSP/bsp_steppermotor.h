#ifndef __BSP_STEPPERMOTOR_H
#define __BSP_STEPPERMOTOR_H

#include "stm32f10x.h"

//引脚定义
#define STEPPER_MOTOR_GPIO          GPIOB
#define STEPPER_MOTOR_A             GPIO_Pin_8
#define STEPPER_MOTOR_B             GPIO_Pin_9
#define STEPPER_MOTOR_C             GPIO_Pin_14
#define STEPPER_MOTOR_D             GPIO_Pin_15
#define STEPPER_MOTOR_CLK           RCC_APB2Periph_GPIOA

//引脚操作
#define STEPPER_MOTOR_A_HIGH   GPIO_SetBits(STEPPER_MOTOR_GPIO, STEPPER_MOTOR_A);
#define STEPPER_MOTOR_A_LOW    GPIO_ResetBits(STEPPER_MOTOR_GPIO, STEPPER_MOTOR_A);

#define STEPPER_MOTOR_B_HIGH   GPIO_SetBits(STEPPER_MOTOR_GPIO,   STEPPER_MOTOR_B);
#define STEPPER_MOTOR_B_LOW    GPIO_ResetBits(STEPPER_MOTOR_GPIO,  STEPPER_MOTOR_B);

#define STEPPER_MOTOR_C_HIGH   GPIO_SetBits(STEPPER_MOTOR_GPIO,   STEPPER_MOTOR_C);
#define STEPPER_MOTOR_C_LOW    GPIO_ResetBits(STEPPER_MOTOR_GPIO,  STEPPER_MOTOR_C);

#define STEPPER_MOTOR_D_HIGH   GPIO_SetBits(STEPPER_MOTOR_GPIO,   STEPPER_MOTOR_D);
#define STEPPER_MOTOR_D_LOW    GPIO_ResetBits(STEPPER_MOTOR_GPIO,  STEPPER_MOTOR_D);

//函数声明
void Stepper_Motor_Init(void);
void Stepper_Motor_Rhythm_4_1_4(uint8_t step, uint8_t dly);
void Stepper_Motor_Rhythm_4_2_4(uint8_t step, uint8_t dly);
void Stepper_Motor_Rhythm_4_1_8(uint8_t step, uint8_t dly);
void Stepper_Motor_Direction(uint8_t dir, uint8_t step, uint8_t dly);
void Stepper_Motor_Direction_Angle(uint8_t dir, uint8_t step, uint16_t angle, uint8_t dly); 
void Stepper_Motor_Stop(void);

#endif
