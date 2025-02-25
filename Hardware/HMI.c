#include "all_data.h"
#include "Serial.h"
#include "stm32f10x.h"
#include "HMI.h"
#include<string.h>
void Serial_SendEnd()
{
    for (int i = 0; i < 3; i++)
        Serial5_SendByte(0xFF);
}
void Serial_SendHMI(char *Tn, char *Val, uint16_t Num)
{
    memset(msg, 0, sizeof msg);  
    sprintf(msg, "%s.%s=\"%d\"", Tn, Val, Num);
    Serial5_SendString(msg);
    Serial_SendEnd();
}



