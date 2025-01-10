#ifndef __STEP_DIVER_H
#define __STEP_DIVER_H
#include "stm32f10x.h"                  // Device header
#include "Delay.h"


void Micorstep_Driver_Init(void);
void UP(float number_of_turns);
void DOWN(float number_of_turns);
void Micorstep_Enable(void);



#endif
