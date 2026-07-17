#ifndef COMPFILTER_H
#define COMPFILTER_H

#include "stm32f1xx_hal.h"
#include "MPU6050.h"
#include <math.h>
#include "../Inc/tim_callback.h"


extern float AngleAcc;
extern float AngleAcc_Raw;      // 原始加速度计角度（未滤波），用于调试
extern float AngleAcc_Filter;   // 经一阶低通滤波后的加速度计角度
extern float AngleGyro;
extern float Angle;

void Complementary_Filter(float dt, float DifSpeed);

#endif
