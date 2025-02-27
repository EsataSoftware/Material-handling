#ifndef __SERVO_H
#define __SERVO_H
#include "stm32f10x.h"                  // Device header
#include "Delay.h"


void robotic_grab(uint16_t Compare);
void stage(uint16_t Compare);
void cloud_tai(uint16_t Compare);

void Servo_Init(void);
#endif
