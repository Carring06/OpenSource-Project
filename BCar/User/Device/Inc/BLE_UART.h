//
// Created by Car on 2026/6/27.
//

#ifndef CLION_BLE_UART_H
#define CLION_BLE_UART_H

#include "uart_callback.h"
#include <string.h>
#include <stdlib.h>
#include "tim_callback.h"

extern uint8_t BLEProcessed_Data[150];

void BLE_UART_Init(void) ;
void BLE_UART_CopyData(uint8_t Size);
void BlueSerial_Control(void);

#endif //CLION_BLE_UART_H
