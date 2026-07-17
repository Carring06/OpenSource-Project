#include "MPU6050.h"
#include <math.h>

#define MPU6050_Address        0xD0               //HAL库使用8位地址（左对齐），0x68 << 1 = 0xD0

/*
 * GY_Offset   — 陀螺仪Y轴零偏值，由 MPU6050_Calibrate() 计算
 *               使用方式：在互补滤波中 (MPU6050_Data.Gyro_Y + GY_Offset) 得到校准后的角速度
 * AngleAcc_Offset — 加速度计角度偏移值（单位：度）
 *                   使用方式：在互补滤波中 AngleAcc_Raw + AngleAcc_Offset 得到校准后的角度
 */
int16_t GY_Offset = 0;
float AngleAcc_Offset = 0;

void MPU6050_Init(void)
{
    MPU6050_WriteData(MPU6050_PWR_MGMT_1, 0x01);//退出睡眠模式

    MPU6050_WriteData(MPU6050_PWR_MGMT_2, 0x00);

    MPU6050_WriteData(MPU6050_SMPLRT_DIV, 0x00);//数据输出最快

    MPU6050_WriteData(MPU6050_CONFIG, 0x00);

    MPU6050_WriteData(MPU6050_GYRO_CONFIG, 0x18);

    MPU6050_WriteData(MPU6050_ACCEL_CONFIG, 0x18);
}

void MPU6050_WriteData(uint8_t Address, uint8_t data)
{
    HAL_I2C_Mem_WriteData(MPU6050_Address, Address, &data, 1);
}

int8_t MPU6050_ReadData(uint8_t Address)
{
    uint8_t data = 0x00;

    HAL_I2C_Mem_ReadData(MPU6050_Address, Address, &data, 1);//地址HAL库自动完成加1操作

    return data;
}

void MPU6050_ReadDatas(uint8_t Address, uint8_t *DataArr, uint8_t Count)
{
    HAL_I2C_Mem_ReadData(MPU6050_Address, Address, DataArr, Count);
}


void MPU6050_GetData0(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t Data[14];

    MPU6050_ReadDatas(MPU6050_ACCEL_XOUT_H, Data, 14);

    *AccX = (Data[0] << 8) | Data[1];
    *AccY = (Data[2] << 8) | Data[3];
    *AccZ = (Data[4] << 8) | Data[5];

    *GyroX = (Data[8] << 8) | Data[9];
    *GyroY = (Data[10] << 8) | Data[11];
    *GyroZ = (Data[12] << 8) | Data[13];
}

void MPU6050_GetData(MPU6050_Data_t *MPU6050_Data)
{
    uint8_t DataArr[14];

    MPU6050_ReadDatas(MPU6050_ACCEL_XOUT_H, DataArr,14);

    MPU6050_Data->Accel_X = (DataArr[0] << 8) | DataArr[1];
    MPU6050_Data->Accel_Y = (DataArr[2] << 8) | DataArr[3];
    MPU6050_Data->Accel_Z = (DataArr[4] << 8) | DataArr[5];

    MPU6050_Data->Gyro_X = (DataArr[8] << 8) | DataArr[9];
    MPU6050_Data->Gyro_Y = (DataArr[10] << 8) | DataArr[11];
    MPU6050_Data->Gyro_Z = (DataArr[12] << 8) | DataArr[13];

}

/*
 * MPU6050_Calibrate  上电静止校准
 * 功能：采集200组静止数据，计算陀螺仪Y轴零偏和加速度计角度偏移
 * 原理：静止时陀螺仪理想输出应为0，加速度计计算的角度应为0，
 *       实际值不为0是因为芯片零偏和机械安装误差。
 *       取平均值后用"取反"作为偏移量，使用时加上即可抵消。
 * 注意：必须在 MPU6050_Init() 之后调用，校准时小车水平静止。
 *       不满足静止条件会导致偏移量不准，影响角度估计。
 */
void MPU6050_Calibrate(void)
{
    int32_t gy_sum = 0;             // 陀螺仪Y轴累加器
    double angle_sum = 0;           // 加速度计角度累加器
    int16_t ax, ay, az, gx, gy, gz; // 存放原始数据
    int count = 200;                // 采样次数，越多越准，但耗时更长

    for (int i = 0; i < count; i++)
    {
        /* 读取MPU6050原始6轴数据 */
        MPU6050_GetData0(&ax, &ay, &az, &gx, &gy, &gz);

        /* 累加陀螺仪Y轴原始值（静止时应该接近0） */
        gy_sum += gy;

        /* 累加加速度计计算出的角度（静止水平时应该接近0） */
        angle_sum += -atan2(ax, az) / 3.1415926535 * 180;

        /* 每5ms采集一次，留出数据更新间隔 */
        HAL_Delay(5);
    }

    /*
     * GY_Offset：取平均后取反，使用时 gy + GY_Offset ≈ 0
     * 例如：静止时gy平均值为31，则GY_Offset = -31，
     *       使用时 gy + (-31) 抵消零偏得到接近0的值
     */
    GY_Offset = -(int16_t)(gy_sum / count);

    /*
     * AngleAcc_Offset：取平均后取反，使用时 AngleAcc_Raw + AngleAcc_Offset ≈ 0
     * 例如：静止时角度平均值为2.49°，则AngleAcc_Offset = -2.49，
     *       使用时 AngleAcc_Raw + (-2.49) 抵消安装倾斜
     */
    AngleAcc_Offset = -(float)(angle_sum / count);
}
