#include "tim_callback.h"

#define ANGLE_T				10
#define SPEED_T				50

uint16_t Count0 = 0;
uint16_t Count1 = 0;
uint32_t exec_us = 0;

MPU6050_Data_t MPU6050_Data;
int16_t AX, AY, AZ, GX, GY, GZ;

int16_t LeftPWM, RightPWM;
int16_t AvePWM, DifPWM;

float LeftSpeed, RightSpeed;
float AveSpeed, DifSpeed;

PID_t AnglePID = {
    .Kp = 3.2,
    .Ki = 0.1,
    .Kd = 4,

    .OutMax = 100,
    .OutMin = -100,

    .OutOffset_Positive = 5,
    .OutOffset_Negative = -4,
};

PID_t SpeedPID = {
    .Kp = 2,
    .Ki = 0.04,
    .Kd = 0,

    .OutMax = 10,
    .OutMin = -10,
 };

PID_t TurnPID = {
    .Kp = 6,
    .Ki = 4,
    .Kd = 0,
    .OutMax = 50,
    .OutMin = -50
};

uint16_t TimerCount = 0;
uint16_t TimerCount1 = 0;
/*
 *①中断里面的总程序执行时间是704 ~ 706us（已经优化至543，544us），而在那份可以标准库程序的执行时间仅为467~490us（耗时最长的时候）
 *②MPU6050_GetData(&MPU6050_Data);执行时间为578，579us(已经优化至416,417us),而在那份可标准库程序的执行时间仅为352，353us
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        Key_Scan();
        Count0 ++;
        Count1 ++;
        if (Count0 >= ANGLE_T)
        {

            MPU6050_GetData(&MPU6050_Data);

            Complementary_Filter(ANGLE_T / 1000.0, DifSpeed);

            if ((Angle > 50 || Angle < -50) && RunFlag == 1)//角度太大，则停止运行
            {
                RunFlag = 0;
            }

            if (RunFlag == 1) //只有运行了才进入PID计算
            {
                AnglePID.Actual = Angle;
                PID_Update(&AnglePID);
                AvePWM = AnglePID.Out;

                LeftPWM = AvePWM + DifPWM / 2;
                RightPWM = AvePWM - DifPWM / 2;

                if (LeftPWM > 100) {LeftPWM = 100;} else if (LeftPWM < -100) {LeftPWM = -100;}
                if (RightPWM > 100) {RightPWM = 100;} else if (RightPWM < -100) {RightPWM = -100;}

                Set_LMotorSpeed(LeftPWM);
                Set_RMotorSpeed(RightPWM);
            }
            else
            {
                Set_LMotorSpeed(0);
                Set_RMotorSpeed(0);
            }
            Count0 = 0;
        }
        if (Count1 >= SPEED_T)
        {
            Count1 = 0;

            LeftSpeed = Encoder_GetML() / 44.0 / 9.27666 / (SPEED_T / 1000.0);
            RightSpeed = Encoder_GetMR() / 44.0 / 9.27666 / (SPEED_T / 1000.0);

            AveSpeed = (LeftSpeed + RightSpeed) / 2.0;
            DifSpeed = LeftSpeed - RightSpeed;

            if (RunFlag == 1) //只有运行了才进入PID计算
            {
                SpeedPID.Actual = AveSpeed;
                PID_Update(&SpeedPID);
                AnglePID.Target = -SpeedPID.Out;

                TurnPID.Actual = DifSpeed;
                PID_Update(&TurnPID);
                DifPWM = TurnPID.Out;
            }
        }
        // TimerCount = __HAL_TIM_GET_COUNTER(&htim1);
        // if (TimerCount >= TimerCount1)TimerCount1 = TimerCount;
    }
}
