//
// Created by Car on 2026/6/18.
//

#ifndef CLION_S_I2C_H
#define CLION_S_I2C_H

#include "main.h"

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(uint8_t data);
uint8_t I2C_ReadByte(void);
void I2C_SendAckBit(uint8_t AckBit);
uint8_t I2C_ReadAckBit(void);

#endif //CLION_S_I2C_H
