/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "NRF24L01.h"
#include "MyKey.h"
#include "OLED.h"
#include "Motor.h"
#include "Encoder.h"
#include "MPU6050.h"
#include "BLE_UART.h"
#include "key_callback.h"
#include "tim_callback.h"
#include "PID.h"
#include "Store.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    STATE_MENU = 0,
    STATE_RUNNING,
    STATE_PARAM_VIEW,
    STATE_OFFSET_VIEW
} AppState_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t RunFlag = 0;
uint8_t K4DoubleClickFlag = 0;
uint8_t K4SingleClickFlag = 0;
AppState_t AppState = STATE_MENU;
uint16_t SpeedLevel = 5;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ClearKeyEvents(void);
void Action_K1(void);
void Action_K2(void);
void Action_K3(void);
void Action_K4(void);
void Display_Update(AppState_t state);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LED_ON(void)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void LED_OFF(void)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}
//深吃这两个函数代码！！！
void SaveParam(void)
{
  /*保存参数至FLASH*/
  Store_Data[1] = GY_Offset;
  Store_Data[2] = *((uint16_t *)&AngleAcc_Offset);
  Store_Data[3] = *((uint16_t *)&AngleAcc_Offset + 1);
  Store_Data[4] = SpeedLevel;
  Store_Save();
}

void LoadParam(void)
{
  /*从FLASH加载参数*/
  uint32_t Temp;
  GY_Offset = Store_Data[1];
  Temp = (Store_Data[3] << 16) | Store_Data[2];
  AngleAcc_Offset = *(float *)&Temp;
  SpeedLevel = Store_Data[4];
}

/* ===== 浮点数格式化工具函数 ===== */
#define FLOAT_BUF_COUNT  40
#define FLOAT_BUF_SIZE   16

/* Float_Printf showSign 参数宏定义 */
#define FLOAT_SIGN_OnlyNEGATIVE    0   /* 仅负数显示-，正数不显示符号 */
#define FLOAT_SIGN_ALWAYS          1   /* 始终显示 +/- */
#define FLOAT_SIGN_OnlyPOSITIVE    2   /* 仅正数显示+，负数不显示符号 */

static char Float_Buf[FLOAT_BUF_COUNT][FLOAT_BUF_SIZE];
static uint8_t Float_BufIdx = 0;


/**
 * @brief  浮点数格式化为字符串（替代 sprintf 的 %f，不依赖 _printf_float）
 * @param  value    浮点数值
 * @param  width    总显示宽度（含符号、整数部分、小数点、小数部分）
 * @param  decimals 小数位数（0~5）
 * @param  showSign FLOAT_SIGN_OnlyNEGATIVE=仅负数显示-，FLOAT_SIGN_ALWAYS=始终显示+/-，FLOAT_SIGN_OnlyPOSITIVE=仅正数显示+
 * @return 格式化后的字符串指针（旋转缓冲区）
 */
