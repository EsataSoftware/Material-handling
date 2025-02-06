#include "stm32f10x.h" // Device header
#include "OLED.h"
#include "SERIAL.h"
#include "KEY.h"
#include "String_To_Num.h"
#include "Delay.h"
#include "Encoder.h"
#include "Pid.h"
#include "PWM.h"
#include "Timer4.h"
#include "stdio.h" //支持print
#include "mpu6050.h"
#include "init.h"
#include "all_data.h"
#include "control.h"

unsigned char QrCode = 0;

int main(void)
{
    unsigned char Oled_Show_Flag  = 0;// 显示二维码
    unsigned char Catch_Frequency = 0; // 抓取的次数
    ALL_Init();
    while (1) {
        if (Oled_Show_Flag == 0 && (Serial5_RxPacket[5] == 1 || Serial5_RxPacket[5] == 2 || Serial5_RxPacket[5] == 3)) // 屏幕显示扫到的二维码
        {
            OLED_ShowNum_high(1, 1, Serial5_RxPacket[0], 1);
            OLED_ShowNum_high(1, 2, Serial5_RxPacket[1], 1);
            OLED_ShowNum_high(1, 3, Serial5_RxPacket[2], 1);
            OLED_ShowChar_high(1, 4, '+');
            OLED_ShowNum_high(1, 5, Serial5_RxPacket[3], 1);
            OLED_ShowNum_high(1, 6, Serial5_RxPacket[4], 1);
            OLED_ShowNum_high(1, 7, Serial5_RxPacket[5], 1);
            Oled_Show_Flag = 1;
            QrCode         = 1;
        }
        if (Write_Flag == 0xcc && Catch_Flag == 1) { // 收到树莓派抓取指令,开始抓取
            Catch_Frequency %= 3;
            PWM1_SetCompare2(600);
            Catch_Mode(++Catch_Frequency);
            Write_Flag = 0x01;
            OLED_ShowString_high(2, 1, "Catch");
            if (Catch_Frequency == 3) { // 三个物块抓取完毕
                OLED_ShowString_high(2, 1, "Over");
                Serial_TxPacket[0] = 0XAD;
                Serial_SendPacket();
                Mode_Flag = REVOLVE_MODE_90;
            }
        }
        if (Place_Blue == 1 || Place_Green == 1 || Place_Red == 1) {
            Serial_TxPacket[0] = 0x01;
            Serial_SendPacket();
            Place_Sthing();
            Place_Blue  = 0;
            Place_Green = 0;
            Place_Red   = 0;
        }
        if (Catch_Blue == 1 || Catch_Green == 1 || Catch_Red == 1) {
            Serial_TxPacket[0] = 0x01;
            Serial_SendPacket();
            Catch_Sthing();
            Catch_Blue  = 0;
            Catch_Green = 0;
            Catch_Red   = 0;
        }
        if (ALL_Place < 3) {
            Serial_TxPacket[1] = Serial5_RxPacket[0];
            Serial_TxPacket[2] = Serial5_RxPacket[1];
            Serial_TxPacket[3] = Serial5_RxPacket[2];
        } else {
            Serial_TxPacket[1] = Serial5_RxPacket[3];
            Serial_TxPacket[2] = Serial5_RxPacket[4];
            Serial_TxPacket[3] = Serial5_RxPacket[5];
        }
    }
}

void TIM6_IRQHandler(void)
{
    static unsigned char i, j;
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
        i++;
        j++;
        i %= 5;
        j %= 10000;
        if (i == 0) {
            MpuGetData();
            GetAngle(&MPU6050, &Angle, 0.005f);
        }
        if (i == 1) {
            Get_Speed();
        }
        if (i == 2) {
            Control_Mode();
        }
        if (i == 3) {
            Pid_Control_Trans();
        }
        // if (j == 3) {
        //     Serial_SendPacket();
        // }
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
