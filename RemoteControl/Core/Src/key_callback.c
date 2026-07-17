/*
 *嵌入式里这类事件回调统一命名习惯：
 *xxxHandler / xxxCallback 都代表回调函数
 */

#include "../Inc/key_callback.h"

uint8_t Key_Flag = 0;

void Key_RegisterAllHandlers(void)
{
    Key_SetPutDownHandler(PutDownHandler);
    Key_SetReleaseHandler(releaseHandler);
    Key_SetClickHandler(clickHandler);
    Key_SetDoubleClickHandler(doubleClickHandler);
    Key_SetLongPressHandler(longPressHandler);
    Key_SetRepeatHandler(repeatHandler);
}

void PutDownHandler(uint8_t keyId)
{
    switch (keyId)
    {
        case K1_ID:break;
        case K2_ID:break;
        case K3_ID:break;
        case K4_ID:break;
    }
}

void releaseHandler(uint8_t keyId)
{
    switch (keyId)
    {
        case K1_ID:break;
        case K2_ID:break;
        case K3_ID:break;
        case K4_ID:break;
    }
}
//上面两个函数不准与clickHandler一同使用！！！
void clickHandler(uint8_t keyId)
{
    switch (keyId)
    {
        case K1_ID:Key_Flag = K1_Flag;break;
        case K2_ID:Key_Flag = K2_Flag;break;
        case K3_ID:Key_Flag = K3_Flag;break;
        case K4_ID:Key_Flag = K4_Flag;break;
        case K5_ID:Key_Flag = K5_Flag;break;
        case K6_ID:Key_Flag = K6_Flag;break;
        case K7_ID:Key_Flag = K7_Flag;break;
        case K8_ID:Key_Flag = K8_Flag;break;
        case K9_ID:Key_Flag = K9_Flag;break;
        case K10_ID:Key_Flag = K10_Flag;break;
        case K11_ID:Key_Flag = K11_Flag;break;
        case K12_ID:Key_Flag = K12_Flag;break;
    }
}

void doubleClickHandler(uint8_t keyId)
{
    switch (keyId)
    {
        case K1_ID:break;
        case K2_ID:break;
        case K3_ID:break;
        case K4_ID:break;
    }
}

void longPressHandler(uint8_t keyId)
{
    switch (keyId)
    {
        case K1_ID:break;
        case K2_ID:break;
        case K3_ID:break;
        case K4_ID:break;
    }
}

void repeatHandler(uint8_t keyId)
{
    switch (keyId)
    {
        case K1_ID:break;
        case K2_ID:break;
        case K3_ID:break;
        case K4_ID:break;
    }
}
