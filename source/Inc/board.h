#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f1xx_hal.h"

typedef enum{
  IO_Low=0x00,
  IO_High=0x01
}IO_State;

typedef enum{
  A,
  B,
  C
}Phase;

typedef struct{
  Phase High;      //¸ß¶ËMOS
  Phase Low;       //µÍ¶ËMOS
}Phase_State;

Phase_State Phase_States[6]={
  {A,B},
  {A,C},
  {B,C},
  {B,A},
  {C,A},
  {C,B}
};

typedef enum{
  AB,AC,BC,BA,CA,CB
}Phase_State_Name;

#define Set_AL_State(x)        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,(GPIO_PinState)x)
#define Set_BL_State(x)        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,(GPIO_PinState)x)
#define Set_CL_State(x)        HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,(GPIO_PinState)x)

#define Set_AH_Speed(x)        TIM2->CCR1=(uint16_t)(x*TIM2->ARR/100.0f)
#define Set_BH_Speed(x)        TIM2->CCR2=(uint16_t)(x*TIM2->ARR/100.0f)
#define Set_CH_Speed(x)        TIM2->CCR3=(uint16_t)(x*TIM2->ARR/100.0f)


#endif