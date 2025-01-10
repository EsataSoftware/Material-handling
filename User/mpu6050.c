#include "mpu6050.h"

#define SMPLRT_DIV          0x19 // 陀螺仪采样率，典型值：0x07(125Hz)
#define CONFIGL             0x1A // 低通滤波频率，典型值：0x06(5Hz)
#define GYRO_CONFIG         0x1B // 陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define ACCEL_CONFIG        0x1C // 加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define ACCEL_ADDRESS       0x3B
#define ACCEL_XOUT_H        0x3B
#define ACCEL_XOUT_L        0x3C
#define ACCEL_YOUT_H        0x3D
#define ACCEL_YOUT_L        0x3E
#define ACCEL_ZOUT_H        0x3F
#define ACCEL_ZOUT_L        0x40
#define TEMP_OUT_H          0x41
#define TEMP_OUT_L          0x42
#define GYRO_XOUT_H         0x43
#define GYRO_ADDRESS        0x43
#define GYRO_XOUT_L         0x44
#define GYRO_YOUT_H         0x45
#define GYRO_YOUT_L         0x46
#define GYRO_ZOUT_H         0x47
#define GYRO_ZOUT_L         0x48
#define PWR_MGMT_1          0x6B // 电源管理，典型值：0x00(正常启用)
#define WHO_AM_I            0x75 // IIC地址寄存器(默认数值0x68，只读)
#define MPU6050_PRODUCT_ID  0x68
#define MPU6052C_PRODUCT_ID 0x72
#define SUCCESS             0
#define FAILED              1
#define MPU6050_ADDRESS     0xD0

_st_Mpu MPU6050;
_st_AngE Angle;

int16_t MpuOffset[6] = {0};

static volatile int16_t *pMpu = (int16_t *)&MPU6050;

void kalman_1(struct _1_ekf_filter *ekf, float input) // 一维卡尔曼
{
    ekf->Now_P = ekf->LastP + ekf->Q;
    ekf->Kg    = ekf->Now_P / (ekf->Now_P + ekf->R);
    ekf->out   = ekf->out + ekf->Kg * (input - ekf->out);
    ekf->LastP = (1 - ekf->Kg) * ekf->Now_P;
}

int8_t mpu6050_rest(void)
{
    if (IIC_Write_One_Byte(MPU6050_ADDRESS, PWR_MGMT_1, 0x80) == FAILED)
        return FAILED; // 复位
    Delay_ms(20);
    return SUCCESS;
}

int8_t MpuInit(void) // 初始化
{
    uint8_t date = SUCCESS;
    do {
        date = IIC_Write_One_Byte(MPU6050_ADDRESS, PWR_MGMT_1, 0x80); // 复位
        Delay_ms(30);
        date += IIC_Write_One_Byte(MPU6050_ADDRESS, SMPLRT_DIV, 0x02);   // 陀螺仪采样率，0x00(500Hz)
        date += IIC_Write_One_Byte(MPU6050_ADDRESS, PWR_MGMT_1, 0x03);   // 设置设备时钟源，陀螺仪Z轴
        date += IIC_Write_One_Byte(MPU6050_ADDRESS, CONFIGL, 0x03);      // 低通滤波频率，0x03(42Hz)
        date += IIC_Write_One_Byte(MPU6050_ADDRESS, GYRO_CONFIG, 0x18);  //+-2000deg/s
        date += IIC_Write_One_Byte(MPU6050_ADDRESS, ACCEL_CONFIG, 0x09); //+-4G
    } while (date != SUCCESS);
    date = IIC_Read_One_Byte(MPU6050_ADDRESS, 0x75);
    if (date != MPU6050_PRODUCT_ID)
        return FAILED;

    //	FLASH_read(MpuOffset,6);//从mcu的FLASH中读取MPU6050的水平静止标定校准值
    return SUCCESS;
}

#define Acc_Read()  IIC_read_Bytes(MPU6050_ADDRESS, 0X3B, buffer, 6)
#define Gyro_Read() IIC_read_Bytes(MPU6050_ADDRESS, 0x43, &buffer[6], 6)

