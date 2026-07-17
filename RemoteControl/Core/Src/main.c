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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "NRF24L01.h"
#include "MyKey.h"
#include "OLED.h"
#include "../Inc/key_callback.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t Count = 0;
uint16_t count= 0;
uint8_t Count0 = 0;

uint8_t SendFlag = 0;
uint8_t CheckFlag = 0;

uint8_t SpeedLevel = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1)
  {
    Key_Scan();
    Count ++;
    count ++;
    if (count == 100)
    {
      count = 0;
      SendFlag = 1;
    }
    if (Count == 1000)
    {
      Count = 0;
      CheckFlag = 1;
      Count0 ++;
    }
  }
}

/**
 * @brief 将无符号整数转换为指定长度的二进制字符串
 *
 * 该函数将传入的无符号整数 n 转换为二进制表示，并存储在字符串 str 中。
 * 输出的二进制字符串长度固定为 bits 位，不足 bits 位时在高位补零。
 *
 * @param n     要转换的无符号整数
 * @param bits  输出的二进制字符串位数（例如：8、16、32）
 * @param str   输出缓冲区，调用者需保证缓冲区大小 >= bits + 1 字节
 *
 * @note  str 字符串以 '\0' 结尾，可直接用于 printf 等函数
 * @warning 调用者必须确保 bits 大于 0，且 str 缓冲区足够大
 *
 * @example
 *   char buf[9];  // 8位二进制 + 结束符
 *   int_to_bin_str(5, 8, buf);  // buf = "00000101"
 */
void int_to_bin_str(unsigned int n, int bits, char *str)
{
  str[bits] = '\0';               // 在字符串末尾设置结束符

  for (int i = bits - 1; i >= 0; i--) {  // 从最低位到最高位填充
    str[i] = (n & 1) + '0';     // 取出最低位（0或1），转换为ASCII字符
    n >>= 1;                    // 右移一位，准备处理下一位
  }
}

// 可以根据手感调整这两个参数
#define DEADZONE    5    // 死区：绝对值小于这个数，就强制归0
#define ENDZONE     95    // 靠近端点的阈值：绝对值大于等于这个数，就强制±100

// 输入范围：-100 ~ +100
// 输出范围：-100 / 0 / +100（或者保持中间值不变）
int8_t Joystick_Fix(int16_t value)
{
  if (value > ENDZONE) {
    return 100;
  } else if (value < -ENDZONE) {
    return -100;
  } else if (value > -DEADZONE && value < DEADZONE) {
    return 0;
  } else {
    // 中间区域保持原值不变，也可以做线性缩放
    return value;
  }
}

