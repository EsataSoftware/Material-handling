#ifndef __ENCODER_H
#define __ENCODER_H
#include "stm32f10x.h"                  // Device header
#include "all_data.h"
#include "Filter.h"
void left_front_Encoder_Init(void);
void right_front_Encoder_Init(void);
void left_back_Encoder_Init(void);
void right_back_Encoder_Init(void);

void Encoder_Init(void);

void Get_Speed(void);


int16_t left_front_Encoder_Get(void);
int16_t right_front_Encoder_Get(void);
int16_t left_back_Encoder_Get(void);
int16_t right_back_Encoder_Get(void);

#endif