void MpuGetData(void) // 读取陀螺仪数据加滤波
{
    uint8_t i;
    uint8_t buffer[12];

    Acc_Read();  // 去读加速度
    Gyro_Read(); // 读取角速度

    for (i = 0; i < 6; i++) {
        pMpu[i] = (((int16_t)buffer[i << 1] << 8) | buffer[(i << 1) + 1]) - MpuOffset[i]; // 整合为16bit，并减去水平静止校准值
        if (i < 3)                                                                        // 以下对加速度做卡尔曼滤波
        {
            {
                static struct _1_ekf_filter ekf[3] = {{0.02, 0, 0, 0, 0.001, 0.543}, {0.02, 0, 0, 0, 0.001, 0.543}, {0.02, 0, 0, 0, 0.001, 0.543}};
                kalman_1(&ekf[i], (float)pMpu[i]); // 一维卡尔曼
                pMpu[i] = (int16_t)ekf[i].out;
            }
        }
        if (i > 2) // 以下对角速度做一阶低通滤波
        {
            uint8_t k          = i - 3;
            const float factor = 0.15f; // 滤波因素
            static float tBuff[3];

            pMpu[i] = tBuff[k] = tBuff[k] * (1 - factor) + pMpu[i] * factor;
        }
    }
}

void MpuGetOffset(void) // 校准
{
    int32_t buffer[6] = {0};
    int16_t i;
    uint8_t k                   = 30;
    const int8_t MAX_GYRO_QUIET = 5;
    const int8_t MIN_GYRO_QUIET = -5;
    /*           wait for calm down    	                                                          */
    int16_t LastGyro[3] = {0};
    int16_t ErrorGyro[3];
    /*           set offset initial to zero    		*/

    memset(MpuOffset, 0, 12);
    MpuOffset[2] = 8192; // set offset from the 8192

    TIM_ITConfig( // 使能或者失能指定的TIM中断
        TIM6,
        TIM_IT_Update,
        DISABLE // 使能
    );
    while (k--) // 30次静止则判定处于静止状态
    {
        do {
            Delay_ms(10);
            MpuGetData();
            for (i = 0; i < 3; i++) {
                ErrorGyro[i] = pMpu[i + 3] - LastGyro[i];
                LastGyro[i]  = pMpu[i + 3];
            }
        } while ((ErrorGyro[0] > MAX_GYRO_QUIET) || (ErrorGyro[0] < MIN_GYRO_QUIET) // 标定静止
                 || (ErrorGyro[1] > MAX_GYRO_QUIET) || (ErrorGyro[1] < MIN_GYRO_QUIET) || (ErrorGyro[2] > MAX_GYRO_QUIET) || (ErrorGyro[2] < MIN_GYRO_QUIET));
    }

    /*           throw first 100  group data and make 256 group average as offset                    */
    for (i = 0; i < 356; i++) // 水平校准
    {
        MpuGetData();
        if (100 <= i) // 取256组数据进行平均
        {
            uint8_t k;
            for (k = 0; k < 6; k++) {
                buffer[k] += pMpu[k];
            }
        }
    }

    for (i = 0; i < 6; i++) {
        MpuOffset[i] = buffer[i] >> 8;
    }
    TIM_ITConfig( // 使能或者失能指定的TIM中断
        TIM6,
        TIM_IT_Update,
        ENABLE // 使能
    );
    //	FLASH_write(MpuOffset,6);//将数据写到FLASH中，一共有6个int16数据
}

/*
**函数 : Q_rsqrt
**功能 : 快速计算 1/Sqrt(x)
**输入 : number
**输出 : 结果
**备注 : None
**====================================================================================================*/
/*====================================================================================================*/
float Q_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = *(long *)&y;
    i  = 0x5f3759df - (i >> 1);
    y  = *(float *)&i;
    y  = y * (threehalfs - (x2 * y * y)); // 1st iteration （第一次牛顿迭代）
    return y;
}

const float M_PI    = 3.1415926535;
const float RtA     = 57.2957795f;
const float AtR     = 0.0174532925f;
const float Gyro_G  = 0.03051756f * 2; // 陀螺仪初始化量程+-2000度每秒于1 / (65536 / 4000) = 0.03051756*2
const float Gyro_Gr = 0.0005326f * 2;  // 面计算度每秒,转换弧度每秒则 2*0.03051756	 * 0.0174533f = 0.0005326*2

static float NormAcc;
Quaternion NumQ = {1, 0, 0, 0};

volatile struct V GyroIntegError = {0};

