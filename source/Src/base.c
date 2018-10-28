#include "base.h"
#include "command.h"
#include "string.h"
#include "usart.h"
#include "stdlib.h"

char Send_Wave_Flag=0;    // 控制是否发送波形的标志

float * debug_wave[4];  //指针数组，指向要发送的四个波形
float wave_gain=1; //波形增益，因为要把float转为int，可能需要适当放大。

char Wave_Set[4]; //波形设置,写入flash中

uint8_t First_Time_Check=0x55;//这个参数写在flash中，如果读出来的不是0x55，则证明是第一次

void set_first_time_check_flag(int arg_num,char **s,float * args){
  if(arg_num!=0x0001){
    uprintf("error arg_num!\r\n");
    return ;
  }
  First_Time_Check=(uint8_t)args[0];
  uprintf("ok,set first time check flag=%c,remember to write!\r\n",First_Time_Check);
}
typedef struct{
  char * pram_string;
  void * pram_data;
  size_t size;
}pram_node;

#define FLASH_Start 0x08020000     
pram_node Pram_Array[]={
  {"First_Time_Check",&First_Time_Check,sizeof(First_Time_Check)},

};

wave_node Wave_Array[]={
  0
};


/*
波形发送函数，该函数可以通过设置debug_wave这个指针数组，来控制要发送的变量。
*/
void set_debug_wave(int arg_num,char ** string_prams,float * float_prams){
  int index;
  char * string;
  int i;
  if(arg_num!=0x0101&&arg_num!=0x0100){
    uprintf("error arg_num!\r\n");
    return ;
  }
  index=(int)float_prams[0];
  string=string_prams[0];
  if(index<0||index>3){
    uprintf("error index!It must be 0-3!\r\n");
    return ;
  }
  
  int length=sizeof(Wave_Array)/sizeof(wave_node);
  for(i=0;i<length;++i){
    if(compare_string(Wave_Array[i].wave_string,string)){
      if(arg_num!=0x0100){    //仅提供通道，不提供波形设置，则不修改，只显示当前通道的波形
        debug_wave[index]=Wave_Array[i].wave_ptr;   //修改当前wave输出的指针
        Wave_Set[index]=i;                          //修改保存到prams的值
      }
      uprintf("OK,OutputData[%d] = %s!\r\n",index,Wave_Array[i].wave_string);
      return ;
    }
  }
}

void set_wave_gain(int arg_num,char **s,float * args){
  if(arg_num>0x0001){
    uprintf("error arg_nums!There must be 1 args!\r\n");
    return ;
  }
  if(args[0]<0){
    uprintf("error gain!It must bigger than 0!\r\n");
    return ;
  }
  if(arg_num){
    wave_gain=args[0];
  }
  uprintf("OK,wave_gain=%f\r\n",wave_gain);
}



float avarge(int *data,int count){
  int i=0;
  int sum=0;
  for(i=0;i<count;++i){
    sum+=data[i];
  }
  return (float)sum/(float)count;
}

/*
卡尔曼滤波函数
*/

float KalMan(Kal_Struct *kal,float x){
  
  float kalman_pre;  //卡尔曼的预测值
  float cov_pre;  //卡尔曼预测值的协方差
  
  
  float kg;//增益
  kalman_pre=kal->kal_out*kal->A;  //计算本次卡尔曼的预测值
  
  cov_pre=kal->cov*kal->A*kal->A+kal->Q;
  
  kg=cov_pre/(cov_pre+kal->R);   //计算本次的卡尔曼增益
  
  kal->kal_out=kalman_pre+kg*(x-kalman_pre);   //通过预测值来计算本次卡尔曼滤波后的输出
  
  kal->cov=(1-kg)*cov_pre;
  
  return kal->kal_out;
}



float Window_Filter(Window_Filter_Struct * wfs,float data){
  int j;
  float sum=0;
  
  if(!wfs->Window_Buffer)
    return 0;
  wfs->Window_Buffer[wfs->i]=data;
  wfs->i++;
  if(wfs->i==wfs->max){
    wfs->i=0; 
  }
  for(j=0;j<wfs->max;++j){
    sum+=wfs->Window_Buffer[j];
  }
  return sum/wfs->max;
}


/*
控制是否发送波形的命令
*/
void set_send_wave_flag(int arg_num,char **s,float * arg){
  if(arg_num!=1&&arg_num!=0){
    uprintf("error arg num!\r\n");
    return ;
  }
  if(arg_num){
    Send_Wave_Flag=(int)arg[0];
  }else{
    Send_Wave_Flag=!Send_Wave_Flag;
  }
  uprintf("ok.Send_Wave_Flag=%d\r\n",Send_Wave_Flag);
}



