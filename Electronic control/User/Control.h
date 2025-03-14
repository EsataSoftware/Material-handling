#ifndef __CONTROL_H
#define __CONTROL_H
#include "stm32f10x.h" // Device header
#include "all_data.h"
#include "Motor.h "
#include "pid.h"
#include "Delay.h"
#include "servo.h"
#include "step_diver.h"
#include "serial.h"

extern unsigned char ALL_Place;

void Forward_Mode(float dt);
void Forward_Mode_Low(float dt);
void Forward_Mode_Mid(float dt);
void Back_Mode(float dt);
void Back_Mode_Low(float dt);
void Back_Mode_Mid(float dt);
void Trans_Mode(float dt); // 横移模式
void Location_Mode(float dt);
void Revolve_Mode(float dt); // 旋转模式
void Stop_Mode(void);
void Control_Mode(void);
void Pid_Control_Trans(void);   // 模式工作函数
void Trans_Left_Mode(float dt); // 横移模式
void Trans_Right_Mode(float dt);
void Location_Mode_Place(float dt);
void Catch_Mode(unsigned char Color);
void Place_Mode(unsigned char Color);
void Place_Sthing(void);
void Place_ALL(unsigned char Place_F);
void Catch_All(unsigned char Catch_F);
void Catch_Sthing(void);
void Revolve_Mode_90(float dt);
void Revolve_Mode_0(float dt);
void Revolve_Mode_180(float dt);

#define STOP_MODE           255
#define REVOLVE_MODE        12
#define FOR_MODE            13
#define TRANS_MODE          14
#define LOCATION_MODE       15
#define TRANS_LEFT_MODE     16
#define BACK_MODE           17
#define LOCATION_PLACE_MODE 18
#define FOR_MODE_LOW        19
#define REVOLVE_MODE_90     20 // 左转
#define REVOLVE_MODE_0      21 // 右转
#define REVOLVE_MODE_180    22 // 翻转
#define TRANS_RIGHT_MODE    23
#define BACK_MODE_LOW       24
#define BACK_MODE_MID       25
#define FOR_MODE_MID        26
void Control_Moto(void);

#endif
