/*
 *嵌入式里这类事件回调统一命名习惯：
 *xxxHandler / xxxCallback 都代表回调函数
 */

#include "key_callback.h"

uint8_t Num = 0;

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
        case K1_ID:break;
        case K2_ID:break;
        case K3_ID:break;
        case K4_ID:K4SingleClickFlag = !K4SingleClickFlag;break;
    }
}

void doubleClickHandler(uint8_t keyId)
{
    switch (keyId)
    {
        case K1_ID:break;
        case K2_ID:break;
        case K3_ID:break;
        case K4_ID:K4DoubleClickFlag = 1;break;
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
