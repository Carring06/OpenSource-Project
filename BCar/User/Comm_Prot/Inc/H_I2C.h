//
// Created by Car on 2026/6/18.
//

#ifndef CLION_S_I2C_H
#define CLION_S_I2C_H

#include "main.h"

void HAL_I2C_Mem_WriteData(uint8_t DevAddr, uint8_t MemAddr, uint8_t *pData, uint16_t Size);
void HAL_I2C_Mem_ReadData(uint8_t DevAddr, uint8_t MemAddr, uint8_t *pData, uint16_t Size);

#endif //CLION_S_I2C_H
