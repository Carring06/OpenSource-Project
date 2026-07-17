//
// Created by Car on 2026/6/27.
//

#ifndef CLION_UART_CALLBACK_H
#define CLION_UART_CALLBACK_H

#include "main.h"
#include "usart.h"
#include "BLE_UART.h"

extern uint8_t BLERxInitialData[150];
extern uint8_t BLEBackupBuffer[150];
extern char BlueSerial_RxPacket[100];
extern uint8_t RxData;
extern uint8_t BlueSerial_RxFlag;

#endif //CLION_UART_CALLBACK_H