void GetAngle(const _st_Mpu *pMpu, _st_AngE *pAngE, float dt)
{
    volatile struct V Gravity, Acc, Gyro, AccGravity;

    static float KpDef = 0.5f;
    static float KiDef = 0.0001f;
    //		static  float KiDef = 0.00001f;

    float q0_t, q1_t, q2_t, q3_t;
    // float NormAcc;
    float NormQuat;
    float HalfTime = dt * 0.5f;

    // 提取等效旋转矩阵中的重力分量
    Gravity.x = 2 * (NumQ.q1 * NumQ.q3 - NumQ.q0 * NumQ.q2);
    Gravity.y = 2 * (NumQ.q0 * NumQ.q1 + NumQ.q2 * NumQ.q3);
    Gravity.z = 1 - 2 * (NumQ.q1 * NumQ.q1 + NumQ.q2 * NumQ.q2);
    // 加速度归一化
    NormAcc = Q_rsqrt(squa(MPU6050.accX) + squa(MPU6050.accY) + squa(MPU6050.accZ));

    Acc.x = pMpu->accX * NormAcc;
    Acc.y = pMpu->accY * NormAcc;
    Acc.z = pMpu->accZ * NormAcc;
    // 向量差乘得出的值
    AccGravity.x = (Acc.y * Gravity.z - Acc.z * Gravity.y);
    AccGravity.y = (Acc.z * Gravity.x - Acc.x * Gravity.z);
    AccGravity.z = (Acc.x * Gravity.y - Acc.y * Gravity.x);
    // 再做加速度积分补偿角速度的补偿值
    GyroIntegError.x += AccGravity.x * KiDef;
    GyroIntegError.y += AccGravity.y * KiDef;
    GyroIntegError.z += AccGravity.z * KiDef;
    // 角速度融合加速度积分补偿值
    Gyro.x = pMpu->gyroX * Gyro_Gr + KpDef * AccGravity.x + GyroIntegError.x; // 弧度制
    Gyro.y = pMpu->gyroY * Gyro_Gr + KpDef * AccGravity.y + GyroIntegError.y;
    Gyro.z = pMpu->gyroZ * Gyro_Gr + KpDef * AccGravity.z + GyroIntegError.z;
    // 一阶龙格库塔法, 更新四元数

    q0_t = (-NumQ.q1 * Gyro.x - NumQ.q2 * Gyro.y - NumQ.q3 * Gyro.z) * HalfTime;
    q1_t = (NumQ.q0 * Gyro.x - NumQ.q3 * Gyro.y + NumQ.q2 * Gyro.z) * HalfTime;
    q2_t = (NumQ.q3 * Gyro.x + NumQ.q0 * Gyro.y - NumQ.q1 * Gyro.z) * HalfTime;
    q3_t = (-NumQ.q2 * Gyro.x + NumQ.q1 * Gyro.y + NumQ.q0 * Gyro.z) * HalfTime;

    NumQ.q0 += q0_t;
    NumQ.q1 += q1_t;
    NumQ.q2 += q2_t;
    NumQ.q3 += q3_t;
    // 四元数归一化
    NormQuat = Q_rsqrt(squa(NumQ.q0) + squa(NumQ.q1) + squa(NumQ.q2) + squa(NumQ.q3));
    NumQ.q0 *= NormQuat;
    NumQ.q1 *= NormQuat;
    NumQ.q2 *= NormQuat;
    NumQ.q3 *= NormQuat;

    // 四元数转欧拉角
    {

#ifdef YAW_GYRO
        *(
            float *)pAngE = atan2f(2 * NumQ.q1 * NumQ.q2 + 2 * NumQ.q0 * NumQ.q3, 1 - 2 * NumQ.q2 * NumQ.q2 - 2 * NumQ.q3 * NumQ.q3) * RtA; // yaw
#else
        float yaw_G = pMpu->gyroZ * Gyro_G;
        if ((yaw_G > 1.0f) || (yaw_G < -1.0f)) // 数据太小可以认为是干扰，不是偏航动作
        {
            pAngE->yaw += yaw_G * dt;
        }
#endif
        pAngE->pitch = asin(2 * NumQ.q0 * NumQ.q2 - 2 * NumQ.q1 * NumQ.q3) * RtA;

        pAngE->roll = atan2(2 * NumQ.q2 * NumQ.q3 + 2 * NumQ.q0 * NumQ.q1, 1 - 2 * NumQ.q1 * NumQ.q1 - 2 * NumQ.q2 * NumQ.q2) * RtA; // PITCH
    }
}
