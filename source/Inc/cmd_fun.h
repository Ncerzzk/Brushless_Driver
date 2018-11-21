#ifndef __CMD_FUN_H
#define __CMD_FUN_H


void set_val(int arg_num,char ** s,float * args);
void get_hall_state(int arg_num,char ** s,float * args);
void set_fd6288(int arg_num,char **s,float *args);
void set_phase(int arg_num,char **s,float *args);
void phase_change(int arg_num,char **s,float *args);
void read_mag(int arg_num,char **s,float *args);
void set_mode(int arg_num,char **s,float * args);
void set_motor_duty(int arg_num,char **s,float * args);
void set_mode_s(int arg_num,char **s,float *args);
#endif