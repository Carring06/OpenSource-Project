#include "../Inc/uart_callback.h"
char BlueSerial_RxPacket[100];
uint8_t BlueSerial_RxFlag;
uint8_t RxData;       // 存放接收到的单个字节
static uint8_t RxState = 0;  // 接收状态机，0空闲，1接收中
static uint8_t pRxPacket = 0;// 数据包指针，0空闲，1接收中

/**
 * @brief  UART接收中断回调函数，处理数据包
 * @note  状态机解析[]包裹的数据，全局变量无修改
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (RxState == 0)
        {
            if (RxData == '[' && BlueSerial_RxFlag == 0)
            {
                RxState = 1;
                pRxPacket = 0;
            }
        }
        else if (RxState == 1)
        {
            if (RxData == ']')
            {
                RxState = 0;
                BlueSerial_RxPacket[pRxPacket] = '\0';
                BlueSerial_RxFlag = 1;
            }
            else
            {
                BlueSerial_RxPacket[pRxPacket] = RxData;
                pRxPacket ++;
                // 防止数组越界截断
                if (pRxPacket >= 99) pRxPacket = 99;
            }
        }

        // 重新开启单次接收中断
        HAL_UART_Receive_IT(&huart2, &RxData, 1);
    }
}

/*以下是开启DMA后需要使用的回调函数，接受不定长数据再用吧，这里就不做修改了*/
// uint8_t BlueSerial_RxFlag = 0;
// uint8_t BLERxInitialData[150];
// uint8_t BLEBackupBuffer[150];  // 备份缓冲区，供主循环安全读取
//
// void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
//     if (huart->Instance == USART1) {
//         //串口1暂时不处理
//     }
//     if (huart->Instance == USART2) {
//         BLE_UART_CopyData(Size);  // 去帧头帧尾
//
//         // 立即拷贝到备份缓冲区，供主循环安全读取
//         memcpy(BLEBackupBuffer, BLEProcessed_Data, Size);
//
//         HAL_UARTEx_ReceiveToIdle_DMA(&huart2, BLERxInitialData, sizeof(BLERxInitialData));
//         __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
//
//         BlueSerial_RxFlag = 1;  // 标志位置1
//     }
// }
