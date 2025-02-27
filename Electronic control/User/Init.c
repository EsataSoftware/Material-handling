#include "init.h"

PidObject PID_FOR_L, PID_FOR_R, PID_BAC_L, PID_BAC_R;
PidObject pidYaw;
PidObject pidRateZ; // 陀螺仪pidh'fe'j'k'l'l'k'jiou'j'sa'l'd'f'k'k'l'j'a'u'l
PidObject pidLX;
PidObject pidLY;

void ALL_Init()
{
    Delay_ms(1200);
    Kalman_Init();
    // OLED_Init();
    // OLED_ShowString_high(2, 1, "Hello");
    Micorstep_Driver_Init(); // 步进电机初始化 最高11.20
    UP(5.00);
    Delay_ms(1000);
    Micorstep_Enable();
    IIC_Init();
    MpuInit();
    // Serial4_Init();
    Serial5_Init();
    Motor_Init();
    Servo_Init();
    Encoder_Init();
    MpuGetOffset();
    pid_Para_Init_Location();
    TIM6_Init();
    robotic_grab(600);
    cloud_tai(1080);
}

void pid_Para_Init_Location()
{
    PID_FOR_L.kp = 30;    // 450 460  450
    PID_FOR_R.kp = 16;    // 430 420  430
    PID_BAC_L.kp = 16;    // 430  420   430
    PID_BAC_R.kp = 30;    // 450    455  455
    pidYaw.kp    = 0.87;  // 0.85    1.35  0.95 1.15  0.85
    pidRateZ.kp  = 0.67;  // 0.454   0.460   0.465   0.4658   0.335  0.400  0.410 0.80  0.60
    pidLX.kp     = 0.015; // 0.065   0.002  0.013  0.023  0.01
    pidLY.kp     = 0.013; // 0.055    0.002  0.013  0.023  0.006

    PID_FOR_L.ki = 3;   // 360
    PID_FOR_R.ki = 3;   // 360
    PID_BAC_L.ki = 3;   // 360
    PID_BAC_R.ki = 3;   // 360
    pidYaw.ki    = 0.0; // 0.015 0.39
    pidRateZ.ki  = 0.0; // 0.25 0.08 0.21
    pidLX.ki     = 0.00;
    pidLY.ki     = 0.00;

    PID_FOR_L.kd = 0.05;   // 0.01
    PID_FOR_R.kd = 0.05;   // 0.001
    PID_BAC_L.kd = 0.05;   // 0.001
    PID_BAC_R.kd = 0.05;   // 0.01
    pidYaw.kd    = 0.013;  // 0.03  0.34
    pidRateZ.kd  = 0.0022; // 0.00108    0.00123    0.00125    0.00138   0.00148
    pidLX.kd     = 0.001;
    pidLY.kd     = 0.002;
}
