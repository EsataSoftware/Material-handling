#include "init.h"

PidObject PID_FOR_L, PID_FOR_R, PID_BAC_L, PID_BAC_R;
PidObject pidYaw;
PidObject pidRateZ; // 陀螺仪pidh'fe'j'k'l'l'k'jiou'j'sa'l'd'f'k'k'l'j'a'u'l
PidObject pidLX;
PidObject pidLY;

PidObject pidLX1;
PidObject pidLY1;

void ALL_Init()
{
    Delay_ms(1200);
    Kalman_Init();
    Micorstep_Driver_Init(); // 步进电机初始化 最高11.30 17.3
    UP(6.30);
    Delay_ms(1000);
    Micorstep_Enable();
    IIC_Init();
    MpuInit();
    Serial4_Init();
    Serial5_Init();
    Motor_Init();
    Servo_Init();
    Encoder_Init();
    MpuGetOffset();
    pid_Para_Init_Location();
    TIM6_Init();
    robotic_grab(1200);
    cloud_tai(1080);
}

void pid_Para_Init_Location()
{
    PID_FOR_L.kp = 30;    
    PID_FOR_R.kp = 16;    
    PID_BAC_L.kp = 16;    
    PID_BAC_R.kp = 30;    
    pidYaw.kp    = 0.85;  
    pidRateZ.kp  = 0.65;  
    pidLX.kp     = 0.015; //0.015
    pidLY.kp     = 0.013; //0.013

    pidLX1.kp     = 0.029; //0.029 0.030
    pidLY1.kp     = 0.038; //0.038


    PID_FOR_L.ki = 3;   
    PID_FOR_R.ki = 3;   
    PID_BAC_L.ki = 3;   
    PID_BAC_R.ki = 3;   
    pidYaw.ki    = 0.0; 
    pidRateZ.ki  = 0.0; 
    pidLX.ki     = 0.00;
    pidLY.ki     = 0.00;

    pidLX1.ki     = 0.003;//0.003
    pidLY1.ki     = 0.002;//0.001

    PID_FOR_L.kd = 0.05;   
    PID_FOR_R.kd = 0.05;  
    PID_BAC_L.kd = 0.05;   
    PID_BAC_R.kd = 0.05;   
    pidYaw.kd    = 0.013;  
    pidRateZ.kd  = 0.0022; 
    pidLX.kd     = 0.001;
    pidLY.kd     = 0.002;

    pidLX1.kd     = 0.0015;//0.0014
    pidLY1.kd     = 0.0018;//0.0018
}
