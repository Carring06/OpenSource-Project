#include "H_I2C.h"
#include "i2c.h"

void HAL_I2C_Mem_WriteData(uint8_t DevAddr, uint8_t MemAddr, uint8_t *pData, uint16_t Size)
{
    HAL_I2C_Mem_Write(&hi2c2, DevAddr, MemAddr, I2C_MEMADD_SIZE_8BIT, pData, Size, HAL_MAX_DELAY);
}

void HAL_I2C_Mem_ReadData(uint8_t DevAddr, uint8_t MemAddr, uint8_t *pData, uint16_t Size)
{
    HAL_I2C_Mem_Read(&hi2c2, DevAddr, MemAddr, I2C_MEMADD_SIZE_8BIT, pData, Size, HAL_MAX_DELAY);
}


