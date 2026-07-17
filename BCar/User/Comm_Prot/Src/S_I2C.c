#include "S_I2C.h"

/************************************************************************************/
//使用软件I2C时再打开注释
// void SI2C_WriteSCL(GPIO_PinState PinState)
// {
//     HAL_GPIO_WritePin(MPU6050_SCL_GPIO_Port,MPU6050_SCL_Pin , PinState);
// }
//
// void SI2C_WriteSDA(GPIO_PinState PinState)
// {
//     HAL_GPIO_WritePin(MPU6050_SDA_GPIO_Port,MPU6050_SDA_Pin , PinState);
// }
//
// uint8_t SI2C_ReadSDA(void)
// {
//     return HAL_GPIO_ReadPin(MPU6050_SDA_GPIO_Port,MPU6050_SDA_Pin);
// }
/************************************************************************************/

void I2C_Init(void)
{
    SI2C_WriteSCL(1);
    SI2C_WriteSDA(1);
}

void I2C_Start(void)
{
    //防止两线异常处于低电平
    SI2C_WriteSDA(1);//先拉高SDA,可以兼容起始条件和重复起始条件，防止因为先拉高SCL后拉高SDA导致产生终止条件
    SI2C_WriteSCL(1);
    //以上为个人添加，如有不妥可以删除
    SI2C_WriteSDA(0);
    SI2C_WriteSCL(0);
}

void I2C_Stop(void)
{
    //防止两线异常
    SI2C_WriteSDA(0);//为确保SDA能产生上升沿
    SI2C_WriteSCL(0);//江协没有这个
    //以上为个人添加，如有不妥可以删除
    SI2C_WriteSCL(1);
    SI2C_WriteSDA(1);
}

void I2C_SendByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        SI2C_WriteSDA(data & (0x80 >> i));
        SI2C_WriteSCL(1);//驱动时钟走个脉冲
        SI2C_WriteSCL(0);
    }
}

uint8_t I2C_ReadByte(void)
{
    uint8_t data = 0x00;

    SI2C_WriteSDA(1);//主机释放总线控制权

    for (uint8_t i = 0; i < 8; i++)
    {
        SI2C_WriteSCL(1);
        data |= SI2C_ReadSDA() ? (0X80 >> i) : (0X00);//是否需要延时，待后续实测
        SI2C_WriteSCL(0);
    }

    return data;
}

void I2C_SendAckBit(uint8_t AckBit)
{
    SI2C_WriteSDA(AckBit);

    SI2C_WriteSCL(1);

    SI2C_WriteSCL(0);
}

uint8_t I2C_ReadAckBit(void)
{
    uint8_t AckBit = 0x00;

    SI2C_WriteSDA(1);//主机释放总线控制权

    SI2C_WriteSCL(1);

    AckBit = SI2C_ReadSDA();//是否需要延时，待后续实测

    SI2C_WriteSCL(0);

    return AckBit;
}
