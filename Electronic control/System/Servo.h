#ifndef __SERVO_H
#define __SERVO_H
#include "stm32f10x.h"                  // Device header
#include "Delay.h"


void PWM1_SetCompare2(uint16_t Compare);
void PWM1_SetCompare3(uint16_t Compare);
void PWM1_SetCompare4(uint16_t Compare);
void cloud_tai(int site); // 770 1950
void robotic_grab(int site); //890 760 500 claw_grab抓取 claw_free释放  claw_look初始位置方便openmv观察

void Servo_Init(void);
#endif
