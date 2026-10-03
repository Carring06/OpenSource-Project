/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOC
#define VCC_UP_DETECT_Pin GPIO_PIN_0
#define VCC_UP_DETECT_GPIO_Port GPIOA
#define VCC_DOWN_DETECT_Pin GPIO_PIN_1
#define VCC_DOWN_DETECT_GPIO_Port GPIOA
#define VBAT1_DETECT_Pin GPIO_PIN_2
#define VBAT1_DETECT_GPIO_Port GPIOA
#define TypeC_POWER_IN_DETECT_Pin GPIO_PIN_4
#define TypeC_POWER_IN_DETECT_GPIO_Port GPIOA
#define DC_POWER_IN_DETECT_Pin GPIO_PIN_5
#define DC_POWER_IN_DETECT_GPIO_Port GPIOA
#define XL6019E1_EN_Pin GPIO_PIN_13
#define XL6019E1_EN_GPIO_Port GPIOB
#define K4_Pin GPIO_PIN_14
#define K4_GPIO_Port GPIOB
#define K3_Pin GPIO_PIN_15
#define K3_GPIO_Port GPIOB
#define K2_Pin GPIO_PIN_8
#define K2_GPIO_Port GPIOA
#define K1_Pin GPIO_PIN_11
#define K1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_3
#define LED2_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_4
#define LED3_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_5
#define LED4_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_9
#define OLED_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
