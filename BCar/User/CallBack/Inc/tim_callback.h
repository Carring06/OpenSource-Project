#ifndef CLION_TIM_CALLBACK_H
#define CLION_TIM_CALLBACK_H


#include "MPU6050.h"
#include "MyKey.h"
#include "CompFilter.h"
#include "PID.h"
#include "Motor.h"
#include "Encoder.h"

//外部变量说明
extern MPU6050_Data_t MPU6050_Data;
extern int16_t AX, AY, AZ, GX, GY, GZ;
extern PID_t AnglePID;
extern PID_t SpeedPID;
extern PID_t TurnPID;
extern PID_t SpeedControlPID;
extern uint8_t RunFlag;
extern float LeftSpeed, RightSpeed;
extern float AveSpeed, DifSpeed;
extern uint32_t exec_us;
extern uint16_t TimerCount;
extern uint16_t TimerCount1;

#endif //CLION_TIM_CALLBACK_H
