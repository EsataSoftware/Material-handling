#include "step_diver.h"

// #include "data_def.h"

#define ENA_GPIO GPIOB
#define ENA      GPIO_Pin_4
#define DIR_GPIO GPIOB
#define DIR      GPIO_Pin_5
#define PUL_GPIO GPIOA
#define PUL      GPIO_Pin_12

uint8_t PUL_state, turn, Micorstep;
float Tim;
float Number_Of_Turns;

void Micorstep_Driver_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = ENA;
    GPIO_Init(ENA_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = DIR;
    GPIO_Init(DIR_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = PUL;
    GPIO_Init(PUL_GPIO, &GPIO_InitStructure);

    NVIC_EnableIRQ(TIM7_IRQn);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    TIM_TimeBaseInitStruct.TIM_Period    = 200 - 1;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 45 - 1; // ¶¨Ê±1ms
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);

    TIM_ClearFlag(TIM7, TIM_FLAG_Update);
    //    TIM_ITConfig(TIM7, TIM_IT_Update,ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStruct.NVIC_IRQChannel                   = TIM7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_Cmd(TIM7, ENABLE);
}

void Micorstep_Enable(void)
{
    Micorstep = 0;
}

void DOWN(float number_of_turns)
{
    while (Micorstep == 0) {
        Number_Of_Turns = number_of_turns;
        if (turn == 0) {
            GPIO_SetBits(ENA_GPIO, ENA);
            GPIO_ResetBits(DIR_GPIO, DIR);

            TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
            NVIC_EnableIRQ(TIM7_IRQn);
            NVIC_InitTypeDef NVIC_InitStructure;
            NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
            NVIC_Init(&NVIC_InitStructure);

            TIM_Cmd(TIM7, ENABLE);
        } else if (turn == 1) {
            Tim = 0;

            TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
            NVIC_DisableIRQ(TIM7_IRQn);
            NVIC_InitTypeDef NVIC_InitStructure;
            NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
            NVIC_Init(&NVIC_InitStructure);

            TIM_Cmd(TIM7, DISABLE);
            TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
            Micorstep = 1;
            turn      = 0;
        }
    }
}

void UP(float number_of_turns)
{
    while (Micorstep == 0) {
        Number_Of_Turns = number_of_turns;
        if (turn == 0) {
            GPIO_SetBits(ENA_GPIO, ENA);
            GPIO_SetBits(DIR_GPIO, DIR);

            TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
            NVIC_EnableIRQ(TIM7_IRQn);
            NVIC_InitTypeDef NVIC_InitStructure;
            NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
            NVIC_Init(&NVIC_InitStructure);

            TIM_Cmd(TIM7, ENABLE);
        } else if (turn == 1) {
            Tim = 0;

            TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
            NVIC_DisableIRQ(TIM7_IRQn);

            NVIC_InitTypeDef NVIC_InitStructure;
            NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
            NVIC_Init(&NVIC_InitStructure);

            TIM_Cmd(TIM7, DISABLE);
            TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
            Micorstep = 1;
            turn      = 0;
        }
    }
}

void TIM7_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) == SET) {
        if (PUL_state == 1) {
            GPIO_SetBits(PUL_GPIO, PUL);
            PUL_state = 0;
            Tim++;
        } else if (PUL_state == 0) {
            GPIO_ResetBits(PUL_GPIO, PUL);
            PUL_state = 1;
            Tim++;
        }
        if (Tim >= Number_Of_Turns * 1598) {
            Tim  = 0;
            turn = 1;
        }

        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}
