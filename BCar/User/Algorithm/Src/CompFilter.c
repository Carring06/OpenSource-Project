#include "CompFilter.h"

/*--------------------------------------------------------------各角度值*/
float AngleAcc;
float AngleAcc_Raw;         // 原始加速度计角度（未滤波），用于调试
float AngleAcc_Filter;      // 经一阶低通滤波后的加速度计角度
float AngleGyro;
float Angle;
/*----------------------------------------------互补滤波的 "融合权重系数"*/
double Alpha = 0.028;

/*
 * Complementary_Filter  互补滤波函数
 * 功能：融合加速度计和陀螺仪数据，得到稳定的角度估计
 * 原理：
 *   1. 使用 GY_Offset 校准陀螺仪零偏，代替原来写死的 -= 31
 *   2. 使用 AngleAcc_Offset 校准加速度计安装倾斜，代替原来写死的 -= 2.49
 *   3. 对加速度计角度做一阶低通预滤波（α=0.8），滤除高频噪声
 *   4. 陀螺仪积分得到 AngleGyro
 *   5. 动态Alpha：角度偏差越大，越信任加速度计
 *   6. Angle = Alpha * AngleAcc_Filter + (1 - Alpha) * AngleGyro
 * 参数：dt — 中断执行间隔（单位：秒），通常为 0.008 或 0.010
 */
void Complementary_Filter(float dt, float DifSpeed)
{
    /* 计算原始加速度计角度，并用偏移量校准安装倾斜 */
    AngleAcc_Raw = -atan2(MPU6050_Data.Accel_X, MPU6050_Data.Accel_Z) / 3.14159 * 180;
    AngleAcc_Raw += AngleAcc_Offset;

    /* 一阶低通预滤波：滤除加速度计的高频噪声，使角度变化更平滑 */
    AngleAcc_Filter = 0.8 * AngleAcc_Raw + 0.2 * AngleAcc_Filter;

    /* 陀螺仪积分：使用 GY_Offset 校准后的角速度 */
    AngleGyro = Angle + (MPU6050_Data.Gyro_Y + GY_Offset) / 32768.0 * 2000 * dt;

    /* 动态Alpha：传感器差异越大越信任加速度计（抑制陀螺仪漂移） */
    Alpha = fabs(AngleAcc_Filter - AngleGyro) * 0.005 + 0.005;

    /* 转向时加速度计受惯性力干扰，降低对加速度计的信任 */
    if (fabs(DifSpeed) > 5.0f)
    {
        if (Alpha > 0.008f) Alpha = 0.008f;
    }

    if (Alpha > 0.02) Alpha = 0.02;
    if (Alpha < 0.005) Alpha = 0.005;

    /* 互补滤波融合 */
    Angle = Alpha * AngleAcc_Filter + (1 - Alpha) * AngleGyro;
}