/*平衡车动画*/
void RC_Start_Animation()
{
  OLED_ShowString(44,15,"STM32",OLED_8X16);
  OLED_ShowChinese_12X12(46,35,"遥控器");
  OLED_Update();
  HAL_Delay(1000);
  while(1)
  {
    uint16_t i;
    OLED_ShowChinese_12X12(40,35,"车度空间");
    OLED_UpdateArea(40,35,i,12);
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
  OLED_Clear();
  OLED_ShowString(0, 12,  "  按键8保存速度 ", OLED_8X16);
  OLED_ShowString(0, 28, "   按键9 减速   ", OLED_8X16);
  OLED_ShowString(0, 44, "   按键10加速 ", OLED_8X16);
  OLED_Update();
  HAL_Delay(3000);
  OLED_Clear();
  while(1)
  {
    uint16_t i;
    OLED_ShowChinese_16X16(0,20,"功能按键与平衡车");
    OLED_ShowChinese_16X16(35,36,"一一映射");
    OLED_UpdateArea(0,0,i,60);
    if(i<16*4)
    {
      HAL_Delay(16);i++;
    }
    if (i<16*8 && i>=12*4)
    {
      HAL_Delay(3);i++;
    }
    if (i>=16*8)
    {
      HAL_Delay(1500);break;
    }
  }
  OLED_Clear();
  OLED_ShowChinese_12X12(40,26,"按下一键");
  OLED_ShowString(0, 40, "                K5>", OLED_6X8);
  OLED_Update();
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  NRF24L01_Init();
  HAL_TIM_Base_Start_IT(&htim1);
  Key_RegisterAllHandlers();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint16_t AD_Data[4];
  int8_t LH,LV,RH,RV;
  uint8_t TX_CallbackData = 0;
  uint8_t PLOS_CNT= 0,ARC_CNT = 0;
  uint8_t BCar_KeyControl = 0;   // 小车回传的活动按键编号
  uint8_t displayKey = 0;        // 当前要OLED显示的按键编号
  uint8_t displayKey_Last = 0;   // 上次显示的按键编号，用于变化检测去重刷新
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_Delay(10);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)AD_Data, 4);

  RC_Start_Animation();
  while (Key_GetPressed(K5_ID) == 0);
  OLED_Clear();
  HAL_Delay(10);
  while (1)
  {
    /* 每次循环顶部先读小车回复，避免被下面 NRF24L01_Send() 清除 RX_DR 标志 */
    if (NRF24L01_Receive())
    {
        BCar_KeyControl = NRF24L01_RxBuffer[4];
        SpeedLevel = NRF24L01_RxBuffer[5];
    }

    LH = (AD_Data[0] * 200 / 4095) - 100;
    LV = (AD_Data[1] * 200 / 4095) - 100;
    RH = (AD_Data[2] * 200 / 4095) - 100;
    RV = (AD_Data[3] * 200 / 4095) - 100;


    LH = Joystick_Fix(LH);
    LV = Joystick_Fix(LV);
    RH = Joystick_Fix(RH);
    RV = Joystick_Fix(RV);

    NRF24L01_TxBuffer[0] = LH;
    NRF24L01_TxBuffer[1] = LV;
    NRF24L01_TxBuffer[2] = RH;
    NRF24L01_TxBuffer[3] = RV;
    NRF24L01_TxBuffer[4] = Key_Flag;

    if (CheckFlag == 1)
    {
      CheckFlag = 0;
      TX_CallbackData = NRF24L01_ReadByte(NRF24L01_OBSERVE_TX);
      PLOS_CNT = (TX_CallbackData >> 4) & 0x0F;//统计最终发送失败、完全丢失的数据包数量。
      ARC_CNT = (TX_CallbackData >> 0) & 0x0F; //统计当前数据包的重传次数。值越大，信号越差，最差重传15次
    }

    if (SendFlag == 1)
    {
        SendFlag = 0;
        NRF24L01_Send();
        Key_Flag = 0;
    }

    /* displayKey优先级：小车回传状态(权威) > 本地按键(即时预览) > 0(无活动) */
    if (BCar_KeyControl != 0)
        displayKey = BCar_KeyControl;
    else if (Key_Flag != 0)
        displayKey = Key_Flag;
    else
            displayKey = 0;
    if (ARC_CNT <= 3) {
      OLED_ShowImage(0, 0, 16, 16, Signal_3);
    }
    else if (ARC_CNT <= 9) {
      OLED_ShowImage(0, 0, 16, 16, Signal_2);
    }
    else if (ARC_CNT < 15) {
      OLED_ShowImage(0, 0, 16, 16, Signal_1);
    }
    else {
      OLED_ShowImage(0, 0, 16, 16, Signal_0);
    }

    /* displayKey变化时才刷新OLED对应区域，减少I2C带宽浪费 */
    if (displayKey != displayKey_Last)
    {
      if (displayKey != 0)
          OLED_Printf(28, 0, OLED_6X8, " K:%03d", displayKey);
      else
          OLED_Printf(28, 0, OLED_6X8, "        ");
      displayKey_Last = displayKey;
    }

    OLED_Printf(28, 14  , OLED_6X8, " SpeedLevel:%03d", SpeedLevel);
    OLED_Printf(80, 0  , OLED_6X8, "ArcC:%02d", ARC_CNT);
    OLED_Printf(0, 24, OLED_8X16, "LH:%+04d",  LH);
    OLED_Printf(72, 24  , OLED_8X16, "LV:%+04d", LV);
    OLED_Printf(0, 48, OLED_8X16, "RH:%+04d", RH);
    OLED_Printf(72 , 48  , OLED_8X16, "RV:%+04d", RV);
    OLED_Update();


    /*单击按键测试程序*/
    // for (uint8_t i = 0; i < KEY_NUM; i++) {
    //   if (Key_GetPressed(i)) {
    //     OLED_Clear();
    //     OLED_Printf(0 , 0  , OLED_8X16, "K%d", i+1);
    //     OLED_Update();
    //   }
    // }
    /*双击按键测试程序*/
    // OLED_Printf(0 , 0  , OLED_8X16, "K%d", Num);
    // OLED_Update();

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
