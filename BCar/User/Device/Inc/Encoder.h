//
// Created by Car on 2026/6/16.
//

#ifndef CLION_ENCODER_H
#define CLION_ENCODER_H

#include "main.h"
#include "tim.h"

#ifndef __ENCODER_H
#define __ENCODER_H

void Encoder_Init(void);
int16_t Encoder_GetML(void);
int16_t Encoder_GetMR(void);
int16_t Encoder_GetMLPos(void);
int16_t Encoder_GetMRPos(void);
float Encoder_GetMLAngle(void);
float Encoder_GetMRAngle(void);

#endif


#endif //CLION_ENCODER_H
