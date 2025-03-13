#include "control.h"
#include "Servo.h"
#include "OLED.h"
#include "ALL_DATA.h"
unsigned char Mode_Flag       = TRANS_LEFT_MODE;
unsigned char Modes_Flag      = 0;
unsigned int Yellow_Flag      = 0;
unsigned int Trans_Flag       = 0;
unsigned char Start_Flag      = 0;
unsigned char Location_Flag   = 0;
unsigned char Stop_Flag       = 0;
unsigned int Adjust_Timer     = 0;
unsigned char Catch_Flag      = 0; // 抓取标志位//此处调整边线
unsigned char Red_Cirtle_Flag = 0; // 寻找红环标志位
unsigned char Place_Red_Flag  = 1; // 红色放置标志位
unsigned char Place_Blue_Flag = 1; // 蓝色放置标志位
unsigned char Place_Red       = 0;
unsigned char Place_Blue      = 0;
unsigned char Place_Green     = 0;
unsigned char Revolve_Time    = 0; // 转向适应时间
unsigned char u               = 2; // 速度补偿

unsigned char Red_Place_Over   = 0;
unsigned char Blue_Place_Over  = 0;
unsigned char Green_Place_Over = 0;

unsigned char Find1  = 1, Find2, Find3;
unsigned char Catch1 = 1, Catch2, Catch3;

unsigned char Place_Over_Red   = 0;
unsigned char Place_Over_Blue  = 0;
unsigned char Place_Over_Green = 0;

unsigned char Catch_Over_Red   = 0;
unsigned char Catch_Over_Blue  = 0;
unsigned char Catch_Over_Green = 0;

unsigned char Catch_Red   = 0;
unsigned char Catch_Green = 0;
unsigned char Catch_Blue  = 0;

unsigned char P_1, P_2, P_3; // 放置标志位
unsigned char C_1, C_2, C_3; // 抓取标志位

extern unsigned char ALL_Place = 0;

unsigned char Catch_11 = 0;
unsigned char Catch_12 = 0;
unsigned char Catch_13 = 0;

unsigned char Find2_1  = 1, Find2_2, Find2_3;
unsigned char Catch2_1 = 1, Catch2_2, Catch2_3;

#define Cloud_Location  1080 // 云台观察位置
#define Cloud_Palace1   2330 // 云台放置载物台1上位置
#define Cloud_Palace2   2280 // 云台放置载物台2上位置
#define Cloud_Palace3   2350 // 云台放置载物台3上位置
#define stage_state1    1950 // 载物台物块1放置
#define stage_state2    1070 // 载物台物块2放置
#define stage_state3    630  // 载物台物块3放置
#define claw_grab       550  // 机械爪抓取
#define claw_free       1200 // 机械爪释放
#define claw_free_stage 1200 // 机械爪载物台放置
PidObject *(pPidObject[]) = {&pidYaw, &pidRateZ, &PID_FOR_L, &PID_FOR_R, &PID_BAC_L, &PID_BAC_R, &pidLX, &pidLY};

// 0.85与地面交互 9.30抓起车载物料盘物料 1.9放下物料到车载物料盘 5.0抓起原料区物料

void Pid_Control_Trans() // 模式工作函数
{
    if (Mode_Flag == REVOLVE_MODE) {
        Revolve_Mode(0.005f);
        Control_Moto();
    } else if (Mode_Flag == STOP_MODE) {
        Stop_Mode();
    } else if (Mode_Flag == FOR_MODE) {
        Forward_Mode(0.005f);
        Control_Moto();
    } else if (Mode_Flag == TRANS_MODE) {
        Trans_Mode(0.005f);
        Control_Moto();
    } else if (Mode_Flag == LOCATION_MODE) {
        Location_Mode(0.005f);
        Control_Moto();
    } else if (Mode_Flag == TRANS_LEFT_MODE) { // 左平移
        Trans_Left_Mode(0.005f);
        Control_Moto();
    } else if (Mode_Flag == TRANS_RIGHT_MODE) { // 右平移
        Trans_Right_Mode(0.005f);
        Control_Moto();
    } else if (Mode_Flag == BACK_MODE) {
        Back_Mode(0.005f);
        Control_Moto();
    } else if (Mode_Flag == LOCATION_PLACE_MODE) {
        Location_Mode_Place(0.005f);
        Control_Moto();
    } else if (Mode_Flag == FOR_MODE_LOW) {
        Forward_Mode_Low(0.005f);
        Control_Moto();
    } else if (Mode_Flag == BACK_MODE_LOW) { // 翻转
        Back_Mode_Low(0.005f);
        Control_Moto();
    } else if (Mode_Flag == REVOLVE_MODE_90) { // 左转
        Revolve_Mode_90(0.005f);
        Control_Moto();
    } else if (Mode_Flag == REVOLVE_MODE_0) { // 右转
        Revolve_Mode_0(0.005f);
        Control_Moto();
    } else if (Mode_Flag == REVOLVE_MODE_180) { // 翻转
        Revolve_Mode_180(0.005f);
        Control_Moto();
    }
}

