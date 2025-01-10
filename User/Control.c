#include "control.h"
#include "Servo.h"
#include "OLED.h"
unsigned char Mode_Flag = TRANS_LEFT_MODE;
unsigned char Work_Mode;
unsigned int Timer            = 0;
unsigned char Stop_Flag       = 0;
unsigned char Location_Flag   = 0;
unsigned int Trans_Flag       = 0;
unsigned char Tran_Flag       = 0;
unsigned char Start_Flag      = 0; // 此处调整边线
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

unsigned char Search_Flag      = 0;
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

unsigned char Search_Flag2 = 0;

unsigned char Find2_1  = 1, Find2_2, Find2_3;
unsigned char Catch2_1 = 1, Catch2_2, Catch2_3;

#define Cloud_Location    1080 // 云台观察位置//1080 1700
#define Could_Palace      2300 // 云台放置载物台上位置//2300
#define stage_datum_point 1200 // 载物台初始位置
#define stage_state1      330  // 载物台物块1放置
#define stage_state2      970  // 载物台物块2放置
#define stage_state3      1710 // 载物台物块3放置
#define claw_grab         300  // 机械爪抓取
#define claw_free         500  // 机械爪释放
#define cloud_place       2250

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
        if (Trans_Flag >= 250 && Start_Flag == 0) // 代码开始左平移出库，之后到达计时时间直走
        {
            if (QrCode == 0) { // 没扫到二维码之前慢走
                Mode_Flag = FOR_MODE_LOW;
            } else if (QrCode == 1) { // 扫到了二维码
                Mode_Flag = FOR_MODE;
                // OLED_ShowNum_high(2, 1, Serial_TxPacket[0], 5);
                Start_Flag = 1; // 起始标志位置1无法再次进入起始条件
            }
        }
        if (Trans_Flag >= 1900 && Stop_Flag == 0) // 寻找到白色进入此状态
        {
            Serial_TxPacket[0] = 0XFA; // 树莓派定位转盘
            Serial_SendPacket();
            Mode_Flag = TRANS_RIGHT_MODE;
            OLED_ShowString_high(2, 1, "Trans");
            if (Trans_Flag >= 2050) {
                OLED_ShowString_high(2, 1, "STOP!");
                Mode_Flag  = STOP_MODE;
                Write_Flag = 0;
                Stop_Flag  = 1;
            }
            // if (LX != 0 && LY != 0) {
            // 回传位置数据跳出停止状态
            // }
        }
        if (LX != 0 && LY != 0 && Location_Flag == 0 && Stop_Flag == 1) // 防止状态跳回此处加入STOP_MODE
        {
            Mode_Flag = LOCATION_MODE; // 进入定位状态
            OLED_ShowString_high(2, 1, "LOCAT");
            Location_Flag = 1;
        }
        if (Write_Flag == 0XCD && Mode_Flag == LOCATION_MODE) { // 进入抓取状态
            Mode_Flag = STOP_MODE;
            OLED_ShowString_high(2, 1, "STOP");
            Catch_Flag = 1; // 抓取标志位置
        }
        if (Angle.yaw >= Angle_Yaw + 75 && Mode_Flag == REVOLVE_MODE_90) {
            pidRest(pPidObject, 6);                // 数据复位
            Mode_Flag          = TRANS_RIGHT_MODE; // 切换模式到平移模式
            Angle_Yaw          = Angle.yaw;
            Serial_TxPacket[0] = 0XAD;
            Serial_SendPacket();
        }
        if (Write_Flag == 0XAD && Mode_Flag == TRANS_RIGHT_MODE && Catch_Flag == 1) {
            Mode_Flag          = REVOLVE_MODE_90;
            Write_Flag         = 0x01;
            Serial_TxPacket[0] = 0X01; //放置树莓派重复进入状态
            Serial_SendPacket();
        } else if (Mode_Flag == REVOLVE_MODE_90 && Catch_Flag == 0) {
            Mode_Flag          = TRANS_RIGHT_MODE;
            Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
            Serial_SendPacket();
            Tran_Flag = 1;
        } else if ((Mode_Flag == TRANS_RIGHT_MODE || Mode_Flag == STOP_MODE) && Catch_Flag == 1 && Tran_Flag == 1) {//还未使用过的姿态调节
            Mode_Flag          = STOP_MODE;
            Serial_TxPacket[0] = 0XAD;// 给树莓派寻黄的指令
            Serial_SendPacket();
            Adjust_Timer++;
            if (Adjust_Timer >= 400) {
                Angle_Yaw          = 0.41 * (90 - Last_Yaw) + Angle.yaw;
                Trans_Flag         = 0;
                Adjust_Timer       = 0;
                Serial_TxPacket[0] = 0XAD;
                Serial_SendPacket();
                Tran_Flag    = 0;
                Adjust_Timer = 0;
                Mode_Flag    = TRANS_RIGHT_MODE;
                Search_Flag  = 0;
                ALL_Place++;
            }
        }
    } else if (ALL_Place == 1 || ALL_Place == 4) {
        if (Write_Flag == 0xAD && Mode_Flag == TRANS_RIGHT_MODE && Catch_Flag == 1 && Catch_11 == 0) {
            Mode_Flag          = FOR_MODE;
            Serial_TxPacket[0] = 0XAA;
            Serial_SendPacket();                            // 给树莓派寻白的指令
        } else if (Write_Flag == 0XAA && Catch_Flag == 1) { // 寻找到白色进入此状态
            Mode_Flag          = LOCATION_MODE;
            Serial_TxPacket[0] = 0XFB; // 树莓派定位色环
            Serial_SendPacket();
            if (LX != 0 && LY != 0) {
                Write_Flag  = 0;
                Search_Flag = 1;
                Serial_SendPacket();
            }
        }
        if (Find1 == 1 && Search_Flag == 1) {

            Place_ALL(Serial_TxPacket[1]);
        }
        if (Find2 == 1) {

            Place_ALL(Serial_TxPacket[2]);
        }
        if (Find3 == 1) {

            Place_ALL(Serial_TxPacket[3]);
        }
        if (Catch1 == 1 && Find1 == 0 && Find2 == 0 && Find3 == 0) { // 放置到粗加工后再夹起来
            if (Catch_11 == 0) {
                if (Serial_TxPacket[1] <= Serial_TxPacket[3]) {
                    Mode_Flag = BACK_MODE;
                } else {
                    Mode_Flag = FOR_MODE;
                }
                Catch_11 = 1;
            }

            Catch_All(Serial_TxPacket[1]);
        }
        if (Catch2 == 1) {
            Catch_All(Serial_TxPacket[2]);
        }
        if (Catch3 == 1) {
            Catch_All(Serial_TxPacket[3]);
        }

        if (Angle.yaw >= Angle_Yaw - 75 && Mode_Flag == REVOLVE_MODE_0) // 此处Mode_Flag由main函数设置
        {
            pidRest(pPidObject, 6);                // 数据复位
            Mode_Flag          = TRANS_RIGHT_MODE; // 切换模式到平移模式
            Angle_Yaw          = Angle_Yaw;
            Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
            Serial_SendPacket();
        }
        if (Write_Flag == 0XAD && (Mode_Flag == TRANS_RIGHT_MODE || Mode_Flag == STOP_MODE)) {
            Mode_Flag = STOP_MODE;
            Adjust_Timer++;
            if (Adjust_Timer >= 400) {
                Angle_Yaw          = 0.41 * (90 - Last_Yaw) + Angle.yaw;
                Mode_Flag          = BACK_MODE;
                Serial_TxPacket[0] = 0XAA;
                Serial_SendPacket();
                Search_Flag2 = 1;
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
            }
        }
    } else if (ALL_Place == 2 || ALL_Place == 5) {
        if (Find1 == 1 && Search_Flag2 == 1) {
            Place_ALL(Serial_TxPacket[1]);
        }
        if (Find2 == 1) {

            Place_ALL(Serial_TxPacket[2]);
        }
        if (Find3 == 1) {

            Place_ALL(Serial_TxPacket[3]);
        }
        if (Angle.yaw <= Angle_Yaw - 75 && Mode_Flag == REVOLVE_MODE_0 && Tran_Flag == 0) // 此处Mode_Flag由main函数设置
        {
            pidRest(pPidObject, 6);       // 数据复位
            Mode_Flag = TRANS_RIGHT_MODE; // 切换模式到平移模式
            Angle_Yaw = Angle_Yaw - 75;
            Tran_Flag = 1;
        }

        if (Write_Flag == 0XAD && (Mode_Flag == TRANS_RIGHT_MODE || Mode_Flag == STOP_MODE)) {
            Serial_TxPacket[0] = 0XAA;
            Mode_Flag          = BACK_MODE;
            Serial_SendPacket();
            Adjust_Timer = 0;
        }
    }
    if (Write_Flag == 0XAA && (Mode_Flag == BACK_MODE || Mode_Flag == STOP_MODE)) {
        if (ALL_Place == 2) {
            Serial_TxPacket[0] = 0XFA; // 进入寻白模式
            Serial_SendPacket();
        } else if (ALL_Place == 5) {
            Serial_TxPacket[0] = 0XEA; // 结束寻蓝
            Serial_SendPacket();
        }
        Serial_SendPacket();
        ALL_Place++;
        Stop_Flag        = 0;
        Location_Flag    = 0;
        Catch_Flag       = 0;
        Find1            = 1;
        Catch1           = 1;
        P_1              = 0;
        P_2              = 0;
        P_3              = 0;
        C_1              = 0;
        C_2              = 0;
        C_3              = 0;
        Adjust_Timer     = 0;
        Search_Flag      = 1;
        Place_Over_Red   = 0;
        Place_Over_Blue  = 0;
        Place_Over_Green = 0;
        Catch_Over_Red   = 0;
        Catch_Over_Blue  = 0;
        Catch_Over_Green = 0;
        Red_Place_Over   = 0;
        Blue_Place_Over  = 0;
        Green_Place_Over = 0;
    } else if (Write_Flag == 0xEA && (Mode_Flag == BACK_MODE || Mode_Flag == STOP_MODE)) {
        Mode_Flag          = TRANS_RIGHT_MODE;
        Serial_TxPacket[0] = 0XAA;
        Serial_SendPacket();
        Adjust_Timer++;
        if (Adjust_Timer >= 160) {
            Mode_Flag = STOP_MODE;
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

    PID_FOR_L.desired = 2 * u - pidRateZ.out;
    PID_FOR_R.desired = 2 * u + pidRateZ.out;
    PID_BAC_L.desired = 2 * u - pidRateZ.out;
    PID_BAC_R.desired = 2 * u + pidRateZ.out;
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

    PID_FOR_L.desired = -4 * u - pidRateZ.out;
    PID_FOR_R.desired = -4 * u + pidRateZ.out;
    PID_BAC_L.desired = -4 * u - pidRateZ.out;
    PID_BAC_R.desired = -4 * u + pidRateZ.out;
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
    PID_FOR_R.desired = -4 * u + pidRateZ.out;
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

    pidLX.desired = 320;
    pidLY.desired = 250;
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

    pidLX.desired = 320;
    pidLY.desired = 250;
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

// 1载物355，2载物970，3载物1710，抓取330 970 1710
//  0.85与地面交互 9.30抓起车载物料盘物料 1.9放下物料到车载物料盘 5.0抓起原料区物料
void Catch_Mode(unsigned char Color) // 将原料区物块抓取放置在托盘上的一系列移动
{
    switch (Color) {
        case 1:
            PWM1_SetCompare3(350); //
            Micorstep_Enable();
            DOWN(5.00);
            Delay_ms(500);
            PWM1_SetCompare2(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(5.00);
            PWM1_SetCompare4(2300);
            Delay_ms(1500);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(500);
            PWM1_SetCompare2(600);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(1.9);
            cloud_tai(1080);
            break;
        case 2:
            PWM1_SetCompare3(1000);
            Micorstep_Enable();
            DOWN(5.00);
            Delay_ms(500);
            PWM1_SetCompare2(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(5.00);
            cloud_tai(2300);
            Delay_ms(1500);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(500);
            PWM1_SetCompare2(600);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(1.9);
            cloud_tai(1080);
            break;
        case 3:
            PWM1_SetCompare3(1710);
            Micorstep_Enable();
            DOWN(5.00);
            Delay_ms(500);
            PWM1_SetCompare2(claw_grab);
            Delay_ms(500);
            Micorstep_Enable();
            UP(5.00);
            cloud_tai(2300);
            Delay_ms(1500);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(500);
            PWM1_SetCompare2(600);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(1.9);
            cloud_tai(1080);
            break;
    }
}

// 1载物355，2载物970，3载物1710，抓取330 970 1710
//  0.85与地面交互 9.30抓起车载物料盘物料 1.9放下物料到车载物料盘 5.0抓起原料区物料
void Place_Mode(unsigned char Color) // 将物块放置在对应位置上的一系列移动
{
    if (Color == 1) {
        PWM1_SetCompare3(355);
        cloud_tai(2300);
        Micorstep_Enable();
        Delay_ms(1500);
        DOWN(1.9);
        Delay_ms(1000);
        PWM1_SetCompare2(300);
        Delay_ms(1000);
        Micorstep_Enable();
        UP(1.9);
        cloud_tai(1080);
        Delay_ms(1000);
        Micorstep_Enable();
        Delay_ms(1000);
        if (ALL_Place == 5) { // 记得改数值还没测
            DOWN(6.0);
        } else {
            DOWN(10.35);
        }
        Delay_ms(1000);
        PWM1_SetCompare2(500);
        Micorstep_Enable();
        if (ALL_Place == 5) {
            UP(6.0);
        } else {
            UP(10.35);
        }
        Delay_ms(1000);
    }
    if (Color == 2) {
        PWM1_SetCompare3(970);
        cloud_tai(2300);
        Micorstep_Enable();
        Delay_ms(500);
        DOWN(1.9);
        Delay_ms(1000);
        PWM1_SetCompare2(300);
        Delay_ms(1000);
        Micorstep_Enable();
        UP(1.9);
        cloud_tai(1080);
        Delay_ms(1000);
        Micorstep_Enable();
        Delay_ms(1000);
        if (ALL_Place == 5) {
            DOWN(6.0);
        } else {
            DOWN(10.35);
        }
        Delay_ms(1000);
        PWM1_SetCompare2(500);
        Micorstep_Enable();
        if (ALL_Place == 5) {
            UP(6.0);
        } else {
            UP(10.35);
        }
        Delay_ms(1000);
    }
    if (Color == 3) {
        PWM1_SetCompare3(1710);
        cloud_tai(2300);
        Micorstep_Enable();
        Delay_ms(500);
        DOWN(1.90);
        Delay_ms(1000);
        PWM1_SetCompare2(300);
        Delay_ms(1000);
        Micorstep_Enable();
        UP(1.90);
        cloud_tai(1080);
        Delay_ms(1000);
        Micorstep_Enable();
        Delay_ms(1000);
        if (ALL_Place == 5) {
            DOWN(6.0);
        } else {
            DOWN(10.35);
        }
        Delay_ms(1000);
        PWM1_SetCompare2(500);
        Micorstep_Enable();
        if (ALL_Place == 5) {
            UP(6.0);
        } else {
            UP(10.35);
        }
        Delay_ms(1000);
    }
}

// 3 1载物355，2载物970，3载物1710，抓取330 970 1710    2 350 机械爪抓取    500 机械爪释放
//  0.85与地面交互 9.30抓起车载物料盘物料 1.9放下物料到车载物料盘 5.0抓起原料区物料
void Catch_Mode_St(unsigned char Color)
{
    switch (Color) {
        case 1:
            PWM1_SetCompare3(355);
            Micorstep_Enable();
            Delay_ms(500);
            DOWN(10.35);
            Delay_ms(1000);
            PWM1_SetCompare2(300);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(10.35);
            Delay_ms(1000);
            cloud_tai(2300);
            Delay_ms(1000);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(1000);
            PWM1_SetCompare2(500);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(1.9);
            Delay_ms(1000);
            PWM1_SetCompare2(500);
            PWM1_SetCompare3(960);
            cloud_tai(1080);
            break;
        case 2:
            PWM1_SetCompare3(960);
            Micorstep_Enable();
            Delay_ms(500);
            DOWN(10.35);
            Delay_ms(1000);
            PWM1_SetCompare2(300);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(10.35);
            Delay_ms(1000);
            cloud_tai(2300);
            Delay_ms(1000);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(1000);
            PWM1_SetCompare2(500);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(1.9);
            Delay_ms(1000);
            PWM1_SetCompare2(500);
            PWM1_SetCompare3(1710);
            cloud_tai(1080);
            break;
        case 3:
            PWM1_SetCompare3(1710);
            Micorstep_Enable();
            Delay_ms(500);
            DOWN(10.35);
            Delay_ms(1000);
            PWM1_SetCompare2(300);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(10.35);
            Delay_ms(1000);
            cloud_tai(2300);
            Delay_ms(1000);
            Micorstep_Enable();
            DOWN(1.9);
            Delay_ms(1000);
            PWM1_SetCompare2(500);
            Delay_ms(1000);
            Micorstep_Enable();
            UP(1.9);
            Delay_ms(1000);
            PWM1_SetCompare2(500);
            PWM1_SetCompare3(355);
            cloud_tai(1080);
            break;
    }
}
void Place_ALL(unsigned char Place_F)
{
    if (Place_F == 1) { // 放红色
        if (P_1 == 0) {
            Serial_TxPacket[0] = 0xAE;
            Serial_SendPacket();
            P_1 = 1;
        }

        if (LX != 0 && LY != 0 && Red_Place_Over == 0 && Place_Red == 0 && Place_Over_Red == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
        }
        if (LX >= 450 && LX <= 750 && LY >= 140 && LY <= 440 && Red_Place_Over == 0) {
            Mode_Flag = STOP_MODE;
            if (Place_Over_Red == 0) {
                Serial_TxPacket[0] = 0x01;
                Serial_SendPacket();
                Place_Red      = 1;
                Place_Over_Red = 1;
            }
        }
    } else if (Place_F == 2) { // 放绿色
        if (P_2 == 0) {
            Serial_TxPacket[0] = 0xAF;
            Serial_SendPacket();
            P_2 = 1;
        }
        if (LX != 0 && LY != 0 && Green_Place_Over == 0 && Place_Green == 0 && Place_Over_Green == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
        }
        if (LX >= 450 && LX <= 750 && LY >= 140 && LY <= 440 && Green_Place_Over == 0) {
            Mode_Flag = STOP_MODE;
            if (Place_Over_Green == 0) {
                Serial_TxPacket[0] = 0x01;
                Serial_SendPacket();
            }
            if (Place_Over_Green == 0) {
                Place_Green      = 1;
                Place_Over_Green = 1;
            }
        }
    } else if (Place_F == 3) { // 放蓝色
        if (P_3 == 0) {
            Serial_TxPacket[0] = 0xBA;
            Serial_SendPacket();
            P_3 = 1;
        }
        if (LX != 0 && LY != 0 && Blue_Place_Over == 0 && Place_Blue == 0 && Place_Over_Blue == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
        }
        if (LX >= 450 && LX <= 750 && LY >= 140 && LY <= 440 && Blue_Place_Over == 0) {
            Mode_Flag = STOP_MODE;
            if (Place_Over_Blue == 0) {
                Serial_TxPacket[0] = 0x01;
                Serial_SendPacket();
            }
            if (Place_Over_Blue == 0) {
                Place_Blue      = 1;
                Place_Over_Blue = 1;
            }
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
            Mode_Flag = BACK_MODE;
        } else {
            Mode_Flag = FOR_MODE;
        }
    } else if (Frequence == 2) {
        pidRest(pPidObject, 6);
        Find3 = 1;
        Find1 = 0;
        Find2 = 0;
        if (Serial_TxPacket[3] <= Serial_TxPacket[2]) {
            Mode_Flag = BACK_MODE;
        } else {
            Mode_Flag = FOR_MODE;
        }
    } else if (Frequence == 3) {
        pidRest(pPidObject, 6);
        Find3 = 0;
        Find1 = 0;
        Find2 = 0;
        if (ALL_Place == 2 || ALL_Place == 5) {
            Mode_Flag          = REVOLVE_MODE_0;
            Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
            Serial_SendPacket();
        }
    }
}

void Catch_All(unsigned char Catch_F)
{
    if (Catch_F == 1) {
        if (C_1 == 0) {
            Serial_TxPacket[0] = 0xBB;
            Serial_SendPacket();
            C_1 = 1;
        }

        if (LX != 0 && LY != 0 && Catch_Over_Red == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
        }
        if (LX >= 450 && LX <= 750 && LY >= 140 && LY <= 440 && Catch_Over_Red == 0) {
            Mode_Flag          = STOP_MODE;
            Serial_TxPacket[0] = 0x01;
            Serial_SendPacket();
            Catch_Red      = 1;
            Catch_Over_Red = 1;
        }
    } else if (Catch_F == 2) {
        if (C_2 == 0) {
            Serial_TxPacket[0] = 0xBC;
            Serial_SendPacket();
            C_2 = 1;
        }
        if (LX != 0 && LY != 0 && Catch_Over_Green == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
        }
        if (LX >= 450 && LX <= 750 && LY >= 140 && LY <= 440 && Catch_Over_Green == 0) {
            Mode_Flag          = STOP_MODE;
            Serial_TxPacket[0] = 0x01;
            Serial_SendPacket();
            Catch_Green      = 1;
            Catch_Over_Green = 1;
        }
    }

    else if (Catch_F == 3) {
        if (C_3 == 0) {
            Serial_TxPacket[0] = 0xBD;
            Serial_SendPacket();
            C_3 = 1;
        }
        if (LX != 0 && LY != 0 && Catch_Over_Blue == 0) {
            Mode_Flag = LOCATION_PLACE_MODE;
        }
        if (LX >= 450 && LX <= 750 && LY >= 140 && LY <= 440 && Catch_Over_Blue == 0) {
            Mode_Flag          = STOP_MODE;
            Serial_TxPacket[0] = 0x01;
            Serial_SendPacket();
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
            Mode_Flag = BACK_MODE;
        } else {
            Mode_Flag = FOR_MODE;
        }
    }
    if (Frequence == 2) {
        Catch3 = 1;
        Catch2 = 0;
        Catch1 = 0;
        if (Serial_TxPacket[3] <= Serial_TxPacket[2]) {
            Mode_Flag = BACK_MODE;
        } else {
            Mode_Flag = FOR_MODE;
        }
    }
    if (Frequence == 3) {
        Catch3 = 0;
        Catch2 = 0;
        Catch1 = 0;
        if (ALL_Place == 1 || ALL_Place == 4) {
            Mode_Flag          = REVOLVE_MODE_0;
            Serial_TxPacket[0] = 0XAD; // 给树莓派寻黄的指令
            Serial_SendPacket();
        }
    }
}
