#ifndef __MY_PID_H
#define __MY_PID_H


typedef struct
{  
    float desire;//期望值
	float measured_value;//测量值
	float integral;//积分
	float derivative;//微分 
  float kp;           //< proportional gain
	float ki;           //< integral gain
	float kd;           //< derivative gain
	float IntegLimitHigh;       //< integral limit
	float IntegLimitLow;
	float OutLimitHigh;//输出限幅
	float OutLimitLow;
	float error0;
	float error1;
	float out;
}PID_Structure;


void PID_ParamInit(void);
void PID_Date_Init(PID_Structure* pid);
void PID_Limt_Init(void);
void pid_update(void);
void PID_Control(PID_Structure* pid);









#endif
