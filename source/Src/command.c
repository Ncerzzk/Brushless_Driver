#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "usart.h"
#include "ctype.h"
#include "cmd_fun.h"
#include "base.h"


struct {
  char * array[CMD_MAX_NUM];
  int array_ptr;
  cmdFunc func_array[CMD_MAX_NUM];
}CMD;

static void test(int arg_num,char **string_prams,float * arg);

/*
将要增加的命令与函数写在这里
*/
void command_init(void){
  add_cmd("test",test);  
  add_cmd("set_val",set_val);
  add_cmd("get_hall",get_hall_state);
  
  add_cmd("set_fd",set_fd6288);

  add_cmd("write",write_prams);
  add_cmd("load",load_prams);
}



static void test(int arg_num,char **string_prams,float * arg){
  int i;
  uprintf("this is testkk\r\n");
  uprintf("i get %d args\r\n",arg_num);
  for(i=0;i<(arg_num&0xFF);++i){
    uprintf("one is %f\r\n",arg[i]);
  }
  for(i=0;i<(arg_num>>8);++i){
    uprintf("string_prams is %s\r\n",string_prams[i]);
  }
}

#define STRING_PRAM_NUM 4
#define FLOAT_PRAM_NUM  4

static int16_t get_prams(char *s,char *** string_prams,float ** float_prams){
  int i=0;
  enum {IN_DIGITAL=1,IN_LETTER=2,IN_SPACE=0}in_flag;
  char **float_buffer;
  char **string_buffer;
  
  int num_float_i=0,num_string_i=0; //第几个数字
  int num_i_i=0;//第几个数字/字符串的第几位
  
  in_flag=IN_SPACE;
  
  /*
  数字参数的存放位置，这些空间需要在函数外释放
  */
  *float_prams=(float *)malloc(sizeof(int)*FLOAT_PRAM_NUM);
  
  /*
  数字参数的文本临时存放，用于调用atof函数
  在本程序结束前会将这些空间释放掉
  */
  float_buffer=(char **)malloc(sizeof(char *)*FLOAT_PRAM_NUM);
  for(i=0;i<FLOAT_PRAM_NUM;++i){
    float_buffer[i]=(char *)malloc(sizeof(char)*20);
  }
  
  /*
  字符串参数的存放位置，这些空间需要在函数外释放
  */
  string_buffer=(char **)malloc(sizeof(char*) * STRING_PRAM_NUM);
  for(i=0;i<STRING_PRAM_NUM;++i){
    string_buffer[i]=(char *)malloc(sizeof(char)*20);
  }
  
  *string_prams=string_buffer;
  
  i=0;
  //使用DMA，增加s[1]!='\r'的条件，因为没有再把\r换成\0了
  while(s[i]!='\0'&&s[i]!='\r'&&num_float_i<=FLOAT_PRAM_NUM&&num_string_i<=STRING_PRAM_NUM&&num_i_i<20){
    if(s[i]!=' '){//到达非空格
      if(in_flag==IN_SPACE){  //在数字中标志位和字符串标志位未设置,即从空格之后出现的第一个数字/字符
        if(isalpha(s[i])){
          in_flag=IN_LETTER;
          num_string_i++;
          string_buffer[num_string_i-1][0]=s[i];  //复制第一个字符
        }else{
          in_flag=IN_DIGITAL;
          num_float_i++;
          float_buffer[num_float_i-1][0]=s[i];   //复制第一个数字字符
        }
        num_i_i=0;
      }else{
        //继续复制
        if(in_flag==IN_LETTER){
          string_buffer[num_string_i-1][num_i_i]=s[i];
        }else{
          float_buffer[num_float_i-1][num_i_i]=s[i];
        }
      }
      num_i_i++;
    }else{ //是空格
      if(in_flag==IN_LETTER){
        string_buffer[num_string_i-1][num_i_i]='\0';
      }else if(in_flag==IN_DIGITAL){
        float_buffer[num_float_i-1][num_i_i]='\0';
      }
      in_flag=IN_SPACE;
    }
    i++;
  }
  if(in_flag==IN_LETTER){  //如果是字符串参数放在最后，这里必须再检查一下，加上\0
    string_buffer[num_string_i-1][num_i_i]='\0';
  }else if(in_flag==IN_DIGITAL){
    float_buffer[num_float_i-1][num_i_i]='\0';
  }
  
  /*
  释放数字参数的字符临时存放空间
  */
  for(i=0;i<FLOAT_PRAM_NUM;++i){
    (*float_prams)[i]=atof(float_buffer[i]);
    free(float_buffer[i]);
  }
  free(float_buffer);
  
  return num_string_i<<8|num_float_i;  
}


/*
void add_cmd(const char * s,cmdFunc f)
新增命令，s为命令字符串，f为绑定的函数。
*/


void add_cmd(char * s,cmdFunc f){
  if(CMD.array_ptr>CMD_MAX_NUM-1)
    return ;
  CMD.array[CMD.array_ptr]=s;
  CMD.func_array[CMD.array_ptr]=f;
  CMD.array_ptr++;
}

/*
compare_cmd(const char* cmd,char *s)
比较cmd与s两个字符串，其中cmd为命令，s为待比较的字符串
若字符串为该命令，则返回命令的长度，否则返回0
*/
char compare_cmd(const char * cmd,char * s){
  int i=0;
  while(cmd[i]!='\0'&&s[i]!='\0'){
    if(cmd[i]!=s[i])
      return 0;
    ++i;
  }
  if(s[i]!=' '&&s[i]!='\0'&&s[i]!='\r'){          //使用DMA必须判断最后一个i套件
    return 0;   //比如命令hello 与命令 helloworld的情况，没有这个判断，会吧后者误认为前者
  }
  return i;
}

uint8_t compare_string(const char *s1,char * s2){
  int i=0;
  while(s1[i]==s2[i]&&s1[i]&&s2[i]){
    i++;
  }
  if(!s1[i]&&!s2[i]){
    return 1;
  }else{
    return 0;
  }  
}

void analize(uint8_t *s){
  int i=0;
  int j=0,temp=0;
  int prams_num;
  char ** string_prams;
  float * float_prams;
  for(i=0;i<CMD.array_ptr;++i){
    j=compare_cmd(CMD.array[i],(char*)s);
    if(j){
      prams_num=get_prams((char*)s+j,&string_prams,&float_prams);
      CMD.func_array[i](prams_num,string_prams,float_prams);
      free(float_prams);
      for(temp=0;temp<STRING_PRAM_NUM;++temp){
        free(string_prams[temp]);
      }
      free(string_prams);
      HAL_UART_Receive_IT(&HUART_USE,&buffer_rx_temp,1);
      return ;
    }
  }
  HAL_UART_Receive_IT(&HUART_USE,&buffer_rx_temp,1);
  uprintf("error command!\r\n");
}


