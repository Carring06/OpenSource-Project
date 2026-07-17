/*****************************************************************************
 * 文件名 : MyKey.c
 * 功能   : 按键驱动 — 五状态机 + 单击/双击/长按/连发
 * 作者   : Car
 * 
 * ========================== 移植说明 ==========================
 * 
 * 【如何添加/删除按键】
 *   1. 在 MyKey.h 中修改 KEY_NUM
 *   2. 在本文件 keys[] 数组中增删对应的 {Port, Pin} 项
 * 
 * 【如何修改按键极性（高/低有效）】注意！！！本程序仅适用多按键为同一电平有效，若不同有效电平则应对Scan进行大范围修改，不可取！
 *   修改下面的 KEY_Trigger_ActiveLevel / KEY_Trigger_IdleLevel：
 *     低有效（上拉，按下=0）：Active = RESET, Idle = SET
 *     高有效（下拉，按下=1）：Active = SET,   Idle = RESET
 * 
 * 【如何调整时间阈值】
 *   下面 KEY_xxx_TICKS 的单位 = Key_Scan 调用周期（本驱动为 1ms）
 * 
 * 【如何使用回调】
 *   在 main.c 中调用 Key_SetXxxHandler(yourFunc) 注册，
 *   回调在 Key_Scan 内执行（即定时器中断），不要放阻塞操作。
 * =============================================================
 *****************************************************************************/

//发现单击时，有点小延时。松开触发也会，按下触发的不会，Key_GetPressed(K4_ID)这个函数也不会

#include "../Inc/MyKey.h"

/* 【移植点】按键有效/无效电平，换极性改这里 */
#define KEY_Trigger_ActiveLevel     GPIO_PIN_RESET   /* 按下时的电平 */
#define KEY_Trigger_IdleLevel       GPIO_PIN_SET     /* 松开时的电平 */

/* 【移植点】消抖时长（按 1ms 周期算） */
#define KEY_DEBOUNCE_TICKS      20
/* 【移植点】长按触发阈值（ms） */
#define KEY_LONG_PRESS_TICKS    1500
/* 【移植点】双击等待窗口（ms） */
#define KEY_DOUBLE_CLICK_TICKS  150
/* 【移植点】连发开始阈值（ms） */
#define KEY_REPEAT_START_TICKS  2000
/* 【移植点】连发触发间隔（ms） */
#define KEY_REPEAT_INTERVAL     100

/* 五状态枚举 */
typedef enum {
    KEY_IDLE            = 0,    /* 空闲，等按下 */
    KEY_PRESS_DEBOUNCE  = 1,    /* 按下消抖中 */
    KEY_PRESSED         = 2,    /* 确认按住 */
    KEY_RELEASE_DEBOUNCE= 3,    /* 松开消抖中 */
    KEY_WAIT_DOUBLE     = 4,    /* 等第二次按下（双击窗口） */
} KeyState_t;

/* 按键实例结构体 */
typedef struct {
    GPIO_TypeDef   *port;              /* GPIO 端口           */
    uint16_t        pin;               /* GPIO 引脚           */
    KeyState_t      state;             /* 当前状态 0~4        */
    uint8_t         debounceCnt;       /* 消抖计数 0→20 确认 */
    uint8_t         pressFlag;         /* 松手标志            */
    uint16_t        holdCnt;           /* 按住计时（长按/连发）*/
    uint16_t        waitDblCnt;        /* 双击窗口计时         */
    uint8_t         longPressTriggered;/* 长按已触发           */
    uint8_t         doubleClickPending;/* 双击等待中           */
    uint8_t         repeatTriggered;   /* 连发已开启           */
    uint16_t        repeatCnt;         /* 连发间隔计时         */
} Key_t;

/* 回调函数指针（NULL=不调用） */
static Key_PutDownHandler_t      PutDownHandler      = NULL; /* 按下 */
static Key_ReleaseHandler_t      releaseHandler      = NULL; /* 松手 */
static Key_ClickHandler_t        clickHandler        = NULL; /* 单击 */
static Key_DoubleClickHandler_t  doubleClickHandler  = NULL; /* 双击 */
static Key_LongPressHandler_t    longPressHandler    = NULL; /* 长按 */
static Key_RepeatHandler_t       repeatHandler       = NULL; /* 连发 */

