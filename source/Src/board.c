#include "board.h"

#define LOW_CLOSE       IO_Low
#define LOW_OPEN        (IO_State)!LOW_CLOSE

Phase_State Now_Phase={A,B};
uint8_t phase_loop_cnt=0;

Phase_State Phase_States[6]={
  {A,B},
  {A,C},
  {B,C},
  {B,A},
  {C,A},
  {C,B}
};

void Phase_Loop(){
  
}
void Phase_Change(Phase_State target,float speed){
  if(Now_Phase.High==target.High){
    // 例如AC->AB
    // 改变低桥臂
    Set_Phase_Low_State(Now_Phase.Low,LOW_CLOSE);
    Set_Phase_Low_State(target.Low,LOW_OPEN);
  }else{
    // 例如AC->BC
    // 改变高桥臂
    Set_Phase_High_Speed(Now_Phase.High,0);
    Set_Phase_High_Speed(target.High,speed);
  }
  Now_Phase=target;
  phase_loop_cnt++;
  if(phase_loop_cnt>5){
    phase_loop_cnt=0;
  }
  //assert(Now_Phase==Phase_States[phase_loop_cnt]
  if(Now_Phase.High==Phase_States[phase_loop_cnt].High&&Now_Phase.Low==\
    Phase_States[phase_loop_cnt].Low){
      uprintf("ok\r\n");
    }else{
      uprintf("error!\r\n");
    }
}

void Set_Phase_Low_State(Phase phase,IO_State state){
  switch(phase){
  case A:
    Set_AL_State(state);
    break;
  case B:
    Set_BL_State(state);
    break;
  case C:
    Set_CL_State(state);
    break;
  }
}

void Set_Phase_High_Speed(Phase phase,float speed){
  speed=speed<0?-speed:speed;
  switch(phase){
  case A:
    Set_AH_Speed(speed);
    break;
  case B:
    Set_BH_Speed(speed);
    break;
  case C:
    Set_CH_Speed(speed);
    break;
  }
}
