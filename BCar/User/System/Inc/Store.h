#ifndef __STORE_H
#define __STORE_H

#include "main.h"          // Device header
#include "MyFLASH.h"

extern uint16_t Store_Data[];

uint8_t Store_Init(void);
void Store_Save(void);
void Store_Clear(void);

#endif
