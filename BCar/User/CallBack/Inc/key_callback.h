#ifndef CLION_KEY_CALLBACK_H
#define CLION_KEY_CALLBACK_H

#include "MyKey.h"

//外部变量声明
extern uint8_t Num;
extern uint8_t K4SingleClickFlag;
extern uint8_t K4DoubleClickFlag;

void Key_RegisterAllHandlers(void);
void PutDownHandler(uint8_t keyId);
void releaseHandler(uint8_t keyId);
//上面两个函数不准与clickHandler一同使用！！！
void clickHandler(uint8_t keyId);
void doubleClickHandler(uint8_t keyId);
void longPressHandler(uint8_t keyId);
void repeatHandler(uint8_t keyId);

#endif //CLION_KEY_CALLBACK_H
