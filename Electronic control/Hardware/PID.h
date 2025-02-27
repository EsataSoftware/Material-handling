#ifndef __PID_H
#define __PID_H
#include "stm32f10x.h"                  // Device header
#include "ALL_DATA.h"

extern void CascadePID(PidObject* pidRate,PidObject* pidAngE,const float dt);  //串级PID
extern void pidRest(PidObject **pid,const uint8_t len); //pid数据复位
extern void pidUpdate(PidObject* pid,const float dt);  //PID


#endif
	
