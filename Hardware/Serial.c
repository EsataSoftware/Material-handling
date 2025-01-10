#include "stm32f10x.h" // Device header
#include <stdio.h>
#include <stdarg.h>
#include "OLED.h"
#include "all_data.h"
#include "Filter.h"
#include "control.h"
uint8_t Serial_TxPacket[10] = {0x01, 0X03, 0X01, 0X55}; // FF 01 02 03 04 FE openmv[0]是寻找标志位 1 2 3扫码得到的物料顺序
uint8_t Serial_RxPacket[10];
uint8_t Serial5_RxPacket[10] = {0};
uint8_t Serial_RxFlag;
float LX, LY, Angle_Yaw;
unsigned char Write_Flag;
unsigned char Serial_Yaw;
unsigned char Yellow_Flag;
unsigned char QFlag;
float Last_Yaw;

void Serial4_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_Init(UART4, &USART_InitStructure);

    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(UART4, ENABLE);
}

// void Serial_5_Init(void)
// {
//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

//     GPIO_InitTypeDef GPIO_InitStructure;
//     GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
//     GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOD, &GPIO_InitStructure);

//     GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
//     GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOC, &GPIO_InitStructure);

//     USART_InitTypeDef USART_InitStructure;
//     USART_InitStructure.USART_BaudRate            = 57600;
//     USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//     USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
//     USART_InitStructure.USART_Parity              = USART_Parity_No;
//     USART_InitStructure.USART_StopBits            = USART_StopBits_1;
//     USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
//     USART_Init(UART5, &USART_InitStructure);

//     USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

//     NVIC_InitTypeDef NVIC_InitStructure;
//     NVIC_InitStructure.NVIC_IRQChannel                   = UART5_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
//     NVIC_Init(&NVIC_InitStructure);

//     USART_Cmd(UART5, ENABLE);
// }

void Serial_SendByte(uint8_t Byte)
{
    USART_SendData(UART5, Byte);
    while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
}

void Serial4_SendByte(uint8_t Byte)
{
    USART_SendData(UART4, Byte);
    while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for (i = 0; i < Length; i++) {
        Serial4_SendByte(Array[i]);
    }
}

void Serial_SendString(char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++) {
        Serial_SendByte(String[i]);
    }
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--) {
        Result *= X;
    }
    return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++) {
        Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
    }
}

int fputc(int ch, FILE *f)
{
    Serial_SendByte(ch);
    return ch;
}

void Serial_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial_SendString(String);
}

void Serial_SendPacket(void)
{
    Serial4_SendByte(0xFF);
    Serial_SendArray(Serial_TxPacket, 4);
    Serial4_SendByte(0xFE);
}

uint8_t Serial_GetRxFlag(void)
{
    if (Serial_RxFlag == 1) {
        Serial_RxFlag = 0;
        return 1;
    }
    return 0;
}

void UART4_IRQHandler(void)
{
    static uint8_t RxState   = 0;
    static uint8_t pRxPacket = 0;
    if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET) {
        uint8_t RxData = USART_ReceiveData(UART4);
        if (RxState == 0) {
            if (RxData == 0xFF) {
                RxState   = 1;
                pRxPacket = 0;
            }
        } else if (RxState == 1) {
            if (RxData != 0xFE) {
                if (QrCode == 0) {
                    Serial5_RxPacket[pRxPacket++] = RxData;
                } else
                    Serial_RxPacket[pRxPacket++] = RxData;
            }
            if (RxData == 0xFE) {
                RxState       = 0;
                Serial_RxFlag = 1;
                Serial_Yaw    = Serial_RxPacket[0]; // 偏航角
                // Serial_RxPacket[1]是oled显示的标志位
                //				Serial_Yaw = Kalman_Filter(&KF_Angle,Serial_Yaw);
                Last_Yaw   = Kalman_Filter(&KF_Angle, Serial_Yaw);
                LX         = Serial_RxPacket[2] * 3;
                LY         = Serial_RxPacket[3] * 2;
                Write_Flag = Serial_RxPacket[4]; // 寻白,寻黄和抓取标志位
            }
        }

        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
    }
}

// void UART5_IRQHandler(void)
// {
//     static uint8_t RxState   = 0;
//     static uint8_t pRxPacket = 0;
//     if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET) {
//         uint8_t RxData = USART_ReceiveData(UART5);
//         if (RxState == 0) {
//             if (RxData == 0xFF) {
//                 RxState   = 1;
//                 pRxPacket = 0;
//             }
//         } else if (RxState == 1) {
//             if (RxData != 0xFE) {
//                 Serial5_RxPacket[pRxPacket++] = RxData;
//             }
//             if (RxData == 0xFE) {
//                 RxState = 0;
//                 if (ALL_Place < 3) {
//                     Serial_TxPacket[1] = Serial5_RxPacket[0];
//                     Serial_TxPacket[2] = Serial5_RxPacket[1];
//                     Serial_TxPacket[3] = Serial5_RxPacket[2];
//                 } else {
//                     Serial_TxPacket[1] = Serial5_RxPacket[3];
//                     Serial_TxPacket[2] = Serial5_RxPacket[4];
//                     Serial_TxPacket[3] = Serial5_RxPacket[5];
//                 }
//             }
//         }

//         USART_ClearITPendingBit(UART5, USART_IT_RXNE);
//     }
// }
