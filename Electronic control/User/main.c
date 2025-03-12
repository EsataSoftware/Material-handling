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
#include "HMI.h"

unsigned char QrCode = 0;

int main(void)
{
    unsigned char Catch_Frequency = 0; // 抓取的次数
    //Serial4_Init();
    // while (Write_Flag != 0xFA);
    ALL_Init();
    while (1) {
        if (QrCode == 0 && (Serial5_RxPacket[5] == 1 || Serial5_RxPacket[5] == 2 || Serial5_RxPacket[5] == 3)) // 屏幕显示扫到的二维码
        {
            // OLED_ShowNum_high(1, 1, Serial5_RxPacket[0], 1);
            // OLED_ShowNum_high(1, 2, Serial5_RxPacket[1], 1);
            // OLED_ShowNum_high(1, 3, Serial5_RxPacket[2], 1);
            // OLED_ShowChar_high(1, 4, '+');
            // OLED_ShowNum_high(1, 5, Serial5_RxPacket[3], 1);
            // OLED_ShowNum_high(1, 6, Serial5_RxPacket[4], 1);
            // OLED_ShowNum_high(1, 7, Serial5_RxPacket[5], 1);
            Serial_SendHMI("t0", "txt", Serial5_RxPacket[0]);
            Serial_SendHMI("t1", "txt", Serial5_RxPacket[1]);
            Serial_SendHMI("t2", "txt", Serial5_RxPacket[2]);
            Serial_SendHMI("t3", "txt", Serial5_RxPacket[3]);
            Serial_SendHMI("t4", "txt", Serial5_RxPacket[4]);
            Serial_SendHMI("t5", "txt", Serial5_RxPacket[5]);
            QrCode = 1;
        }
        if (Catch_Flag == 1) {
            DOWN(2.00);
            Catch_Flag++;
        }
        if (Write_Flag == 0xcc && Catch_Flag == 2) { // 收到树莓派抓取指令,开始抓取
            Catch_Frequency %= 3;
            robotic_grab(800);
            Write_Flag = 0x01;
            Catch_Mode(++Catch_Frequency);
            if (Catch_Frequency == 3) {    // 三个物块抓取完毕
                Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
                Serial4_SendPacket();
                Mode_Flag = REVOLVE_MODE_90;
            }
        }
        if (Place_Blue == 1 || Place_Green == 1 || Place_Red == 1) {
            Serial_TxPacket[0] = 0x01;
            Serial4_SendPacket();
            Place_Sthing();
            Place_Blue  = 0;
            Place_Green = 0;
            Place_Red   = 0;
        }
        if (Catch_Blue == 1 || Catch_Green == 1 || Catch_Red == 1) {
            Serial_TxPacket[0] = 0x01;
            Serial4_SendPacket();
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
        j %= 1000;
        if (i == 0) {
            MpuGetData();
            GetAngle(&MPU6050, &Angle, 0.005f);
        } else if (i == 1) {
            Get_Speed();
        } else if (i == 2) {
            Control_Mode();
        } else if (i == 3) {
            Pid_Control_Trans();
        }
        if (!QrCode && j == 3) {
            Serial4_SendPacket();
        }
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
