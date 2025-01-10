#ifndef __IC_H
#define __IC_H

void IC_Init(void);
uint32_t IC_GetEncode(void);
void TIM_SetCCR(TIM_TypeDef* TIMx, uint16_t Counter);

#endif