void write_prams(int arg_num,char ** s,float * args){
  uint32_t SectorError;
  uint32_t temp;
  int i;
  FLASH_EraseInitTypeDef EraseInitStruct;
  
  if(arg_num!=0x00){
    uprintf("error arg_num!\r\n");
    return ;
  }
  
  HAL_FLASH_Unlock();
  
  
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_Start;
  EraseInitStruct.NbPages = 1;
  
  if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
  { 
    uprintf("erase flash fail!\r\n");
    HAL_FLASH_Lock();
    return ;
  }
  int pram_num=sizeof(Pram_Array)/sizeof(pram_node);
  for(i=0;i<pram_num;++i){
    temp=*((uint32_t *)(Pram_Array[i].pram_data));
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,FLASH_Start+i*4,temp);
    uprintf("write Pram[%d]  %s Ok!\r\n",i,Pram_Array[i].pram_string);
  }
  HAL_FLASH_Lock();
  uprintf("Write OK!\r\n");
}

void load_prams(int arg_num,char ** s,float * args){
  int i;
  if(arg_num!=0x0000){
    uprintf("error arg_num!\r\n");
    return ;
  }
  int pram_num=sizeof(Pram_Array)/sizeof(pram_node);
  memcpy(Pram_Array[0].pram_data,(void *)(FLASH_Start+0*4),Pram_Array[0].size);//先读first_time_check
  if(First_Time_Check!=0x55){
    uprintf("This is the first time to step up!\r\n");
    First_Time_Check=0x55;
    write_prams(0,0,0);
  }
  
  for(i=0;i<pram_num;++i){
    memcpy(Pram_Array[i].pram_data,(void *)(FLASH_Start+i*4),Pram_Array[i].size);
    uprintf("Load Pram %dth %s:f:%f  d:%d\r\n",i,Pram_Array[i].pram_string,\
      *(float *)(Pram_Array[i].pram_data),\
        *(int *)(Pram_Array[i].pram_data));
  }	
}

void get_info(int arg_num,char **s,float *args){
  if(arg_num>0){
    uprintf("error arg_num!\r\n");
    return ;
  }
  int num=sizeof(Wave_Array)/sizeof(wave_node);
  for(int i=0;i<num;++i){
    uprintf("%s:  %f\r\n",Wave_Array[i].wave_string,*Wave_Array[i].wave_ptr);
  }
}

void HB_Push(History_Buffer * hb,float data){
  if(hb->i==hb->max){    //已至末尾
    hb->i=0;              //移到数组头
    if(!hb->is_full){
      hb->is_full=1;
    }
  }
  hb->buffer[hb->i]=data;
  hb->i++;
}

void HB_Clear(History_Buffer * hb){
  hb->i=0;
}

int HB_Now(History_Buffer *hb){
  return hb->i;
}

float HB_Get(History_Buffer * hb,int index){
  if(index<0){
    if(hb->is_full){
      index+=hb->max;
    }else{
      index=0;
    }
  }
  return hb->buffer[index];
}
/*************************************************

函数名: LPButterworth(float curr_input,Butter_BufferData *Buffer,Butter_Parameter *Parameter)

说明: 二阶巴特沃斯数字低通滤波器

入口: float curr_input 当前输入

出口: 滤波器输出值

备注: 2阶Butterworth低通滤波器

*************************************************/


float LPButterworth(float curr_input,Butter_BufferData *Buffer,Butter_Parameter *Parameter)
{
  
  static int LPB_Cnt=0;
  
  /* 加速度计Butterworth滤波 */
  
  /* 获取最新x(n) */
  
  Buffer->Input_Butter[2]=curr_input;
  
  if(LPB_Cnt>=500)
    
  {
    
    /* Butterworth滤波 */
    
    Buffer->Output_Butter[2]=
      
      Parameter->b[0] * Buffer->Input_Butter[2]
        
        +Parameter->b[1] * Buffer->Input_Butter[1]
          
          +Parameter->b[2] * Buffer->Input_Butter[0]
            
            -Parameter->a[1] * Buffer->Output_Butter[1]
              
              -Parameter->a[2] * Buffer->Output_Butter[0];
    
  }
  
  else
    
  {
    
    Buffer->Output_Butter[2]=Buffer->Input_Butter[2];
    
    LPB_Cnt++;
    
  }
  
  /* x(n) 序列保存 */
  
  Buffer->Input_Butter[0]=Buffer->Input_Butter[1];
  
  Buffer->Input_Butter[1]=Buffer->Input_Butter[2];
  
  /* y(n) 序列保存 */
  
  Buffer->Output_Butter[0]=Buffer->Output_Butter[1];
  
  Buffer->Output_Butter[1]=Buffer->Output_Butter[2];
  
  return (Buffer->Output_Butter[2]);
  
}

float Limit_Dealt_Filter(float now,Window_Filter_Struct * wfs,float max_dealt){
  float sub=now-wfs->Window_Buffer[wfs->i];
  float temp;
  if(sub>max_dealt){
    temp=wfs->Window_Buffer[wfs->i]+max_dealt;
  }else if(sub<-max_dealt){
    temp=wfs->Window_Buffer[wfs->i]-max_dealt;
  }else{
    temp=now;
  }
  return Window_Filter(wfs,temp);
}