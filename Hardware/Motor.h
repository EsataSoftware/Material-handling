#ifndef __MOTOR_H
#define __MOTOR_H

void Motor_Init(void);
//void Motor_SetSpeed(int16_t lf_speed, int16_t rf_speed, int16_t lb_speed, int16_t rb_speed);
//void Motor_SetSpeed(int8_t Speed);

//void go_forward(int16_t lf_speed, int16_t rfront_speed, int16_t lb_speed, int16_t rb_speed);
//void go_back(int16_t lf_speed, int16_t rfront_speed, int16_t lb_speed, int16_t rb_speed);
//void go_left(int16_t lf_speed, int16_t rfront_speed, int16_t lb_speed, int16_t rb_speed);
//void go_right(int16_t lf_speed, int16_t rfront_speed, int16_t lb_speed, int16_t rb_speed);
//void Motor_Stop(void);




void Motor_left_front_wheel_SetSpeed(int16_t lf_speed)  ;  //左前电机正反转



void Motor_right_front_wheel_SetSpeed(int16_t rfront_speed);		//右前电机正反转




void Motor_left_back_wheel_SetSpeed(int16_t lb_speed);		//左后电机正反转




 void Motor_right_back_wheel_SetSpeed(int16_t rb_speed);		//右后电机正反转




void go_forward1(int16_t lf_speed, int16_t rf_speed, int16_t lb_speed, int16_t rb_speed);

void Stop(void);
void ForWard(void);

void Test(void);

void Adjust_Left(void);

void Adjust_Right(void);

void Back(void);


void Big_Adjust_Left(void);

void Big_Adjust_Right(void);


void Turn_Left(void);

void Turn_Right(void);


void Forward_Left_Out(void);



void Forward_Right_Out(void);



void Back_Left_Out(void);


void Back_Right_Out(void);



void Moto_Out(void);


void Serial_Out(void);






#endif