char* Float_Printf(float value, int width, int decimals, uint8_t showSign)
{
  char* buf = Float_Buf[Float_BufIdx];
  Float_BufIdx = (Float_BufIdx + 1) % FLOAT_BUF_COUNT;

  int pos = 0;
  char sign = 0;

  /* 1. 处理符号 */
  if (value < 0.0f)
  {
    if (showSign != 2) sign = '-';  /* showSign=2时不显示负号，其他都显示 */
    value = -value;
  }
  else if (showSign >= 1)
  {
    sign = '+';  /* showSign=1或2时显示正号 */
  }

  /* 2. 拆分整数部分和小数部分 */
  int intPart = (int)value;
  float frac = value - (float)intPart;

  /* 3. 计算小数部分（0~5位），不用pow，用乘法循环 */
  int fracInt = 0;
  if (decimals > 0)
  {
    float multiplier = 1.0f;
    for (int i = 0; i < decimals; i++) multiplier *= 10.0f;
    fracInt = (int)(frac * multiplier);  /* 直接截断，不四舍五入 */
  }

  /* 4. 计算整数部分的位数 */
  int intDigits = 0;
  int temp = intPart;
  if (temp == 0)
  {
    intDigits = 1;
  }
  else
  {
    while (temp > 0)
    {
      intDigits++;
      temp /= 10;
    }
  }

  /* 5. 计算总所需长度：符号 + 整数位 + 小数点 + 小数位 */
  int totalLen = (sign ? 1 : 0) + intDigits + (decimals > 0 ? 1 : 0) + decimals;

  /* 6. 计算需要补零的数量（符号占1位后，剩余宽度用于补零+整数+小数点+小数） */
  int padZero = 0;
  int contentLen = intDigits + (decimals > 0 ? 1 : 0) + decimals;
  if (width > (sign ? 1 : 0) + contentLen)
  {
    padZero = width - (sign ? 1 : 0) - contentLen;
  }

  /* 7. 从左到右构建字符串：符号 -> 补零 -> 整数部分 -> 小数点 -> 小数部分 */

  /* 符号（放在最前面） */
  if (sign)
  {
    buf[pos++] = sign;
  }

  /* 补零 */
  for (int i = 0; i < padZero; i++)
  {
    buf[pos++] = '0';
  }

  /* 整数部分（递归写入） */
  if (intPart == 0)
  {
    buf[pos++] = '0';
  }
  else
  {
    /* 先写高位到临时数组，再反转写入buf */
    char intBuf[12];
    int intLen = 0;
    temp = intPart;
    while (temp > 0)
    {
      intBuf[intLen++] = '0' + (temp % 10);
      temp /= 10;
    }
    /* 反转 */
    for (int i = intLen - 1; i >= 0; i--)
    {
      buf[pos++] = intBuf[i];
    }
  }

  /* 小数点 */
  if (decimals > 0)
  {
    buf[pos++] = '.';

    /* 小数部分（补零 + 数字） */
    int fracDigits = 1;
    int divisor = 1;
    for (int i = 1; i < decimals; i++)
    {
      divisor *= 10;
      fracDigits *= 10;
    }
    fracDigits *= 10;

    for (int i = 0; i < decimals; i++)
    {
      int digit = (fracInt / divisor) % 10;
      buf[pos++] = '0' + digit;
      divisor /= 10;
    }
  }

  buf[pos] = '\0';
  return buf;
}

/*平衡车动画*/
void BCar_Start_Animation()
{
	OLED_ShowString(44,6,"STM32",OLED_8X16);
	OLED_ShowChinese_12X12(46,26,"平衡车");
  OLED_ShowString(44,48,"HAL_Car",OLED_6X8);
	OLED_Update();
	HAL_Delay(1500);
	while(1)
	{
		uint16_t i;
		OLED_ShowChinese_12X12(40,26,"车度空间");
		OLED_UpdateArea(40,26,i,12);
		if(i<12*2)
		{
			HAL_Delay(16);i++;
		}
		if (i<12*4 && i>=12*2)
		{
			HAL_Delay(3);i++;
		}
		if (i>=12*4)
		{
			HAL_Delay(1500);break;
		}
	}
  OLED_ClearArea(44,48,42,8);
  OLED_UpdateArea(44,48,42,8);
  while(1)
  {
    uint16_t i;
    OLED_ShowChinese_12X12(40,46,"除了");
    OLED_ShowString(64,48,"OLED",OLED_6X8);
    OLED_UpdateArea(40,46,i,16);
    if(i<16*2)
    {
      HAL_Delay(16);i++;
    }
    if (i<16*6 && i>=16*2)
    {
      HAL_Delay(3);i++;
    }
    if (i>=16*6)
    {
      HAL_Delay(1500);break;
    }
  }
  while(1)
  {
    uint16_t i;
    OLED_ShowChinese_12X12(30,46,"其余从零撰写");
    OLED_UpdateArea(30,46,i,12);
    if(i<12*3)
    {
      HAL_Delay(16);i++;
    }
    if (i<12*6 && i>=12*3)
    {
      HAL_Delay(3);i++;
    }
    if (i>=12*6)
    {
      HAL_Delay(1500);break;
    }
  }
	OLED_Clear();
  OLED_ShowChinese_12X12(30+16,0,"期末月");
  OLED_ShowChinese_12X12(10+16,13,"懒懒散散一个月");
  OLED_ShowChinese_12X12(24+16,26,"历经千辛");
  OLED_ShowChinese_12X12(6+16,40,"改了许许多多");
  OLED_ShowString(96,43,"BUG",OLED_6X8);
  OLED_ShowChinese_12X12(26+16,52,"终于完成！");
	OLED_Update();
	HAL_Delay(4000);
	OLED_Clear();
	OLED_ShowChinese_12X12(40,26,"按下一键");
  OLED_ShowString(0, 40, "                K1>", OLED_6X8);
	OLED_Update();
}

