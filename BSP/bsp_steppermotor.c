#include "bsp_steppermotor.h"
#include "bsp_delay.h"

/**
  * 函    数：Stepper_Motor_Init()
  * 参    数：无
  * 返 回 值：无
  */
void Stepper_Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //使能GPIO时钟
    RCC_APB2PeriphClockCmd(STEPPER_MOTOR_CLK, ENABLE);

    //配置GPIO引脚为推挽输出
    GPIO_InitStructure.GPIO_Pin = STEPPER_MOTOR_A | STEPPER_MOTOR_B | STEPPER_MOTOR_C | STEPPER_MOTOR_D;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    //初始化GPIO
    GPIO_Init(STEPPER_MOTOR_GPIO, &GPIO_InitStructure);

    //初始状态，所有引脚低电平
    STEPPER_MOTOR_A_LOW;
    STEPPER_MOTOR_B_LOW;
    STEPPER_MOTOR_C_LOW;
    STEPPER_MOTOR_D_LOW;
}

/**
  * 函    数：Stepper_Motor_Rhythm_4_1_4()
  * 参    数：step - 步骤，dly - 延迟时间
  * 返 回 值：无
  */
void Stepper_Motor_Rhythm_4_1_4(uint8_t step, uint8_t dly)
{
    switch(step)
    {
        case 1:
            //1 0 0 0
            STEPPER_MOTOR_A_HIGH; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_LOW;        //1
            break;
        case 2:
            //0 1 0 0
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_HIGH; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_LOW;       //2
            break;  
        case 3:
            //0 0 1 0
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_HIGH; STEPPER_MOTOR_D_LOW;        //3
            break;
        case 4:
            //0 0 0 1
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_HIGH;       //4
            break;
    }
    DelayMs(dly);
}

/**
  * 函    数：Stepper_Motor_Rhythm_4_2_4()
  * 参    数：step - 步骤，dly - 延迟时间
  * 返 回 值：无
  */
void Stepper_Motor_Rhythm_4_2_4(uint8_t step, uint8_t dly)
{
    switch(step)
    {
        case 1:
            //1 0 0 1
            STEPPER_MOTOR_A_HIGH; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_HIGH;        //1
            break;
        case 2:
            //1 1 0 0
            STEPPER_MOTOR_A_HIGH; STEPPER_MOTOR_B_HIGH; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_LOW;       //2
            break;  
        case 3:
            //0 1 1 0
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_HIGH; STEPPER_MOTOR_C_HIGH; STEPPER_MOTOR_D_LOW;        //3
            break;
        case 4:
            //0 0 1 1
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_HIGH; STEPPER_MOTOR_D_HIGH;       //4
            break;
    }
    DelayMs(dly);
}

/**
  * 函    数：tepper_Motor_Rhythm_4_1_8()
  * 参    数：step - 步骤，dly - 延迟时间
  * 返 回 值：无
  */
void Stepper_Motor_Rhythm_4_1_8(uint8_t step, uint8_t dly)
{
    switch(step)
    {
        case 1:
            //1 0 0 0
            STEPPER_MOTOR_A_HIGH; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_LOW;        //1
            break;
        case 2:
            //1 1 0 0
            STEPPER_MOTOR_A_HIGH; STEPPER_MOTOR_B_HIGH; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_LOW;       //2
            break;  
        case 3:
            //0 1 0 0
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_HIGH; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_LOW;        //3
            break;
        case 4:
            //0 1 1 0
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_HIGH; STEPPER_MOTOR_C_HIGH; STEPPER_MOTOR_D_LOW;       //4
            break;
        case 5:
            //0 0 1 0
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_HIGH; STEPPER_MOTOR_D_LOW;        //5
            break;
        case 6:
            //0 0 1 1
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_HIGH; STEPPER_MOTOR_D_HIGH;       //6
            break;
        case 7:
            //0 0 0 1
            STEPPER_MOTOR_A_LOW; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_HIGH;        //7
            break;
        case 8:
            //1 0 0 1
            STEPPER_MOTOR_A_HIGH; STEPPER_MOTOR_B_LOW; STEPPER_MOTOR_C_LOW; STEPPER_MOTOR_D_HIGH;       //8
            break;
    }
    DelayMs(dly);
}

/**
  * 函    数：Stepper_Motor_Direction()
  * 参    数：dir - 转动方向，mode - 转动模式，dly - 每步之间的延迟时间
  * 返 回 值：无
  */
void Stepper_Motor_Direction(uint8_t dir, uint8_t mode, uint8_t dly) 
{
    //dir: 1正转，0反转
    //mode: 0:418, 1:414, 2:424, 3:停止
    //dly: 每步之间的延迟时间，单位为毫秒
    if (dir)
    {
        switch(mode)
        {
            case 0:
                for (uint8_t i = 1; i < 9; i++) 
                {
                    Stepper_Motor_Rhythm_4_1_8(i, dly);
                }
                break;
            case 1:
                for (uint8_t i = 1; i < 5; i++) 
                {
                    Stepper_Motor_Rhythm_4_1_4(i, dly);
                }
                break;
            case 2:
                for (uint8_t i = 1; i < 5; i++) 
                {
                    Stepper_Motor_Rhythm_4_2_4(i, dly);
                }
                break;
            default: break;
        }
    }
    else 
    {
        switch(mode)
        {
            case 0:
                for (uint8_t i = 8; i > 0; i--) 
                {
                    Stepper_Motor_Rhythm_4_1_8(i, dly);
                }
                break;
            case 1:
                for (uint8_t i = 4; i > 0; i--) 
                {
                    Stepper_Motor_Rhythm_4_1_4(i, dly);
                }
                break;
            case 2:
                for (uint8_t i = 4; i > 0; i--) 
                {
                    Stepper_Motor_Rhythm_4_2_4(i, dly);
                }
                break;
            default: break;
        }
    }
}

/**
  * 函    数：Stepper_Motor_Direction_Angle()
  * 参    数：dir - 转动方向，mode - 转动模式，angle - 转动角度，dly - 每步之间的延迟时间
  * 返 回 值：无
  */
void Stepper_Motor_Direction_Angle(uint8_t dir, uint8_t mode, uint16_t angle, uint8_t dly) 
{
    //dir: 1正转，0反转
    //mode: 0:418, 1:414, 2:424, 3:停止
    //angle: 转动角度，单位为度
    //dly: 每步之间的延迟时间，单位为毫秒

    uint32_t total = (uint32_t)angle * 64 * 64 / 360;         //根据转动角度计算总步数，64*64是每转的总步数，除以360是为了根据角度计算步数                       
    uint8_t rhythm_mode = mode == 0 ? 8 : 4;                  //根据转动模式确定每步的节奏模式
    uint32_t cycles = total / rhythm_mode;                    //计算需要循环的次数，每cycle包含rhythm_mode步

    if (total % rhythm_mode != 0) 
    {
        cycles ++;
    }

    for (uint32_t i = 0; i < cycles; i++) 
    {
        Stepper_Motor_Direction(dir, mode, dly);
    }
}

/**
  * 函    数：Stepper_Motor_Stop()
  * 参    数：无
  * 返 回 值：无
  */
void Stepper_Motor_Stop(void) 
{
    //停止转动，所有引脚低电平
    STEPPER_MOTOR_A_LOW;
    STEPPER_MOTOR_B_LOW;
    STEPPER_MOTOR_C_LOW;
    STEPPER_MOTOR_D_LOW;
}   