void Control_Mode()
{

    if (ALL_Place == 0 || ALL_Place == 3) {
        Trans_Flag++;
        if (Trans_Flag >= 230 && Start_Flag == 0) // 代码开始左平移出库，之后到达计时时间直走
        {
            Mode_Flag  = FOR_MODE;
            Start_Flag = 1; // 起始标志位置1无法再次进入起始条件
        }
        if (Trans_Flag >= 1400 && Stop_Flag == 0) {
            Mode_Flag          = STOP_MODE;
            Serial_TxPacket[0] = 0XFC; // 树莓派定位转盘
            Serial4_SendPacket();
            Serial_TxPacket[0] = 0XAD;
            Stop_Flag          = 1;
        } else if (Trans_Flag >= 1300 && Stop_Flag == 0) {
            Mode_Flag = TRANS_RIGHT_MODE;
        }
        if (LX != 0 && LY != 0 && Location_Flag == 0) {
            Mode_Flag     = LOCATION_MODE; // 进入定位状态
            Location_Flag = 1;
        }
        if (Write_Flag == 0XCD && Mode_Flag == LOCATION_MODE) { // 进入抓取状态
            Mode_Flag  = STOP_MODE;
            Catch_Flag = 1; // 抓取标志位置
            Modes_Flag++;
        }
        if (Angle.yaw >= Angle_Yaw + 75 && Mode_Flag == REVOLVE_MODE_90 && Modes_Flag == 1) {
            pidRest(pPidObject, 6);       // 数据复位
            Mode_Flag = TRANS_RIGHT_MODE; // 切换模式到平移模式
            Angle_Yaw = Angle.yaw;
            Modes_Flag++;
            Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
            Serial4_SendPacket();
        }
        if (Write_Flag == 0XAD && Mode_Flag == TRANS_RIGHT_MODE && Modes_Flag == 2) {
            Mode_Flag          = REVOLVE_MODE_90;
            Write_Flag         = 0x01;
            Serial_TxPacket[0] = 0X01; // 防止树莓派重复进入状态
            Serial4_SendPacket();
            Catch_Flag = 0;
            Modes_Flag++;
        } else if (Angle.yaw >= Angle_Yaw + 75 && Mode_Flag == REVOLVE_MODE_90 && Modes_Flag == 3) {
            pidRest(pPidObject, 6);       // 数据复位
            Mode_Flag = TRANS_RIGHT_MODE; // 切换模式到平移模式
            Angle_Yaw = Angle.yaw;
            Modes_Flag++;
        } else if (Mode_Flag == TRANS_RIGHT_MODE && Modes_Flag == 4) {
            Yellow_Flag++;
            if (Yellow_Flag == 1200) {
                Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
                Serial4_SendPacket();
            }
            if (Yellow_Flag >= 1200) {
                if (Write_Flag == 0XAD) {
                    pidRest(pPidObject, 6);
                    Mode_Flag    = FOR_MODE;
                    Modes_Flag   = 0;
                    Adjust_Timer = 0;
                    Trans_Flag   = 0;
                    ALL_Place++;
                    Yellow_Flag = 0;
                }
            }
        }
    } else if (ALL_Place == 1 || ALL_Place == 4) {
        Trans_Flag++;
        if (Trans_Flag >= 650 && Mode_Flag == FOR_MODE && Modes_Flag == 0) {
            Mode_Flag = STOP_MODE;
            Modes_Flag++;
        }
        if (Find1 == 1) {
            Place_ALL(Serial_TxPacket[1]);
        } else if (Find2 == 1) {
            Place_ALL(Serial_TxPacket[2]);
        } else if (Find3 == 1) {
            Place_ALL(Serial_TxPacket[3]);
        }
        if (Catch1 == 1 && Find1 == 0 && Find2 == 0 && Find3 == 0) { // 放置到粗加工后再夹起来
            Catch_All(Serial_TxPacket[1]);
        }
        if (Catch2 == 1) {
            Catch_All(Serial_TxPacket[2]);
        }
        if (Catch3 == 1) {
            Catch_All(Serial_TxPacket[3]);
        }
        if (Catch1 == 0 && Catch2 == 0 && Catch3 == 0 && Modes_Flag == 1) {
            Mode_Flag = REVOLVE_MODE_0;
            Modes_Flag++;
        }
        if (Angle.yaw >= Angle_Yaw - 75 && Mode_Flag == REVOLVE_MODE_0 && Modes_Flag == 2) // 此处Mode_Flag由main函数设置
        {
            pidRest(pPidObject, 6);                // 数据复位
            Mode_Flag          = TRANS_RIGHT_MODE; // 切换模式到平移模式
            Angle_Yaw          = Angle_Yaw;
            Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
            Serial4_SendPacket();
            Modes_Flag++;
        }
        if (Write_Flag == 0XAD && (Mode_Flag == TRANS_RIGHT_MODE || Mode_Flag == STOP_MODE) && Modes_Flag == 3) {
            Mode_Flag = BACK_MODE;
            Adjust_Timer++;
            if (Adjust_Timer >= 400) {
                Serial_TxPacket[0] = 0XFB; // 树莓派定位色环
                Serial4_SendPacket();
                Mode_Flag = LOCATION_PLACE_MODE;
                ALL_Place++;
                Find1            = 1;
                Catch1           = 1; // 下次要用的数据进行恢复
                P_1              = 0;
                P_2              = 0;
                P_3              = 0;
                Place_Over_Red   = 0;
                Place_Over_Blue  = 0;
                Place_Over_Green = 0;
                Adjust_Timer     = 0;
                Modes_Flag       = 0;
            }
        }
    } else if (ALL_Place == 2 || ALL_Place == 5) {
        if (Find1 == 1) {
            Place_ALL(Serial_TxPacket[1]);
        }
        if (Find2 == 1) {
            Place_ALL(Serial_TxPacket[2]);
        }
        if (Find3 == 1) {
            Place_ALL(Serial_TxPacket[3]);
        }
        if (Find1 == 0 && Find2 == 0 && Find3 == 0 && Modes_Flag == 0) {
            Mode_Flag = REVOLVE_MODE_0;
            Modes_Flag++;
        }
        if (Angle.yaw <= Angle_Yaw - 75 && Mode_Flag == REVOLVE_MODE_0 && Modes_Flag == 1) // 此处Mode_Flag由main函数设置
        {
            pidRest(pPidObject, 6);       // 数据复位
            Mode_Flag = TRANS_RIGHT_MODE; // 切换模式到平移模式
            Angle_Yaw = Angle_Yaw;
            Modes_Flag++;
        }

        if (Write_Flag == 0XAD && (Mode_Flag == TRANS_RIGHT_MODE || Mode_Flag == STOP_MODE) && Modes_Flag == 2) {
            Mode_Flag    = BACK_MODE;
            Adjust_Timer = 0;
            Modes_Flag++;
        }
        if ((Mode_Flag == BACK_MODE || Mode_Flag == STOP_MODE) && Modes_Flag == 3) {
            Trans_Flag++;
            if (Trans_Flag >= 500) {
                if (ALL_Place == 2) {
                    Serial_TxPacket[0] = 0XFC; // 树莓派定位转盘
                    Serial4_SendPacket();
                    ALL_Place++;
                } else if (ALL_Place == 5) {
                    Write_Flag = 0XEA; // 结束寻蓝
                }
                Find1            = 1;
                Catch1           = 1;
                P_1              = 0;
                P_2              = 0;
                P_3              = 0;
                C_1              = 0;
                C_2              = 0;
                C_3              = 0;
                Adjust_Timer     = 0;
                Place_Over_Red   = 0;
                Place_Over_Blue  = 0;
                Place_Over_Green = 0;
                Catch_Over_Red   = 0;
                Catch_Over_Blue  = 0;
                Catch_Over_Green = 0;
                Red_Place_Over   = 0;
                Blue_Place_Over  = 0;
                Green_Place_Over = 0;
            }
        } else if (Write_Flag == 0xEA && (Mode_Flag == BACK_MODE || Mode_Flag == STOP_MODE)&& Modes_Flag == 3) {
            Trans_Flag++;
            if (Trans_Flag >= 1500) {
                Mode_Flag = TRANS_RIGHT_MODE;
                Adjust_Timer++;
                if (Adjust_Timer >= 230) {
                    Mode_Flag = STOP_MODE;
                }
            }
        }
    }
}

