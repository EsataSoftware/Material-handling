#ifndef __ALL_DATA_H
#define __ALL_DATA_H


typedef struct{
	int16_t accX;
	int16_t accY;
	int16_t accZ;
	int16_t gyroX;
	int16_t gyroY;
	int16_t gyroZ;
}_st_Mpu;

typedef struct{
	float roll;
	float pitch;
	float yaw;
}_st_AngE;


typedef volatile struct
{
	float desired;     //< set point
	float offset;      //
	float prevError;    //< previous error
	float integ;        //< integral
	float kp;           //< proportional gain
	float ki;           //< integral gain
	float kd;           //< derivative gain
	float IntegLimitHigh;       //< integral limit
	float IntegLimitLow;
	float measured;
	float out;
	float OutLimitHigh;
	float OutLimitLow;
}PidObject;


typedef struct
{
    float lastP;		//上次的协方差
    float nowP;			//本次的协方差
    float x_hat;		//卡尔曼滤波的计算值，即为后验最优值
    float Kg;			//卡尔曼增益系数
    float Q;			//过程噪声
    float R;			//测量噪声
}Kalman;

extern PidObject PID_FOR_L,PID_FOR_R,PID_BAC_L,PID_BAC_R;
extern float F_L_Speed,F_R_Speed,B_L_Speed,B_R_Speed;
extern PidObject pidYaw;
extern PidObject pidRateZ;
extern PidObject pidLX;
extern PidObject pidLY;

extern PidObject pidLX1;
extern PidObject pidLY1;


extern _st_Mpu MPU6050;
extern _st_AngE Angle;
extern unsigned char Mode_Flag;//运行方式标志位
extern unsigned char Work_Mode;

extern const float Gyro_G;

extern float LX,LY,Angle_Yaw,LX1,LY1;
extern unsigned char Serial_TxPacket[10];//发送数据包
extern unsigned char Write_Flag;//寻白，寻黄色标志位
extern unsigned char Serial_Yaw;//串口传输过来的校准角度
extern unsigned char QrCode;
extern PidObject *(pPidObject[]);
extern unsigned char Catch_Flag;//抓取模式标志位
extern unsigned char Place_Red_Flag;
extern Kalman KF_X,KF_Y,KF_FL,KF_FR,KF_BL,KF_BR;

extern uint8_t Serial_TxPacket[10];

extern unsigned char Place_Red;
extern unsigned char Place_Blue;
extern unsigned char Place_Green;

extern unsigned char Red_Place_Over;
extern unsigned char Blue_Place_Over; 
extern unsigned char Green_Place_Over;
extern uint8_t Serial5_RxPacket[10];
extern Kalman KF_Angle;



extern unsigned char Catch_Red;
extern unsigned char Catch_Green;
extern unsigned char Catch_Blue;

extern float Last_Yaw;


#endif
