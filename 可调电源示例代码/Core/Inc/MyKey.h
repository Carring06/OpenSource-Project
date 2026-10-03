//
// Created by Car on 2026/6/27.
//

#ifndef CLION_MYKEY_H
#define CLION_MYKEY_H

#include <main.h>

#define KEY_NUM         4
#define K1_ID           0
#define K2_ID           1
#define K3_ID           2
#define K4_ID           3

#define K1_Flag           1
#define K2_Flag           2
#define K3_Flag           3
#define K4_Flag           4


void Key_Scan(void);
uint8_t Key_GetPressed(uint8_t keyId);
uint8_t Key_IsDown(uint8_t keyId);


/* 按下回调（每次按下触发，保留原接口） */
typedef void (*Key_PutDownHandler_t)(uint8_t keyId);
void Key_SetPutDownHandler(Key_PutDownHandler_t handler);

/* 松手回调（每次松手触发，保留原接口） */
typedef void (*Key_ReleaseHandler_t)(uint8_t keyId);
void Key_SetReleaseHandler(Key_ReleaseHandler_t handler);

/* 单击回调（双击窗口超时后触发） */
typedef void (*Key_ClickHandler_t)(uint8_t keyId);
void Key_SetClickHandler(Key_ClickHandler_t handler);

/* 双击回调（两次松手后触发） */
typedef void (*Key_DoubleClickHandler_t)(uint8_t keyId);
void Key_SetDoubleClickHandler(Key_DoubleClickHandler_t handler);

/* 长按回调（按住超过阈值时触发） */
typedef void (*Key_LongPressHandler_t)(uint8_t keyId);
void Key_SetLongPressHandler(Key_LongPressHandler_t handler);

/* 连发回调（连发模式下按间隔重复触发） */
typedef void (*Key_RepeatHandler_t)(uint8_t keyId);
void Key_SetRepeatHandler(Key_RepeatHandler_t handler);

/* 连发结束回调（连发中松手时触发） */
typedef void (*Key_RepeatEndHandler_t)(uint8_t keyId);
void Key_SetRepeatEndHandler(Key_RepeatEndHandler_t handler);


#endif //CLION_MYKEY_H
