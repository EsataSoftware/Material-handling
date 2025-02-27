#include "filter.h"

Kalman KF_X, KF_Y, KF_FL, KF_FR, KF_BL, KF_BR;
Kalman KF_Angle;

void Kalman_Init()
{
    KF_Angle.Q     = 0.225; // 过程噪声可以认为是0
    KF_Angle.R     = 0.425; // 给一个较小的值，可以在debug中调节
    KF_Angle.Kg    = 0;
    KF_Angle.lastP = 1; // lastP相当于上一次的值，初始值可以为1，不可以为0
    KF_Angle.x_hat = 0;

    KF_Y.Q     = 0.225; // 过程噪声可以认为是0
    KF_Y.R     = 0.425; // 给一个较小的值，可以在debug中调节
    KF_Y.Kg    = 0;
    KF_Y.lastP = 1; // lastP相当于上一次的值，初始值可以为1，不可以为0
    KF_Y.x_hat = 0;

    KF_FL.Q     = 0.225; // 过程噪声可以认为是0
    KF_FL.R     = 0.425; // 给一个较小的值，可以在debug中调节
    KF_FL.Kg    = 0;
    KF_FL.lastP = 1; // lastP相当于上一次的值，初始值可以为1，不可以为0
    KF_FL.x_hat = 0;

    KF_FR.Q     = 0.225; // 过程噪声可以认为是0
    KF_FR.R     = 0.425; // 给一个较小的值，可以在debug中调节
    KF_FR.Kg    = 0;
    KF_FR.lastP = 1; // lastP相当于上一次的值，初始值可以为1，不可以为0
    KF_FR.x_hat = 0;

    KF_BL.Q     = 0.225; // 过程噪声可以认为是0
    KF_BL.R     = 0.425; // 给一个较小的值，可以在debug中调节
    KF_BL.Kg    = 0;
    KF_BL.lastP = 1; // lastP相当于上一次的值，初始值可以为1，不可以为0
    KF_BL.x_hat = 0;

    KF_BR.Q     = 0.225; // 过程噪声可以认为是0
    KF_BR.R     = 0.425; // 给一个较小的值，可以在debug中调节
    KF_BR.Kg    = 0;
    KF_BR.lastP = 1; // lastP相当于上一次的值，初始值可以为1，不可以为0
    KF_BR.x_hat = 0;
}

float Kalman_Filter(Kalman *KF, float input)
{
    float output = 0, x_t;                        // output为卡尔曼滤波计算值
    x_t          = KF->x_hat;                     // 当前先验预测值 = 上一次最优值
    KF->nowP     = KF->lastP + KF->Q;             // 本次的协方差矩阵
    KF->Kg       = KF->nowP / (KF->nowP + KF->R); // 卡尔曼增益系数计算
    output       = x_t + KF->Kg * (input - x_t);  // 当前最优值
    KF->x_hat    = output;                        // 更新最优值
    KF->lastP    = (1 - KF->Kg) * KF->nowP;       // 更新协方差矩阵
    return output;
}