/* ===== 按键事件清除，避免“误触”和“重复触发” ===== */
void ClearKeyEvents(void)
{
    Key_GetPressed(K1_ID);
    Key_GetPressed(K2_ID);
    Key_GetPressed(K3_ID);
    K4SingleClickFlag = 0;
    K4DoubleClickFlag = 0;
}

/* ===== K1：启动/停止 ===== */
void Action_K1(void)
{
    if (AppState == STATE_MENU)
    {
        PID_Init(&AnglePID);
        PID_Init(&SpeedPID);
        PID_Init(&TurnPID);
        Angle = AngleAcc_Filter;
        LED_ON();
        RunFlag = 1;
        AppState = STATE_RUNNING;
    }
    else if (AppState == STATE_RUNNING)
    {
        LED_OFF();
        RunFlag = 0;
        AppState = STATE_MENU;
        ClearKeyEvents();
    }
}

/* ===== K2：校准（仅菜单状态下可触发） ===== */
void Action_K2(void)
{
    if (AppState != STATE_MENU) return;

    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "Calibrating MPU6050...");
    OLED_Update();

    MPU6050_Calibrate();

    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "Calibration Finished");
    OLED_Update();
    HAL_Delay(1000);

    AppState = STATE_MENU;
    ClearKeyEvents();
}

/* ===== K3：参数显示切换 ===== */
void Action_K3(void)
{
    if (AppState == STATE_MENU)
    {
        RunFlag = 2;
        AppState = STATE_PARAM_VIEW;
    }
    else if (AppState == STATE_PARAM_VIEW)
    {
        RunFlag = 0;
        AppState = STATE_MENU;
        ClearKeyEvents();
    }
}

