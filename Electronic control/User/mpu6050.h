#ifndef _MPU6050_H_
#define _MPU6050_H_
#include "stm32f10x.h"
#include "delay.h"
#include "IIC.h"
#include "flash.h"
#include <string.h>
#include <math.h>
#include "all_data.h"


#define squa( Sq )        (((float)Sq)*((float)Sq))

//const float M_PI = 3.1415926535;
//const float RtA = 57.2957795f;
//const float AtR = 0.0174532925f;
//const float Gyro_G = 0.03051756f*2;	  	//陀螺仪初始化量程+-2000度每秒于1 / (65536 / 4000) = 0.03051756*2		
//const float Gyro_Gr = 0.0005326f*2;     //面计算度每秒,转换弧度每秒则 2*0.03051756	 * 0.0174533f = 0.0005326*2



struct _1_ekf_filter //一维卡尔曼滤波
{
	float LastP;
	float	Now_P;
	float out;
	float Kg;
	float Q;
	float R;	
};

typedef volatile struct {
  float q0;
  float q1;
  float q2;
  float q3;
} Quaternion;

struct V{
	float x;
	float y;
	float z;
	};


int8_t MpuInit(void); //初始化
void MpuGetData(void); //读取陀螺仪数据加滤波
void MpuGetOffset(void); //校准
void GetAngle(const _st_Mpu *pMpu,_st_AngE *pAngE, float dt);
	
extern _st_Mpu MPU6050;
extern _st_AngE Angle;





#endif
