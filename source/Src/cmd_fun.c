
#include "cmd_fun.h"
#include "usart.h"
#include "command.h"
#include "board.h"


uint8_t send_flag=0;

typedef struct{
  char * var_name;
  void * value_ptr;
}Var_Edit_Struct;

extern uint8_t First_Time_Check;
Var_Edit_Struct Var_List[10]={
  {"first",&First_Time_Check}
};


void set_val(int arg_num,char ** s,float * args){
  void * edit_value;
  if(arg_num!=0x0201){
    uprintf("error arg_num!\r\n");
    return ;
  }

  for(int i=0;i<sizeof(Var_List)/sizeof(Var_Edit_Struct);++i){
    if(compare_string(Var_List[i].var_name,s[0])){
      edit_value=Var_List[i].value_ptr;
      break;
    }
  }
  
  if(compare_string(s[1],"u8")){
    *(uint8_t *)edit_value=(uint8_t)args[0];
    uprintf("ok set %s = %d\r\n",s[0],*(uint8_t *)edit_value);  
  }else if(compare_string(s[1],"int")){
    *(int16_t *)edit_value=(int16_t)args[0];
    uprintf("ok set %s = %d\r\n",s[0],*(int16_t *)edit_value);
  }else if(compare_string(s[1],"f")){
    *(float *)edit_value=args[0];
    uprintf("ok set %s = %f\r\n",s[0],*(float *)edit_value);
  }
}

void get_hall_state(int arg_num,char ** s,float * args){
  if(arg_num!=0x0000){
    uprintf("error arg_num!\r\n");
    return ;
  }
  uprintf("gpio-A3:%d\r\n\
gpio-A4:%d\r\n\
gpio-A5:%d\r\n",\
  HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3),
  HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4),
  HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5));
}

void set_fd6288(int arg_num,char **s,float *args){
  int io_state=0;
  // set_fd a h 1
  if(arg_num!=0x0201){
    uprintf("error arg_num\r\n!");
    return ;
  }
  
  io_state=(int)args[0];
  if(compare_string(s[1],"h")){
    switch(s[0][0]){
    case 'a':
      Set_Phase_High_Speed(A,100*io_state);
      break;
    case 'b':
      Set_Phase_High_Speed(B,100*io_state);
      break;
    case 'c':
      Set_Phase_High_Speed(C,100*io_state);
      break;      
    default:
      uprintf("error phase!\r\n");
      return ;
    }
    
  }else{
    switch(s[0][0]){
    case 'a':
      Set_Phase_Low_State(A,io_state);
      break;
    case 'b':
      Set_Phase_Low_State(B,io_state);
      break;
    case 'c':
      Set_Phase_Low_State(C,io_state);
      break;
    default:
      uprintf("error phase!\r\n");
      return ;      
    }
  }
  uprintf("ok,set %s-%s ->  %d,\r\n",s[0],s[1],io_state);
  
}