void Revolve_Mode_0(float dt)
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw - 75;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //*****************************************************************************************
    // 编码器环pid
    // 目标值更改
    PID_FOR_L.desired = -pidYaw.out;
    PID_FOR_R.desired = +pidYaw.out;
    PID_BAC_L.desired = -pidYaw.out;
    PID_BAC_R.desired = +pidYaw.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Revolve_Mode_90(float dt)
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw + 75;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //*****************************************************************************************
    // 编码器环pid
    // 目标值更改
    PID_FOR_L.desired = -pidYaw.out;
    PID_FOR_R.desired = +pidYaw.out;
    PID_BAC_L.desired = -pidYaw.out;
    PID_BAC_R.desired = +pidYaw.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Revolve_Mode_180(float dt)
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw + 150;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //*****************************************************************************************
    // 编码器环pid
    // 目标值更改
    PID_FOR_L.desired = u - pidYaw.out;
    PID_FOR_R.desired = u + pidYaw.out;
    PID_BAC_L.desired = u - pidYaw.out;
    PID_BAC_R.desired = u + pidYaw.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Forward_Mode_Low(float dt)
{
    //	Serial_TxPacket[0] = 0XAA;
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);

    PID_FOR_L.desired = 1 * u - pidRateZ.out;
    PID_FOR_R.desired = 1 * u + pidRateZ.out;
    PID_BAC_L.desired = 1 * u - pidRateZ.out;
    PID_BAC_R.desired = 1 * u + pidRateZ.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Forward_Mode(float dt)
{
    //	Serial_TxPacket[0] = 0XAA;
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);

    PID_FOR_L.desired = 4 * u - pidRateZ.out;
    PID_FOR_R.desired = 4 * u + pidRateZ.out;
    PID_BAC_L.desired = 4 * u - pidRateZ.out;
    PID_BAC_R.desired = 4 * u + pidRateZ.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Back_Mode(float dt)
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);

    PID_FOR_L.desired = -5 * u - pidRateZ.out;
    PID_FOR_R.desired = -5 * u + pidRateZ.out;
    PID_BAC_L.desired = -5 * u - pidRateZ.out;
    PID_BAC_R.desired = -5 * u + pidRateZ.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Back_Mode_Low(float dt)
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);

    PID_FOR_L.desired = -1 * u - pidRateZ.out;
    PID_FOR_R.desired = -1 * u + pidRateZ.out;
    PID_BAC_L.desired = -1 * u - pidRateZ.out;
    PID_BAC_R.desired = -1 * u + pidRateZ.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Trans_Mode(float dt) // 横移模式
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);

    PID_FOR_L.desired = 7 * u - pidRateZ.out;
    PID_FOR_R.desired = -7 * u + pidRateZ.out;
    PID_BAC_L.desired = -7 * u - pidRateZ.out;
    PID_BAC_R.desired = 7 * u + pidRateZ.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Trans_Left_Mode(float dt) // 横移模式
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = 0;
    pidYaw.measured = Angle.yaw - Angle_Yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);

    PID_FOR_L.desired = -5 * u - pidRateZ.out;
    PID_FOR_R.desired = 5 * u + pidRateZ.out;
    PID_BAC_L.desired = 5 * u - pidRateZ.out;
    PID_BAC_R.desired = -5 * u + pidRateZ.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Trans_Right_Mode(float dt) // 横移模式
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = 0;
    pidYaw.measured = Angle.yaw - Angle_Yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);

    PID_FOR_L.desired = 5 * u - pidRateZ.out;
    PID_FOR_R.desired = -5 * u + pidRateZ.out;
    PID_BAC_L.desired = -5 * u - pidRateZ.out;
    PID_BAC_R.desired = 5 * u + pidRateZ.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Location_Mode(float dt)
{

    //************************************************************************************
    // 位置环pid

    pidLX.desired = 315;
    pidLY.desired = 225;
    if (LX == 0 || LY == 0) {
        pidLX.desired = 0;
        pidLY.desired = 0;
    }
    pidLX.measured = LX;
    pidLY.measured = LY;
    pidUpdate(&pidLX, dt);
    pidUpdate(&pidLY, dt);
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);
    //*****************************************************************************************
    // 编码器环pid
    // 目标值更改
    PID_FOR_L.desired = pidLX.out + pidLY.out - pidYaw.out;
    PID_FOR_R.desired = pidLX.out - pidLY.out + pidYaw.out;
    PID_BAC_L.desired = pidLX.out - pidLY.out - pidYaw.out;
    PID_BAC_R.desired = pidLX.out + pidLY.out + pidYaw.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Location_Mode_Place(float dt)
{

    //************************************************************************************
    // 位置环pid

    pidLX1.desired = 315;
    pidLY1.desired = 190;
    if (LX == 0 || LY == 0) {
        pidLX1.desired = 0;
        pidLY1.desired = 0;
    }
    pidLX1.measured = LX;
    pidLY1.measured = LY;
    pidUpdate(&pidLX1, dt);
    pidUpdate(&pidLY1, dt);
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //******************************************************************************
    // 角速度环pid
    pidRateZ.desired  = pidYaw.out;
    pidRateZ.measured = MPU6050.gyroZ * Gyro_G;
    pidUpdate(&pidRateZ, dt);
    //*****************************************************************************************
    // 编码器环pid
    // 目标值更改
    PID_FOR_L.desired = pidLX1.out + pidLY1.out - pidYaw.out;
    PID_FOR_R.desired = pidLX1.out - pidLY1.out + pidYaw.out;
    PID_BAC_L.desired = pidLX1.out - pidLY1.out - pidYaw.out;
    PID_BAC_R.desired = pidLX1.out + pidLY1.out + pidYaw.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Revolve_Mode(float dt)
{
    //*****************************************************************************************
    // 姿态环pid
    pidYaw.desired  = Angle_Yaw + 75;
    pidYaw.measured = Angle.yaw;
    pidUpdate(&pidYaw, dt);
    //*****************************************************************************************
    // 编码器环pid
    // 目标值更改
    PID_FOR_L.desired = u - pidYaw.out;
    PID_FOR_R.desired = u + pidYaw.out;
    PID_BAC_L.desired = u - pidYaw.out;
    PID_BAC_R.desired = u + pidYaw.out;
    //*****************************************************************************************
    // 测量值导入
    PID_FOR_L.measured = F_L_Speed;
    PID_FOR_R.measured = F_R_Speed;
    PID_BAC_L.measured = B_L_Speed;
    PID_BAC_R.measured = B_R_Speed;
    //*****************************************************************************************
    // pid运算
    pidUpdate(&PID_FOR_L, dt);
    pidUpdate(&PID_FOR_R, dt);
    pidUpdate(&PID_BAC_L, dt);
    pidUpdate(&PID_BAC_R, dt);
}
void Stop_Mode()
{
    Motor_left_front_wheel_SetSpeed(0);
    Motor_right_front_wheel_SetSpeed(0);
    Motor_left_back_wheel_SetSpeed(0);
    Motor_right_back_wheel_SetSpeed(0);
}
void Control_Moto()
{
    Motor_left_front_wheel_SetSpeed(PID_FOR_L.out);
    Motor_right_front_wheel_SetSpeed(PID_FOR_R.out);
    Motor_left_back_wheel_SetSpeed(PID_BAC_L.out);
    Motor_right_back_wheel_SetSpeed(PID_BAC_R.out);
}

// 1载物600，2载物1080，3载物1710，抓取600 1080 1710
//  0.85与地面交互 9.30抓起车载物料盘物料 1.9放下物料到车载物料盘 5.0抓起原料区物料
void Catch_Mode(unsigned char Color) // 将原料区物块抓取放置在托盘上的一系列移动
{
    switch (Color) {
        case 1:
            stage(stage_state1); //
            Micorstep_Enable();
            DOWN(3.50);
            Delay_ms(500);
            robotic_grab(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(5.50);
            cloud_tai(Cloud_Palace1);
            Delay_ms(500);
            Micorstep_Enable();
            DOWN(1.80);
            Delay_ms(500);
            robotic_grab(claw_free_stage);
            Micorstep_Enable();
            Delay_ms(500);
            UP(1.80);
            Delay_ms(500);
            cloud_tai(Cloud_Location);
            Micorstep_Enable();
            DOWN(2.00);
            Delay_ms(500);
            break;
        case 2:
            stage(stage_state2);
            Micorstep_Enable();
            DOWN(3.50);
            Delay_ms(500);
            robotic_grab(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(5.50);
            cloud_tai(Cloud_Palace2);
            Delay_ms(500);
            Micorstep_Enable();
            DOWN(1.80);
            Delay_ms(500);
            robotic_grab(claw_free_stage);
            Micorstep_Enable();
            Delay_ms(500);
            UP(1.80);
            Delay_ms(500);
            cloud_tai(Cloud_Location);
            Micorstep_Enable();
            DOWN(2.00);
            Delay_ms(500);
            break;
        case 3:
            stage(stage_state3);
            Micorstep_Enable();
            DOWN(3.50);
            Delay_ms(500);
            robotic_grab(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(5.50);
            cloud_tai(Cloud_Palace3);
            Delay_ms(500);
            Micorstep_Enable();
            DOWN(1.80);
            Delay_ms(500);
            robotic_grab(claw_free_stage);
            Micorstep_Enable();
            Delay_ms(500);
            UP(1.80);
            Delay_ms(500);
            cloud_tai(Cloud_Location);
            Micorstep_Enable();
            UP(1.00);
            Delay_ms(500);
            break;
    }
}

// 1载物600，2载物1080，3载物1710，抓取600 970 1710
//  0.85与地面交互 9.30抓起车载物料盘物料 1.9放下物料到车载物料盘 5.0抓起原料区物料
void Place_Mode(unsigned char Color) // 将物块放置在对应位置上的一系列移动
{
    if (Color == 1) {
        stage(stage_state1);
        cloud_tai(Cloud_Palace1);
        Delay_ms(500);
        Micorstep_Enable();
        DOWN(2.80);
        Delay_ms(500);
        robotic_grab(claw_grab);
        Delay_ms(500);
        Micorstep_Enable();
        UP(2.80);
        Delay_ms(500);
        cloud_tai(Cloud_Location);
        Micorstep_Enable();
        if (ALL_Place == 5) {
            DOWN(6.0);
        } else {
            DOWN(12.00);
        }
        Delay_ms(500);
        robotic_grab(claw_free);
        Micorstep_Enable();
        Delay_ms(500);
        if (ALL_Place == 5) {
            UP(6.0);
        } else {
            UP(12.00);
        }
        Delay_ms(500);
    }
    if (Color == 2) {
        stage(stage_state2);
        cloud_tai(Cloud_Palace2);
        Delay_ms(500);
        Micorstep_Enable();
        DOWN(2.80);
        Delay_ms(500);
        robotic_grab(claw_grab);
        Delay_ms(500);
        Micorstep_Enable();
        UP(2.80);
        Delay_ms(500);
        cloud_tai(Cloud_Location);
        Micorstep_Enable();
        if (ALL_Place == 5) {
            DOWN(6.0);
        } else {
            DOWN(12.00);
        }
        Delay_ms(500);
        robotic_grab(claw_free);
        Micorstep_Enable();
        Delay_ms(500);
        if (ALL_Place == 5) {
            UP(6.0);
        } else {
            UP(12.00);
        }
        Delay_ms(500);
    }
    if (Color == 3) {
        stage(stage_state3);
        cloud_tai(Cloud_Palace3);
        Delay_ms(500);
        Micorstep_Enable();
        DOWN(2.80);
        Delay_ms(500);
        robotic_grab(claw_grab);
        Delay_ms(500);
        Micorstep_Enable();
        UP(2.80);
        Delay_ms(500);
        cloud_tai(Cloud_Location);
        Micorstep_Enable();
        if (ALL_Place == 5) {
            DOWN(6.0);
        } else {
            DOWN(12.00);
        }
        Delay_ms(500);
        robotic_grab(claw_free);
        Micorstep_Enable();
        Delay_ms(500);
        if (ALL_Place == 5) {
            UP(6.0);
        } else {
            UP(12.00);
        }
        Delay_ms(500);
    }
}

// 1载物600，2载物1080，3载物1710，抓取600 970 1710  2 280 机械爪抓取    600 机械爪释放
//  0.85与地面交互 9.30抓起车载物料盘物料 1.9放下物料到车载物料盘 5.0抓起原料区物料
void Catch_Mode_St(unsigned char Color)
{
    switch (Color) {
        case 1:
            stage(stage_state1);
            Micorstep_Enable();
            DOWN(12.00);
            Delay_ms(1000);
            robotic_grab(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(12.00);
            Delay_ms(1000);
            cloud_tai(Cloud_Palace1);
            Delay_ms(500);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(1500);
            robotic_grab(claw_free);
            Micorstep_Enable();
            UP(1.9);
            Delay_ms(500);
            cloud_tai(Cloud_Location);
            break;
        case 2:
            stage(stage_state2);
            Micorstep_Enable();
            DOWN(12.00);
            Delay_ms(1000);
            robotic_grab(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(12.00);
            Delay_ms(1000);
            cloud_tai(Cloud_Palace2);
            Delay_ms(500);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(1500);
            robotic_grab(claw_free);
            Micorstep_Enable();
            UP(1.9);
            Delay_ms(500);
            cloud_tai(Cloud_Location);
            break;
        case 3:
            stage(stage_state3);
            Micorstep_Enable();
            DOWN(12.00);
            Delay_ms(1000);
            robotic_grab(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(12.00);
            Delay_ms(1000);
            cloud_tai(Cloud_Palace3);
            Delay_ms(500);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(1500);
            robotic_grab(claw_free);
            Micorstep_Enable();
            UP(1.9);
            Delay_ms(500);
            cloud_tai(Cloud_Location);
            break;
    }
}
void Place_ALL(unsigned char Place_F)
{
    if (Place_F == 1) { // 放红色
        if (P_1 == 0) {
            Serial_TxPacket[0] = 0xAE;
            Serial4_SendPacket();
            P_1 = 1;
        }
        if (LX != 0 && LY != 0 && Red_Place_Over == 0 && Place_Red == 0 && Place_Over_Red == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
            // Place_Over_Red = 1;
        }
        if (Write_Flag == 0xAE && Red_Place_Over == 0) {
            Mode_Flag      = STOP_MODE;
            Write_Flag     = 0x01;
            Place_Red      = 1;
            Red_Place_Over = 1;
        }
    } else if (Place_F == 2) { // 放绿色
        if (P_2 == 0) {
            Serial_TxPacket[0] = 0xAF;
            Serial4_SendPacket();
            P_2 = 1;
        }
        if (LX != 0 && LY != 0 && Green_Place_Over == 0 && Place_Green == 0 && Place_Over_Green == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
            // Place_Over_Green = 1;
        }
        if (Write_Flag == 0xAF && Green_Place_Over == 0) {
            Mode_Flag        = STOP_MODE;
            Write_Flag       = 0x01;
            Place_Green      = 1;
            Green_Place_Over = 1;
        }
    } else if (Place_F == 3) { // 放蓝色
        if (P_3 == 0) {
            Serial_TxPacket[0] = 0xBA;
            Serial4_SendPacket();
            P_3 = 1;
        }
        if (LX != 0 && LY != 0 && Blue_Place_Over == 0 && Place_Blue == 0 && Place_Over_Blue == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
            // Place_Over_Blue = 1;
        }
        if (Write_Flag == 0xBA && Blue_Place_Over == 0) {
            Mode_Flag       = STOP_MODE;
            Write_Flag      = 0x01;
            Place_Blue      = 1;
            Blue_Place_Over = 1;
        }
    }
}

void Place_Sthing()
{
    static unsigned char Frequence = 0;
    Frequence %= 3;
    Frequence++;
    Place_Mode(Frequence);
    Delay_ms(1000);
    if (Frequence == 1) {
        pidRest(pPidObject, 6);
        Find2 = 1;
        Find1 = 0;
        Find3 = 0;
        if (Serial_TxPacket[2] <= Serial_TxPacket[1]) {
            Mode_Flag = BACK_MODE_LOW;
        } else {
            Mode_Flag = FOR_MODE_LOW;
        }
    } else if (Frequence == 2) {
        pidRest(pPidObject, 6);
        Find3 = 1;
        Find1 = 0;
        Find2 = 0;
        if (Serial_TxPacket[3] <= Serial_TxPacket[2]) {
            Mode_Flag = BACK_MODE_LOW;
        } else {
            Mode_Flag = FOR_MODE_LOW;
        }
    } else if (Frequence == 3) {
        pidRest(pPidObject, 6);
        Find3 = 0;
        Find1 = 0;
        Find2 = 0;
        if (Serial_TxPacket[3] >= Serial_TxPacket[1]) {
            Mode_Flag = BACK_MODE_LOW;
        } else {
            Mode_Flag = FOR_MODE_LOW;
        }
        // if (ALL_Place == 2 || ALL_Place == 5) {
        //     Mode_Flag = REVOLVE_MODE_0;
        // }
    }
}

void Catch_All(unsigned char Catch_F)
{
    if (Catch_F == 1) {
        if (C_1 == 0) {
            Serial_TxPacket[0] = 0xBB;
            Serial4_SendPacket();
            C_1 = 1;
        }
        if (LX != 0 && LY != 0 && Catch_Over_Red == 0) {
            Mode_Flag = LOCATION_MODE;
        }
        if (Write_Flag == 0xBB && Catch_Over_Red == 0) {
            Mode_Flag      = STOP_MODE;
            Write_Flag     = 0x01;
            Catch_Red      = 1;
            Catch_Over_Red = 1;
        }
    } else if (Catch_F == 2) {
        if (C_2 == 0) {
            Serial_TxPacket[0] = 0xBC;
            Serial4_SendPacket();
            C_2 = 1;
        }
        if (LX != 0 && LY != 0 && Catch_Over_Green == 0) {
            Mode_Flag = LOCATION_MODE;
        }
        if (Write_Flag == 0xBC && Catch_Over_Green == 0) {
            Mode_Flag        = STOP_MODE;
            Write_Flag       = 0x01;
            Catch_Green      = 1;
            Catch_Over_Green = 1;
        }
    } else if (Catch_F == 3) {
        if (C_3 == 0) {
            Serial_TxPacket[0] = 0xBD;
            Serial4_SendPacket();
            C_3 = 1;
        }
        if (LX != 0 && LY != 0 && Catch_Over_Blue == 0) {
            Mode_Flag = LOCATION_MODE;
        }
        if (Write_Flag == 0xBD && Catch_Over_Blue == 0) {
            Mode_Flag       = STOP_MODE;
            Write_Flag      = 0x01;
            Catch_Blue      = 1;
            Catch_Over_Blue = 1;
        }
    }
}

void Catch_Sthing()
{
    static unsigned char Frequence = 0;
    Frequence %= 3;
    Frequence++;
    Catch_Mode_St(Frequence);
    Delay_ms(1000);
    if (Frequence == 1) {
        Catch2 = 1;
        Catch1 = 0;
        Catch3 = 0;
        if (Serial_TxPacket[2] <= Serial_TxPacket[1]) {
            Mode_Flag = BACK_MODE_LOW;
        } else {
            Mode_Flag = FOR_MODE_LOW;
        }
    }
    if (Frequence == 2) {
        Catch3 = 1;
        Catch2 = 0;
        Catch1 = 0;
        if (Serial_TxPacket[3] <= Serial_TxPacket[2]) {
            Mode_Flag = BACK_MODE_LOW;
        } else {
            Mode_Flag = FOR_MODE_LOW;
        }
    }
    if (Frequence == 3) {
        Catch3 = 0;
        Catch2 = 0;
        Catch1 = 0;
        // if (ALL_Place == 1 || ALL_Place == 4) {
        //     Mode_Flag = REVOLVE_MODE_0;
        // }
    }
}