/* 注册按下回调 */
void Key_SetPutDownHandler(Key_PutDownHandler_t handler)
{
    PutDownHandler = handler;
}

/* 注册松手回调 */
void Key_SetReleaseHandler(Key_ReleaseHandler_t handler)
{
    releaseHandler = handler;
}

/* 注册单击回调 */
void Key_SetClickHandler(Key_ClickHandler_t handler)
{
    clickHandler = handler;
}

/* 注册双击回调 */
void Key_SetDoubleClickHandler(Key_DoubleClickHandler_t handler)
{
    doubleClickHandler = handler;
}

/* 注册长按回调 */
void Key_SetLongPressHandler(Key_LongPressHandler_t handler)
{
    longPressHandler = handler;
}

/* 注册连发回调 */
void Key_SetRepeatHandler(Key_RepeatHandler_t handler)
{
    repeatHandler = handler;
}

/* 【移植点】按键硬件映射表：添加按键在这里加 {Port, Pin} */
static Key_t keys[KEY_NUM] = {
    {K1_GPIO_Port,  K1_Pin},
    {K2_GPIO_Port,  K2_Pin},
    {K3_GPIO_Port,  K3_Pin},
    {K4_GPIO_Port,  K4_Pin},
};

/* 按键扫描：由 1ms 定时器中断周期调用 */
void Key_Scan(void)
{
    for (uint8_t i = 0; i < KEY_NUM; i++)
    {
        GPIO_PinState level = HAL_GPIO_ReadPin(keys[i].port, keys[i].pin);

        switch (keys[i].state)
        {
            /* === IDLE：等待按下 === */
            case KEY_IDLE:
                if (level == KEY_Trigger_ActiveLevel)       /* 检测到有效电平（按下） */
                {
                    keys[i].state = KEY_PRESS_DEBOUNCE;     /* 进入按下消抖 */
                    keys[i].debounceCnt = 0;                /* 消抖计数器归零 */
                }
                break;

            /* === PRESS_DEBOUNCE：按下消抖，连续 20ms 有效才确认 === */
            case KEY_PRESS_DEBOUNCE:
                if (level == KEY_Trigger_ActiveLevel)       /* 仍按着，继续计数 */
                {
                    if (++keys[i].debounceCnt >= KEY_DEBOUNCE_TICKS)
                    {                                       /* 连续 20 次 → 确认按下 */
                        keys[i].state = KEY_PRESSED;
                        keys[i].debounceCnt = 0;
                        keys[i].holdCnt = 0;                /* 长按/连发计时归零 */
                        keys[i].longPressTriggered = 0;
                        if (PutDownHandler)
                        {
                            PutDownHandler(i);              /* 回调：按下 */
                        }
                    }
                }
                else if (level == KEY_Trigger_IdleLevel)    /* 中途松手 → 抖动，回 IDLE */
                {
                    keys[i].state = KEY_IDLE;
                }
                break;

            /* === PRESSED：已确认按住，处理长按和连发 === */
            case KEY_PRESSED:
                keys[i].holdCnt ++;                     /* 按住计时 +1ms */

                /* 长按检测：holdCnt >= 1500 触发一次 */
                if (!keys[i].longPressTriggered
                    && keys[i].holdCnt >= KEY_LONG_PRESS_TICKS)
                {
                    keys[i].longPressTriggered = 1;     /* 标记已触发，防重复 */
                    if (longPressHandler)
                    {
                        longPressHandler(i);            /* 回调：长按 */
                    }
                }

                /* 连发开启：长按已触发 + holdCnt >= 2000 开启 */
                if (keys[i].longPressTriggered
                    && !keys[i].repeatTriggered
                    && keys[i].holdCnt >= KEY_REPEAT_START_TICKS)
                {
                    keys[i].repeatTriggered = 1;
                    keys[i].repeatCnt = 0;
                    if (repeatHandler)
                    {
                        repeatHandler(i);               /* 回调：首次连发 */
                    }
                }

                /* 连发持续：每 100ms 触发一次 repeatHandler */
                if (keys[i].repeatTriggered)
                {
                    keys[i].repeatCnt++;
                    if (keys[i].repeatCnt >= KEY_REPEAT_INTERVAL)
                    {
                        keys[i].repeatCnt = 0;
                        if (repeatHandler)
                        {
                            repeatHandler(i);           /* 回调：每 100ms 连发 */
                        }
                    }
                }

                /* 松开检测 */
                if (level == KEY_Trigger_IdleLevel)
                {
                    keys[i].state = KEY_RELEASE_DEBOUNCE;
                    keys[i].debounceCnt = 0;
                }
                break;

            /* === RELEASE_DEBOUNCE：松开消抖，确认后按优先级产生事件 === */
            case KEY_RELEASE_DEBOUNCE:
                if (level == KEY_Trigger_IdleLevel)         /* 仍松着，继续计数 */
                {
                    if (++keys[i].debounceCnt >= KEY_DEBOUNCE_TICKS)
                    {                                       /* 连续 20 次 → 确认松开 */
                        keys[i].debounceCnt = 0;
                        keys[i].pressFlag = 1;

                        /* 优先级 1：连发中松手 → 回 IDLE */
                        if (keys[i].repeatTriggered)
                        {
                            keys[i].state = KEY_IDLE;
                            keys[i].repeatTriggered = 0;
                            keys[i].repeatCnt = 0;
                        }
                        /* 优先级 2：长按后松手 → 回 IDLE，不产生事件 */
                        else if (keys[i].longPressTriggered)
                        {
                            keys[i].state = KEY_IDLE;
                            keys[i].longPressTriggered = 0;
                            keys[i].doubleClickPending = 0;
                            /* 不调 releaseHandler，不置 pressFlag */
                        }
                        /* 优先级 3：双击第二下松手 */
                        else if (keys[i].doubleClickPending == 1)
                        {
                            keys[i].doubleClickPending = 0;
                            keys[i].state = KEY_IDLE;
                            if (doubleClickHandler)
                            {
                                doubleClickHandler(i);      /* 回调：双击 */
                            }
                        }
                        /* 优先级 4：首次松手 → 进 WAIT_DOUBLE 等双击 */
                        else
                        {
                            keys[i].state = KEY_WAIT_DOUBLE;/* 等 200ms 看有无第二下 */
                            keys[i].waitDblCnt = 0;
                            keys[i].doubleClickPending = 1; /* 标记等待中 */
                        }
                    }
                }
                break;


            /* === WAIT_DOUBLE：第一次松手后等第二次按下，超 200ms 触发单击 === */
            case KEY_WAIT_DOUBLE:
                keys[i].waitDblCnt++;               /* 等待计时 +1ms */
                if (level == KEY_Trigger_ActiveLevel)/* 检测到第二次按下 */
                {
                    keys[i].state = KEY_PRESS_DEBOUNCE; /* 进双击流程 */
                    keys[i].debounceCnt = 0;
                }                                   /* doubleClickPending 保持=1 */
                else if (keys[i].waitDblCnt >= KEY_DOUBLE_CLICK_TICKS)
                {                                   /* 200ms 超时 → 单击 */
                    keys[i].state = KEY_IDLE;
                    keys[i].doubleClickPending = 0;
                    if (releaseHandler)
                    {
                        releaseHandler(i);          /* 回调：松手 */
                    }
                    if (clickHandler)
                    {
                        clickHandler(i);            /* 回调：单击 */
                    }
                }
                break;
        }
    }
}



/* 查询：是否发生过一次松手（读取后自动清零） */
uint8_t Key_GetPressed(uint8_t keyId)
{
    if (keyId >= KEY_NUM) return 0;

    uint8_t ret = keys[keyId].pressFlag;
    keys[keyId].pressFlag = 0;
    return ret;
}

/* 查询：按键当前是否按住 */
uint8_t Key_IsDown(uint8_t keyId)
{
    if (keyId >= KEY_NUM) return 0;

    return (keys[keyId].state == KEY_PRESSED);
}
