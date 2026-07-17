//
// Created by Car on 2026/6/27.
//

#include "../Inc/BLE_UART.h"

uint8_t BLEProcessed_Data[150];

void BLE_UART_Init(void)
{
    // 开启USART2接收中断
    HAL_UART_Receive_IT(&huart2, &RxData, 1);
}

void BlueSerial_Control(void)
{
    if (BlueSerial_RxFlag == 1)
    {
        char *Tag = strtok(BlueSerial_RxPacket, ",");
        if (strcmp(Tag, "key") == 0)
        {
            // char *Name = strtok(NULL, ",");
            // char *Action = strtok(NULL, ",");

        }
        else if (strcmp(Tag, "slider") == 0)
        {
            char *Name = strtok(NULL, ",");
            char *Value = strtok(NULL, ",");

            if (strcmp(Name, "AngleKp") == 0)
            {
                AnglePID.Kp = atof(Value);
            }
            else if (strcmp(Name, "AngleKi") == 0)
            {
                AnglePID.Ki = atof(Value);
            }
            else if (strcmp(Name, "AngleKd") == 0)
            {
                AnglePID.Kd = atof(Value);
            }
            else if (strcmp(Name, "SpeedKp") == 0)
            {
                SpeedPID.Kp = atof(Value);
            }
            else if (strcmp(Name, "SpeedKi") == 0)
            {
                SpeedPID.Ki = atof(Value);
            }
            else if (strcmp(Name, "SpeedKd") == 0)
            {
                SpeedPID.Kd = atof(Value);
            }
            else if (strcmp(Name, "TurnKp") == 0)
            {
                TurnPID.Kp = atof(Value);
            }
            else if (strcmp(Name, "TurnKi") == 0)
            {
                TurnPID.Ki = atof(Value);
            }
            else if (strcmp(Name, "TurnKd") == 0)
            {
                TurnPID.Kd = atof(Value);
            }
            else if (strcmp(Name, "POutOffset") == 0)
            {
                AnglePID.OutOffset_Positive = atof(Value);
            }
            else if (strcmp(Name, "NOutOffset") == 0)
            {
                AnglePID.OutOffset_Negative = atof(Value);
            }
        }
        else if (strcmp(Tag, "joystick") == 0)
        {
            int8_t LH = atoi(strtok(NULL, ","));
            int8_t LV = atoi(strtok(NULL, ","));
            int8_t RH = atoi(strtok(NULL, ","));
            int8_t RV = atoi(strtok(NULL, ","));

            SpeedPID.Target = LV / 25;
            TurnPID.Target = -(RH / 25);
        }

        BlueSerial_RxFlag = 0;
    }
}

