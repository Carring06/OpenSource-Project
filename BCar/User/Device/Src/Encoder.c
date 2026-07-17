//
// Created by Car on 2026/6/16.
//

#include "Encoder.h"


int16_t count1 = 0;
int16_t count2 = 0;

/**
 * @brief  编码器初始化（仅保留启动逻辑，所有底层配置由CubeMX生成）
 * @note   你只需要在CubeMX里配置好编码器，这里直接启动即可
 */
void Encoder_Init(void)
{
    // 仅启动编码器定时器（CubeMX已经完成所有底层配置，包括时钟、GPIO、编码器模式）
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

/**
 * @brief  获取编码器计数值
 * @retval 编码器增量值（读取后清零）
 */
int16_t Encoder_GetML(void)
{
    int16_t Temp;
    Temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    return Temp;
}

int16_t Encoder_GetMR(void)
{
    int16_t Temp;
    Temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    return Temp;
}

int16_t Encoder_GetMLPos(void)
{
    count1 += __HAL_TIM_GET_COUNTER(&htim3);
    return count1 / 2;
}

int16_t Encoder_GetMRPos(void)
{
    count2 += __HAL_TIM_GET_COUNTER(&htim4);
    return count2 / 2;
}

float Encoder_GetMLAngle(void)
{
    count1 += __HAL_TIM_GET_COUNTER(&htim3);
    return count1 / 22.0f / (30613.0f / 1500.0f) * 360.0f;

}

float Encoder_GetMRAngle(void)
{
    count2 += __HAL_TIM_GET_COUNTER(&htim4);
    return count2 / 22.0f / (30613.0f / 1500.0f) * 360.0f;
}

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1)
  {
    count++;
    if (count == 1000)
    {
      speed = Encoder_Get(); // 轮速计测量
      count = 0;
    }
  }
}
*/



