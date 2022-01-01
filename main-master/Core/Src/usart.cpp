/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "hw.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}
/* USART4 init function */

void MX_USART4_UART_Init(void)
{

  huart4.Instance = USART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }

}
/* USART5 init function */

void MX_USART5_UART_Init(void)
{

  huart5.Instance = USART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = SIM7600_TX_Pin|SIM7600_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART4)
  {
  /* USER CODE BEGIN USART4_MspInit 0 */

  /* USER CODE END USART4_MspInit 0 */
    /* USART4 clock enable */
    __HAL_RCC_USART4_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART4 GPIO Configuration
    PC10     ------> USART4_TX
    PC11     ------> USART4_RX
    */
    GPIO_InitStruct.Pin = LRWAN_TX_Pin|LRWAN_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_USART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART4 interrupt Init */
    HAL_NVIC_SetPriority(USART4_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART4_5_IRQn);
  /* USER CODE BEGIN USART4_MspInit 1 */

  /* USER CODE END USART4_MspInit 1 */
  }
  else if(uartHandle->Instance==USART5)
  {
  /* USER CODE BEGIN USART5_MspInit 0 */

  /* USER CODE END USART5_MspInit 0 */
    /* USART5 clock enable */
    __HAL_RCC_USART5_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART5 GPIO Configuration
    PC12     ------> USART5_TX
    PD2     ------> USART5_RX
    */
    GPIO_InitStruct.Pin = DEBUG_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_USART5;
    HAL_GPIO_Init(DEBUG_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DEBUG_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_USART5;
    HAL_GPIO_Init(DEBUG_RX_GPIO_Port, &GPIO_InitStruct);

    /* USART5 interrupt Init */
    HAL_NVIC_SetPriority(USART4_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART4_5_IRQn);
  /* USER CODE BEGIN USART5_MspInit 1 */

  /* USER CODE END USART5_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOB, SIM7600_TX_Pin|SIM7600_RX_Pin);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART4)
  {
  /* USER CODE BEGIN USART4_MspDeInit 0 */

  /* USER CODE END USART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART4_CLK_DISABLE();

    /**USART4 GPIO Configuration
    PC10     ------> USART4_TX
    PC11     ------> USART4_RX
    */
    HAL_GPIO_DeInit(GPIOC, LRWAN_TX_Pin|LRWAN_RX_Pin);

    /* USART4 interrupt Deinit */
  /* USER CODE BEGIN USART4:USART4_5_IRQn disable */
    /**
    * Uncomment the line below to disable the "USART4_5_IRQn" interrupt
    * Be aware, disabling shared interrupt may affect other IPs
    */
    /* HAL_NVIC_DisableIRQ(USART4_5_IRQn); */
  /* USER CODE END USART4:USART4_5_IRQn disable */

  /* USER CODE BEGIN USART4_MspDeInit 1 */

  /* USER CODE END USART4_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART5)
  {
  /* USER CODE BEGIN USART5_MspDeInit 0 */

  /* USER CODE END USART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART5_CLK_DISABLE();

    /**USART5 GPIO Configuration
    PC12     ------> USART5_TX
    PD2     ------> USART5_RX
    */
    HAL_GPIO_DeInit(DEBUG_TX_GPIO_Port, DEBUG_TX_Pin);

    HAL_GPIO_DeInit(DEBUG_RX_GPIO_Port, DEBUG_RX_Pin);

    /* USART5 interrupt Deinit */
  /* USER CODE BEGIN USART5:USART4_5_IRQn disable */
    /**
    * Uncomment the line below to disable the "USART4_5_IRQn" interrupt
    * Be aware, disabling shared interrupt may affect other IPs
    */
    /* HAL_NVIC_DisableIRQ(USART4_5_IRQn); */
  /* USER CODE END USART5:USART4_5_IRQn disable */

  /* USER CODE BEGIN USART5_MspDeInit 1 */

  /* USER CODE END USART5_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
