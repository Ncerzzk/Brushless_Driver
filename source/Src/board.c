#include "board.h"
#include "math.h"

#define LOW_CLOSE       IO_Low
#define LOW_OPEN        (IO_State)!LOW_CLOSE

Phase_State AB={A,B};
Phase_State AC={A,C};
Phase_State BC={B,C};
Phase_State BA={B,A};
Phase_State CA={C,A};
Phase_State CB={C,B};

Mode Board_Mode=TEST;
Phase_State Now_Phase={A,B};
uint8_t phase_loop_cnt=0;
char  Phase_String[3]={'A','B','C'};

Phase_State * Phase_Table[7]={0};  // 第一个元素是空，因为霍尔没有000的状态。下标是霍尔状态。
Phase_State * Phase_Table_Reverse[7]={0}; // 反向换向表
Phase_State ** Phase_Table_Using=0; //当前使用的换向表

uint8_t Phase_Test_Table[6]={1,3,2,6,4,5};//{2,6,4,5,1,3}; // 测试换向表时记录的霍尔状态
//{5,4,6,2,3,1} //反转时的霍尔状态
//{&AB,&CB,&CA,&BA,&BC,&AC};

//将其与正转时候的霍尔状态统一，方便程序里写
//{2,6,4,5,1,3}
//{&BA,&CA,&CB,&AB,&AC,&BC}
//无刷电机的霍尔传感器一般是双极锁存型霍尔传感器，正转与反转在统一位置的霍尔值不同，刚好相反
//如正转是为010(2),反转时就是101(5)
Phase_State * const Phase_Const[6]={&AB,&AC,&BC,&BA,&CA,&CB};    
Phase_State * const Phase_Const_Reverse[6]={&BA,&CA,&CB,&AB,&AC,&BC};
//{&CB,&CA,&BA,&BC,&AC,&AB};
//{&CA,&CB,&AB,&AC,&BC,&BA};
int Test_Table_Cnt=0;

float Motor_Duty=30;       //电机占空比

int16_t Start_Position=317;
int Phase_Change_Cnt=0;//换向计数，仅用于磁编码器的无刷电机

int Phase_Open_Cnt=0;// 相开启时间计数，防止某一相导通太长时间导致电流过大
                     // 1ms 增加一次，在systick中断中增加
uint8_t Hall_Position=0;

void Set_Motor_Duty(float duty){ //设置电机占空比
  if(duty<0){
    Phase_Table_Using=Phase_Table_Reverse;
  }else{
    Phase_Table_Using=Phase_Table;
  }
  Motor_Duty=fabs(duty);
}

void Phase_Table_Init(){
  
  uint8_t hall_state=0;
  for(int i=0;i<6;++i){
    hall_state=Phase_Test_Table[i];
    Phase_Table[hall_state]=Phase_Const[i];
    Phase_Table_Reverse[hall_state]=Phase_Const_Reverse[i];
  }
  Phase_Table_Using=Phase_Table;
  
  /*
  //逆时针换相表
  Phase_Table[2]=&AB;
  Phase_Table[6]=&AC;
  Phase_Table[4]=&BC;
  Phase_Table[5]=&BA;
  Phase_Table[1]=&CA;
  Phase_Table[3]=&CB;
  */
}

void Phase_Change(Phase_State target,float speed){
  Close_Phases();
  Phase_Open_Cnt=0;
  Set_Phase_Low_State(target.Low,LOW_OPEN);
  Set_Phase_High_Speed(target.High,speed);     //开启目标高低桥  
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
  speed=speed>95?95:speed;
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

void Close_Phases(){
  Set_Phase_Low_State(A,0);
  Set_Phase_Low_State(B,0);
  Set_Phase_Low_State(C,0);
  Set_Phase_High_Speed(A,0);
  Set_Phase_High_Speed(B,0);
  Set_Phase_High_Speed(C,0);
  
  //uprintf("ok,close all phases\r\n");
}

uint16_t Read_Mag(){
  uint8_t data[2];
  uint16_t position;
  HAL_GPIO_WritePin(CSN_GPIO_Port,CSN_Pin,0);
  HAL_SPI_Receive(&hspi2,data,2,10);
  HAL_GPIO_WritePin(CSN_GPIO_Port,CSN_Pin,1);
  position=(uint16_t)(data[0])<<8|data[1];
  position>>=6;
  //uprintf("data = %x %x \r\n",data[0],data[1]);
  //uprintf("position=%d\r\n",position);
  return position;
}

uint8_t Get_Hall_Position(){
  uint8_t temp;
  temp=GPIOA->IDR&(GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
  return temp>>3;
}

void Set_To_Statble_Positon(){  //将转子定位到固定的位置
  int i=0;
  uint8_t position,last_position;
  int position_same_cnt=0;         //本次位置与上次位置相同的情况 计数值
  for(i=0;i<10;++i){
    Phase_Change(AB,TEST_TABLE_SPEED);
    HAL_Delay(10);
    Close_Phases();
    
    position=Get_Hall_Position();
    if(position==last_position){
      position_same_cnt++;
    }
    last_position=position;
    if(position_same_cnt>=3){
      uprintf("already in stable position!i=%d the same cnt=%d\r\n",i,position_same_cnt);
      break;
    }
  }
  
}