/* ===== K4：偏移量显示 / 保存参数 ===== */
void Action_K4(void)
{
    if (K4DoubleClickFlag == 1)
    {
        OLED_Clear();
        OLED_Printf(0, 0, OLED_6X8, "Saving Param...");
        OLED_Update();
        HAL_Delay(1000);

        SaveParam();

        OLED_Clear();
        OLED_Printf(0, 0, OLED_6X8, "Save Complete");
        OLED_Update();
        HAL_Delay(1000);

        K4DoubleClickFlag = 0;
        K4SingleClickFlag = 0;
        AppState = STATE_MENU;
        ClearKeyEvents();
    }
    else if (K4SingleClickFlag == 1)
    {
        if (AppState == STATE_MENU)
        {
            AppState = STATE_OFFSET_VIEW;
            OLED_Clear();
            OLED_Printf(0, 0, OLED_6X8, "AngleAccOffset:%s", Float_Printf(AngleAcc_Offset, 5, 2, FLOAT_SIGN_ALWAYS));
            OLED_Printf(0, 16, OLED_6X8, "   GY_Offset   :%05d", GY_Offset);
            OLED_Printf(0, 32, OLED_6X8, "   SpeedLevel  :%05d", SpeedLevel);
            OLED_Printf(0, 56, OLED_6X8, "<DoubleK4:Save Param>");
            OLED_Update();
            K4SingleClickFlag = 0;
        }
        else if (AppState == STATE_OFFSET_VIEW)
        {
            K4SingleClickFlag = 0;
            AppState = STATE_MENU;
            ClearKeyEvents();
        }
    }
}
/* ===== K8：切换为减速模式 ===== */
void Action_K8(void)
{
  SaveParam();
}
/* ===== K9：切换为减速模式 ===== */
void Action_K9(void)
{
  if (SpeedLevel <= 1)return;
  SpeedLevel--;
}
/* ===== K10：切换为减速模式 ===== */
void Action_K10(void)
{
  if (SpeedLevel >= 5)return;
  SpeedLevel ++;
}
/* ===== OLED显示更新 ===== */
void Display_Update(AppState_t state)
{
    if (state == STATE_MENU && K4SingleClickFlag == 0)
    {
        LED_OFF();
        OLED_Clear();
        OLED_Printf(0, 0,  OLED_8X16, "K1:Start Program");
        OLED_Printf(0, 16, OLED_8X16, "K2:Calibrate");
        OLED_Printf(0, 32, OLED_8X16, "K3:Param Display");
        OLED_Printf(0, 48, OLED_8X16, "K4:Param Save");
        OLED_Update();
    }
    else if ((state == STATE_RUNNING || state == STATE_PARAM_VIEW) && K4SingleClickFlag == 0)
    {
        int16_t Coord_Offset = 0;
        OLED_Clear();
        OLED_Printf(0, 0, OLED_6X8, "  Angle");
        OLED_Printf(0, 8, OLED_6X8, "P:%s", Float_Printf(AnglePID.Kp, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
        OLED_Printf(0, 16, OLED_6X8, "I:%s", Float_Printf(AnglePID.Ki, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
        OLED_Printf(0, 24, OLED_6X8, "D:%s", Float_Printf(AnglePID.Kd, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));

        OLED_Printf(50, 0, OLED_6X8, "Speed");
        OLED_Printf(50, 8, OLED_6X8, "%s", Float_Printf(SpeedPID.Kp, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
        OLED_Printf(50, 16, OLED_6X8, "%s", Float_Printf(SpeedPID.Ki, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
        OLED_Printf(50, 24, OLED_6X8, "%s", Float_Printf(SpeedPID.Kd, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));

        OLED_Printf(88, 0, OLED_6X8, "Turn");
        OLED_Printf(88, 8, OLED_6X8, "%s", Float_Printf(TurnPID.Kp, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
        OLED_Printf(88, 16, OLED_6X8, "%s", Float_Printf(TurnPID.Ki, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
        OLED_Printf(88, 24, OLED_6X8, "%s", Float_Printf(TurnPID.Kd, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));


        if (state == STATE_RUNNING)
        {
            OLED_Printf(50, 48, OLED_6X8, "%s", Float_Printf(SpeedPID.Out, 5, 0, FLOAT_SIGN_ALWAYS));
            OLED_Printf(88, 48, OLED_6X8, "%s", Float_Printf(TurnPID.Out, 5, 0, FLOAT_SIGN_ALWAYS));
            OLED_Printf(0, 48, OLED_6X8, "O:%s", Float_Printf(AnglePID.Out, 5, 0, FLOAT_SIGN_ALWAYS));

            OLED_Printf(0, 32, OLED_6X8, "T:%s", Float_Printf(AnglePID.Target, 5, 1, FLOAT_SIGN_ALWAYS));
            OLED_Printf(50, 32, OLED_6X8, "%s", Float_Printf(SpeedPID.Target, 5, 1, FLOAT_SIGN_ALWAYS));
            OLED_Printf(88, 32, OLED_6X8, "%s", Float_Printf(TurnPID.Target, 5, 1, FLOAT_SIGN_ALWAYS));
        }
        else
        {
            Coord_Offset = -8;
            OLED_Printf(0, 56, OLED_6X8, "   <Param Display>");
            OLED_Printf(0, 56 -8 + Coord_Offset, OLED_6X8, "OOSP%s", Float_Printf(AnglePID.OutOffset_Positive, 5, 0, FLOAT_SIGN_ALWAYS));
            OLED_Printf(58, 56 -8 + Coord_Offset, OLED_6X8, " OOSN%s", Float_Printf(AnglePID.OutOffset_Negative, 5, 0, FLOAT_SIGN_ALWAYS));
        }
        OLED_Printf(0, 40 + Coord_Offset, OLED_6X8, "A:%s", Float_Printf(Angle, 5, 1, FLOAT_SIGN_ALWAYS));
        OLED_Printf(50, 40 + Coord_Offset, OLED_6X8, "%s", Float_Printf(AveSpeed, 5, 1, FLOAT_SIGN_ALWAYS));
        OLED_Printf(88, 40 + Coord_Offset, OLED_6X8, "%s", Float_Printf(DifSpeed, 5, 1, FLOAT_SIGN_ALWAYS));
        OLED_Printf(0,56 + Coord_Offset,OLED_6X8,"    SpeedLevel:%d", SpeedLevel);
        OLED_Update();

        char msg[250];
        sprintf(msg,"[plot,%s,%s]", Float_Printf(SpeedPID.Actual, 5, 2, FLOAT_SIGN_ALWAYS), Float_Printf(SpeedPID.Target, 5, 2, FLOAT_SIGN_ALWAYS));
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  NRF24L01_Init();
  HAL_TIM_Base_Start_IT(&htim1);
  //开启串口接收
  BLE_UART_Init();
  Motor_Init();
  Encoder_Init();
  MPU6050_Init();
  //注册按键回调函数
  Key_RegisterAllHandlers();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  int8_t LH,LV,RH,RV;
  uint8_t Remote_KeyControl = 0;
  uint8_t Remote_KeyLast = 0;
  uint8_t displayKey = 0;
  LED_OFF();
  LoadParam();
  BCar_Start_Animation();
  while (Key_GetPressed(K1_ID) == 0);
  OLED_Clear();
  if (Store_Init())
  {
    /*未存储参数，将程序中的默认参数保存到FLASH中*/
    SaveParam();

    /*提示已重置参数*/
    OLED_Clear();
    OLED_ShowString(0, 0,  "     [提示]     ", OLED_8X16);
    OLED_ShowString(0, 16, "   已重置参数   ", OLED_8X16);
    OLED_ShowString(0, 32, " 请注意执行校准 ", OLED_8X16);
    OLED_ShowString(0, 48, "            K1>", OLED_8X16);
    OLED_Update();

    /*按K4键继续*/
    while (Key_GetPressed(K1_ID) == 0);
  }
  /*上电后加载FLASH中保存的参数*/
  LoadParam();
  OLED_Clear();
  HAL_Delay(10);
  while (1)
  {
    BlueSerial_Control();

    if (NRF24L01_Receive())
    {
      LH = NRF24L01_RxBuffer[0];
      LV = NRF24L01_RxBuffer[1];
      RH = NRF24L01_RxBuffer[2];
      RV = NRF24L01_RxBuffer[3];
      Remote_KeyControl = NRF24L01_RxBuffer[4];

      SpeedPID.Target = LV / 100.0 * SpeedLevel;
      TurnPID.Target = -(RH / 100.0 * SpeedLevel);

      if (Remote_KeyControl != 0 && Remote_KeyLast == 0)
      {
        switch (Remote_KeyControl)
        {
          case K1_Flag: Action_K1(); break;
          case K2_Flag: Action_K2(); break;
          case K3_Flag: Action_K3(); break;
          case K4_Flag: K4SingleClickFlag = 1; Action_K4(); break;
          case K8_Flag: Action_K8(); break;
          case K9_Flag: Action_K9(); break;
          case K10_Flag: Action_K10(); break;
        }
        NRF24L01_TxBuffer[5] = SpeedLevel;
        NRF24L01_Send();
      }
      Remote_KeyLast = Remote_KeyControl;

      /* 反向通道：根据当前状态计算活动按键，回传给遥控器 */
      switch (AppState)
      {
        case STATE_MENU:      displayKey = 0; break;
        case STATE_RUNNING:   displayKey = K1_Flag; break;
        case STATE_PARAM_VIEW: displayKey = K3_Flag; break;
        case STATE_OFFSET_VIEW: displayKey = K4_Flag; break;
        default: displayKey = 0; break;
      }
      NRF24L01_TxBuffer[4] = displayKey;
      NRF24L01_Send();
    }

    switch (AppState)
    {
      case STATE_MENU:
        if (Key_GetPressed(K1_ID)) Action_K1();
        if (Key_GetPressed(K2_ID)) Action_K2();
        if (Key_GetPressed(K3_ID)) Action_K3();
        if (K4SingleClickFlag || K4DoubleClickFlag) Action_K4();
        break;
      case STATE_RUNNING:
        if (Key_GetPressed(K1_ID)) Action_K1();
        if (K4SingleClickFlag || K4DoubleClickFlag) Action_K4();
        break;
      case STATE_PARAM_VIEW:
        if (Key_GetPressed(K3_ID)) Action_K3();
        if (K4SingleClickFlag || K4DoubleClickFlag) Action_K4();
        break;
      case STATE_OFFSET_VIEW:
        if (K4SingleClickFlag || K4DoubleClickFlag) Action_K4();
        break;
      default:
        break;
    }

    if (RunFlag != 1)
    {
      LED_OFF();
    }

    static uint8_t RunFlag_Last = 0;
    if (RunFlag == 0 && RunFlag_Last == 1 && AppState != STATE_MENU)
    {
      AppState = STATE_MENU;
      ClearKeyEvents();
    }
    RunFlag_Last = RunFlag;

    Display_Update(AppState);


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
