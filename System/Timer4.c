#include "stm32f10x.h" // Device header
#include "mpu6050.h"

void TIM6_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    // 使能TIM6时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    // 设置计时器的时钟分频
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1; // 设置预分频值, 10kHz的计数频率

    // 设置计时器的重载值和计时单位
    TIM_TimeBaseStructure.TIM_Period      = 10 - 1; // 设置自动重装载值, 实现10ms的计时
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    // 配置计时器6
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

    // 使能计时器6的更新中断
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    // 启动计时器6
    TIM_Cmd(TIM6, ENABLE);

    // 启用计时器6的中断
    NVIC_EnableIRQ(TIM6_IRQn);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel                   = TIM6_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 6》使能定时器6
    TIM_Cmd(TIM6, ENABLE);
}

// 定时器6的中断处理函数
