#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>

extern uint8_t Serial_TxPacket[10]; // FF 01 02 03 04 FE
extern uint8_t Serial_RxPacket[10];
extern uint8_t Serial_RxFlag;


void Serial5_SendByte(uint8_t Byte);
void Serial4_SendArray(uint8_t *Array, uint16_t Length);
void Serial5_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void Serial4_Init(void);
void Serial_5_Init(void);
void Serial4_SendPacket(void);

uint8_t Serial_GetRxFlag(void);

#endif
