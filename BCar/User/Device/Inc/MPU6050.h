//
// Created by Car on 2026/6/22.
//

#ifndef CLION_MPU6050_H
#define CLION_MPU6050_H

#include "main.h"
#include "H_I2C.h"


#define	MPU6050_SMPLRT_DIV		0x19
#define	MPU6050_CONFIG			0x1A
#define	MPU6050_GYRO_CONFIG		0x1B
#define	MPU6050_ACCEL_CONFIG	0x1C

#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42
#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48

#define	MPU6050_PWR_MGMT_1		0x6B
#define	MPU6050_PWR_MGMT_2		0x6C
#define	MPU6050_WHO_AM_I		0x75



typedef struct {
    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
    int16_t Accel_X;
    int16_t Accel_Y;
    int16_t Accel_Z;
}MPU6050_Data_t;


void MPU6050_Init(void);
void MPU6050_WriteData(uint8_t Address, uint8_t data);
int8_t MPU6050_ReadData(uint8_t Address);
void MPU6050_GetData(MPU6050_Data_t *MPU6050_Data);
void MPU6050_GetData0(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);

/*
 * 陀螺仪Y轴零偏值，由 MPU6050_Calibrate() 计算
 * 使用方式：在互补滤波中 (MPU6050_Data.Gyro_Y + GY_Offset) 得到校准后的角速度
 */
extern int16_t GY_Offset;

/*
 * 加速度计角度偏移值（单位：度），由 MPU6050_Calibrate() 计算
 * 使用方式：在互补滤波中 AngleAcc_Raw + AngleAcc_Offset 得到校准后的角度
 */
extern float AngleAcc_Offset;

/*
 * MPU6050_Calibrate  上电静止校准函数
 * 功能：采集200组静止状态下的陀螺仪和加速度计数据，
 *       计算 GY_Offset（陀螺仪Y轴零偏）和 AngleAcc_Offset（加速度计角度偏移），
 *       用于 Complementary_Filter() 中对原始数据进行校准。
 * 用法：在 MPU6050_Init() 之后调用一次即可。
 * 注意：校准时小车必须水平静止放置在平面上，校准过程约持续1秒。
 */
void MPU6050_Calibrate(void);
#endif //CLION_MPU6050_H
